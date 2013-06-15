/*
 * Copyright 2012-2013 Canonical Ltd.
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

import QtQuick 2.0

FocusScope {
    id: editor

    property variant contactModelItem
    property alias street: street.value
    property alias locality: locality.value
    property alias region: region.value
    property alias postcode: postcode.value
    property alias country: country.value

    height: fields.height

    Column {
        id: fields
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: units.dp(1)

        AddressEditorField {
            id: street

            anchors.left: parent.left
            anchors.right: parent.right
            value: contactModelItem.street
            description: i18n.tr("Street")
            focus: true
        }

        AddressEditorField {
            id: locality

            anchors.left: parent.left
            anchors.right: parent.right
            value: contactModelItem.locality
            description: i18n.tr("Locality")
        }

        AddressEditorField {
            id: region

            anchors.left: parent.left
            anchors.right: parent.right
            value: contactModelItem.region
            description: i18n.tr("Region")
        }

        AddressEditorField {
            id: postcode

            anchors.left: parent.left
            anchors.right: parent.right
            value: contactModelItem.postcode
            description: i18n.tr("Postal Code")
        }

        AddressEditorField {
            id: country

            anchors.left: parent.left
            anchors.right: parent.right
            value: contactModelItem.country
            description: i18n.tr("Country")
        }
    }
}