/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "phoneapplication.h"

#include <QDir>
#include <QUrl>
#include <QDebug>
#include <QStringList>
#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickView>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QLibrary>
#include "config.h"
#include "phoneappdbus.h"
#include <QQmlEngine>

static void printUsage(const QStringList& arguments)
{
    qDebug() << "usage:"
             << arguments.at(0).toUtf8().constData()
             << "[contact://CONTACT_KEY]"
             << "[call://PHONE_NUMBER]"
             << "[message://PHONE_NUMBER]"
             << "[messageId://MESSAGE_ID]"
             << "[voicemail://]"
             << "[--dual-panel]"
             << "[--single-panel]"
             << "[--fullscreen]"
             << "[--test-contacts]"
             << "[--help]"
             << "[-testability]";
}

PhoneApplication::PhoneApplication(int &argc, char **argv)
    : QGuiApplication(argc, argv), m_view(0), m_applicationIsReady(false)
{
    setApplicationName("PhoneApp");
    m_dbus = new PhoneAppDBus(this);
}

bool PhoneApplication::setup()
{
    static QList<QString> validSchemes;
    bool singlePanel = true;
    bool fullScreen = false;
    QString contactEngine = "folks";

    if (validSchemes.isEmpty()) {
        validSchemes << "contact";
        validSchemes << "call";
        validSchemes << "message";
        validSchemes << "messageId";
        validSchemes << "voicemail";
    }

    QString contactKey;
    QStringList arguments = this->arguments();

    if (arguments.contains("--help")) {
        printUsage(arguments);
        return false;
    }

    /* Ubuntu APP Manager gathers info on the list of running applications from the .desktop
       file specified on the command line with the desktop_file_hint switch. 
       So app will be launched like this:

       /usr/bin/phone-app --desktop_file_hint=/usr/share/applications/phone-app.desktop

       So remove that argument and continue parsing.
    */
    for (int i = arguments.count() - 1; i >=0; --i) {
        if (arguments[i].startsWith("--desktop_file_hint")) {
            arguments.removeAt(i);
        }
    }


    if (arguments.contains("--dual-panel")) {
        arguments.removeAll("--dual-panel");
        singlePanel = false;
    }

    if (arguments.contains("--single-panel")) {
        arguments.removeAll("--single-panel");
        singlePanel = true;
    }

    if (arguments.contains("--fullscreen")) {
        arguments.removeAll("--fullscreen");
        fullScreen = true;
    }

    if (arguments.contains("--test-contacts")) {
        arguments.removeAll("--test-contacts");
        contactEngine = "memory";
    }

    // The testability driver is only loaded by QApplication but not by QGuiApplication.
    // However, QApplication depends on QWidget which would add some unneeded overhead => Let's load the testability driver on our own.
    if (arguments.contains("-testability")) {
        arguments.removeAll("-testability");
        QLibrary testLib(QLatin1String("qttestability"));
        if (testLib.load()) {
            typedef void (*TasInitialize)(void);
            TasInitialize initFunction = (TasInitialize)testLib.resolve("qt_testability_init");
            if (initFunction) {
                initFunction();
            } else {
                qCritical("Library qttestability resolve failed!");
            }
        } else {
            qCritical("Library qttestability load failed!");
        }
    }

    if (arguments.size() == 2) {
        QUrl uri(arguments.at(1));
        if (validSchemes.contains(uri.scheme())) {
            m_arg = arguments.at(1);
        }
    }

    // check if the app is already running, if it is, send the message to the running instance
    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered("com.canonical.PhoneApp");
    if (reply.isValid() && reply.value()) {
        QDBusInterface appInterface("com.canonical.PhoneApp",
                                    "/com/canonical/PhoneApp",
                                    "com.canonical.PhoneApp");
        appInterface.call("SendAppMessage", m_arg);
        return false;
    }

    if (!m_dbus->connectToBus()) {
        qWarning() << "Failed to expose com.canonical.PhoneApp on DBUS.";
    }

    m_view = new QQuickView();
    QObject::connect(m_view, SIGNAL(statusChanged(QQuickView::Status)), this, SLOT(onViewStatusChanged(QQuickView::Status)));
    QObject::connect(m_view->engine(), SIGNAL(quit()), SLOT(quit()));
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);
    m_view->setTitle("Phone");
    m_view->rootContext()->setContextProperty("application", this);
    m_view->rootContext()->setContextProperty("contactKey", contactKey);
    m_view->rootContext()->setContextProperty("dbus", m_dbus);
    m_view->rootContext()->setContextProperty("appLayout", singlePanel ? "singlePane" : "dualPane" );
    m_view->rootContext()->setContextProperty("contactEngine", contactEngine);
    m_view->engine()->setBaseUrl(QUrl::fromLocalFile(phoneAppDirectory()));
    m_view->setSource(QUrl::fromLocalFile("phone-app.qml"));
    if (fullScreen) {
        m_view->showFullScreen();
    } else {
        m_view->show();
    }

    connect(m_dbus,
            SIGNAL(request(QString)),
            SLOT(onMessageReceived(QString)));
    connect(m_dbus,
            SIGNAL(messageSendRequested(QString,QString)),
            SLOT(onMessageSendRequested(QString,QString)));

    return true;
}

