/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import "../dateUtils.js" as DateUtils

ListItem.Base {
    id: communicationDelegate

    property variant model
    property variant item
    property url avatar
    property url itemIcon
    property variant timestamp
    property string title
    property string subtitle
    property string text
    property alias customContentArea: customArea
    property int __spacing: units.gu(1)

    property string __normalFontColor: "#333333"
    property string __selectedFontColor: "#f37505"

    __height: content.height + units.gu(2)

    Rectangle {
        id: selectedOverlay
        anchors.fill: parent
        color: "white"
        opacity: 0.4
        visible: item && item.newItem
    }

    Row {
        id: content
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: __spacing
        anchors.leftMargin: units.gu(2)
        anchors.rightMargin: units.gu(2)
        height: childrenRect.height
        spacing: __spacing

        UbuntuShape {
            height: units.gu(6)
            width: units.gu(6)

            image: Image {
                source: communicationDelegate.avatar != "" ? communicationDelegate.avatar : "../assets/avatar-default.png"
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
            }
        }

        Image {
            anchors.top: parent.top
            height: units.gu(2)
            width: units.gu(2)
            source: communicationDelegate.itemIcon
        }

        Column {
            width: parent.width - x

            Label {
                width: parent.width
                height: text != "" ? paintedHeight : 0
                elide: Text.ElideRight
                text: communicationDelegate.title
                color: selected ? __selectedFontColor : __normalFontColor
                fontSize: "medium"
                opacity: 0.9
            }

            Label {
                width: parent.width
                height: text != "" ? paintedHeight : 0
                elide: Text.ElideRight
                text: DateUtils.formatLogDate(communicationDelegate.timestamp)
                color: selected ? __selectedFontColor : __normalFontColor
                fontSize: "small"
                opacity: 0.6
            }

            Label {
                width: parent.width
                height: text != "" ? paintedHeight : 0
                elide: Text.ElideRight
                text: communicationDelegate.subtitle
                color: selected ? __selectedFontColor : __normalFontColor
                font.weight: Font.DemiBold
                fontSize: "small"
                opacity: 0.6
            }

            Label {
                width: parent.width
                height: text != "" ? paintedHeight : 0
                text: communicationDelegate.text
                wrapMode: Text.WordWrap
                color: selected ? __selectedFontColor : __normalFontColor
                fontSize: "small"
                opacity: 0.6
            }

            Item {
                id: customArea
                height: childrenRect.height
                width: childrenRect.width
            }
        }
    }

    MouseArea {
        anchors.fill: content
        onClicked: communicationDelegate.clicked(mouse)
    }
}
