/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "calllogmodel.h"
#include "contactmodel.h"
#include "telepathyhelper.h"
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/CallEvent>
#include <TelepathyQt/Contact>

QVariant CallLogEntry::data(int role) const
{
    switch(role) {
    case CallLogModel::Duration:
        return duration;
    case CallLogModel::Missed:
        return missed;
    default:
        return LogEntry::data(role);
    }
}

CallLogModel::CallLogModel(QObject *parent) :
    AbstractLoggerModel(parent)
{
    // set the role names
    QHash<int, QByteArray> roles = roleNames();
    roles[Duration] = "duration";
    roles[Missed] = "missed";
    setRoleNames(roles);

    fetchLog(Tpl::EventTypeMaskCall);
}

void CallLogModel::onCallEnded(const Tp::CallChannelPtr &channel)
{
    Tp::Contacts contacts = channel->remoteMembers();
    if (contacts.isEmpty()) {
        qWarning() << "Call channel had no remote contacts:" << channel;
        return;
    }

    CallLogEntry *entry = new CallLogEntry();
    // FIXME: handle conference call
    Q_FOREACH(const Tp::ContactPtr &contact, contacts) {
        entry->phoneNumber = contact->id();
        break;
    }

    // fill the contact info
    ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(entry->phoneNumber);
    entry->contactAlias = entry->phoneNumber;
    if (contact) {
        fillContactInfo(entry, contact);
    }

    // fill the call info
    entry->timestamp = channel->property("timestamp").toDateTime();
    bool isIncoming = channel->initiatorContact() != TelepathyHelper::instance()->account()->connection()->selfContact();
    entry->incoming = isIncoming;
    entry->duration = QTime(0,0,0);

    // outgoing calls can be missed calls?
    if (entry->incoming && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer) {
        entry->missed = true;
    } else {
        QDateTime activeTime = channel->property("activeTime").toDateTime();
        entry->duration.addSecs(activeTime.secsTo(QDateTime::currentDateTime()));
        entry->missed = false;
    }

    // and finally add the entry
    appendEntry(entry);
}

LogEntry *CallLogModel::createEntry(const Tpl::EventPtr &event)
{
    CallLogEntry *entry = new CallLogEntry();
    Tpl::CallEventPtr callEvent = event.dynamicCast<Tpl::CallEvent>();

    if (callEvent.isNull()) {
        qWarning() << "The event" << event << "is not a Tpl::CallEvent!";
    }

    entry->missed = (callEvent->endReason() == Tp::CallStateChangeReasonNoAnswer);
    entry->duration = callEvent->duration();
    return entry;
}