PhoneApplication::~PhoneApplication()
{
    if (m_view) {
        delete m_view;
    }
}

void PhoneApplication::onViewStatusChanged(QQuickView::Status status)
{
    if (status != QQuickView::Ready) {
        return;
    }

    QQuickItem *mainView = m_view->rootObject();
    if (mainView) {
        QObject::connect(mainView, SIGNAL(applicationReady()), this, SLOT(onApplicationReady()));
    }
}

void PhoneApplication::onApplicationReady()
{
    QObject::disconnect(QObject::sender(), SIGNAL(applicationReady()), this, SLOT(onApplicationReady()));
    m_applicationIsReady = true;
    parseArgument(m_arg);
    m_arg.clear();
}

void PhoneApplication::onMessageSendRequested(const QString &phoneNumber, const QString &message)
{
    QQuickItem *mainView = m_view->rootObject();
    if (!mainView) {
        return;
    }
    const QMetaObject *mo = mainView->metaObject();
    int index = mo->indexOfMethod("sendMessage(QVariant,QVariant)");
    if (index != -1) {
        QMetaMethod method = mo->method(index);
        method.invoke(mainView,
                      Q_ARG(QVariant, QVariant(phoneNumber)),
                      Q_ARG(QVariant, QVariant(message)));
    }
}

void PhoneApplication::parseArgument(const QString &arg)
{
    if (arg.isEmpty()) {
        return;
    }

    QStringList args = arg.split("://");
    if (args.size() != 2) {
        return;
    }

    QString scheme = args[0];
    QString value = args[1];

    QQuickItem *mainView = m_view->rootObject();
    if (!mainView) {
        return;
    }
    const QMetaObject *mo = mainView->metaObject();


    if (scheme == "contact") {
        // Workaround to propagate a property change even when the contactKey was the same
        m_view->rootContext()->setContextProperty("contactKey", "");
        m_view->rootContext()->setContextProperty("contactKey", value);
    } else if (scheme == "call") {
        int index = mo->indexOfMethod("callNumber(QVariant)");
        if (index != -1) {
            QMetaMethod method = mo->method(index);
            method.invoke(mainView, Q_ARG(QVariant, QVariant(value)));
        }
    } else if (scheme == "message") {
        if (value.isEmpty()) {
            int index = mo->indexOfMethod("startNewMessage()");
            if (index != -1) {
                QMetaMethod method = mo->method(index);
                method.invoke(mainView);
            }
        } else {
            int index = mo->indexOfMethod("startChat(QVariant)");
            if (index != -1) {
                QMetaMethod method = mo->method(index);
                method.invoke(mainView,
                              Q_ARG(QVariant, QVariant(value)));
            }
       }
    } else if (scheme == "messageId") {
        int index = mo->indexOfMethod("showMessage(QVariant)");
        if (index != -1) {
            QMetaMethod method = mo->method(index);
            method.invoke(mainView,
                          Q_ARG(QVariant, QVariant("")),
                          Q_ARG(QVariant, QVariant(value)));
        }
    } else if (scheme == "voicemail") {
        int index = mo->indexOfMethod("showVoicemail()");
        if (index != -1) {
            QMetaMethod method = mo->method(index);
            method.invoke(mainView);
        }
    }
}

void PhoneApplication::onMessageReceived(const QString &message)
{
    if (m_applicationIsReady) {
        parseArgument(message);
        m_arg.clear();
        activateWindow();
    } else {
        m_arg = message;
    }
}

void PhoneApplication::activateWindow()
{
    if (m_view) {
        m_view->raise();
        m_view->requestActivate();
    }
}
