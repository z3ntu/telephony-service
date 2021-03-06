/**
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Andreas Pokorny <andreas.pokorny@canonical.com>
 */

#include <QDBusInterface>
#include "powerdaudiomodemediator.h"

PowerDAudioModeMediator::PowerDAudioModeMediator(PowerD &powerd)
    : powerd(powerd)
{
}

void PowerDAudioModeMediator::audioModeChanged(const QString &mode)
{
    bool enableProximity = !(mode == "speaker" || mode == "bluetooth" || mode == "wired_headset");

    if (mProximityEnabled != enableProximity)
    {
        mProximityEnabled = enableProximity;
        apply();
    }
}

void PowerDAudioModeMediator::apply() const
{
    if (mProximityEnabled) {
        powerd.enableProximityHandling();
    } else {
        // we need to power the screen on before disabling the proximity handling
        QDBusInterface unityIface("com.canonical.Unity.Screen",
                                  "/com/canonical/Unity/Screen",
                                  "com.canonical.Unity.Screen",
                                  QDBusConnection::systemBus());
        QList<QVariant> args;
        args.append("on");
        args.append(3);
        unityIface.callWithArgumentList(QDBus::NoBlock, "setScreenPowerMode", args);
        powerd.disableProximityHandling();
    }
}

void PowerDAudioModeMediator::audioOutputClosed()
{
    if (mProximityEnabled)
    {
        mProximityEnabled = false;
        apply();
    }
}
