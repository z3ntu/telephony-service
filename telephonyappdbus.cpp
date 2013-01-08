/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "telephonyappdbus.h"
#include "telephonyappadaptor.h"

// Qt
#include <QtDBus/QDBusConnection>

static const char* DBUS_SERVICE = "com.canonical.TelephonyApp";
static const char* DBUS_OBJECT_PATH = "/com/canonical/TelephonyApp";

TelephonyAppDBus::TelephonyAppDBus(QObject* parent) : QObject(parent)
{
}

TelephonyAppDBus::~TelephonyAppDBus()
{
}

bool
TelephonyAppDBus::connectToBus()
{
    bool ok = QDBusConnection::sessionBus().registerService(DBUS_SERVICE);
    if (!ok) {
        return false;
    }
    new TelephonyAppAdaptor(this);
    QDBusConnection::sessionBus().registerObject(DBUS_OBJECT_PATH, this);

    return true;
}

void
TelephonyAppDBus::ShowMessages(const QString &number)
{
    Q_EMIT request(QString("message://%1").arg(number));
}

void TelephonyAppDBus::ShowMessage(const QString &messageId)
{
    Q_EMIT request(QString("messageId://%1").arg(messageId));
}

void TelephonyAppDBus::NewMessage()
{
    Q_EMIT request(QString("message://"));
}

void TelephonyAppDBus::SendMessage(const QString &number, const QString &message)
{
    Q_EMIT messageSendRequested(number, message);
}

void TelephonyAppDBus::ShowVoicemail()
{
    Q_EMIT request(QString("voicemail://"));
}

void
TelephonyAppDBus::CallNumber(const QString &number)
{
    Q_EMIT request(QString("call://%1").arg(number));
}

void TelephonyAppDBus::SendAppMessage(const QString &message)
{
    Q_EMIT request(message);
}
