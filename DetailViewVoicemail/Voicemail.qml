import QtQuick 2.0
import Ubuntu.Components 0.1
import "../Widgets" as LocalWidgets
import "../DetailViewKeypad"

LocalWidgets.TelephonyPage {
    id: voicemail

    property variant contact
    property QtObject call: callManager.foregroundCall
    property string number: callManager.voicemailNumber

    title: "Voicemail"

    Component.onDestruction: {
        // if this view was destroyed but we still have
        // active calls, then it means it was manually removed
        // from the stack
        if (previousTab != -1 && callManager.hasCalls) {
            telephony.selectedTabIndex = previousTab
        }
    }

    function isVoicemailActive() {
        return telephony.isVoicemailActive();
    }

    function endCall() {
        if (call) {
            call.endCall();
        }
    }

    Connections {
        target: callManager
        onCallEnded: {
            if (!callManager.hasCalls) {
                if (voicemail.visible && voicemail.previousTab != -1) {
                    telephony.selectedTabIndex = voicemail.previousTab
                }
                telephony.endCall();
            }
        }
    }

    Rectangle {
        id: background

        anchors.fill: parent
        color: "#3a3c41"
    }

    Item {
        id: container

        anchors.fill: parent

        Item {
            id: body

            anchors.fill: parent
            //width: childrenRect.width
            //height: childrenRect.height

            TextCustom {
                id: name

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: units.gu(2)
                anchors.bottom: number.top
                text: "Voicemail"
                color: "white"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "large"
            }

            TextCustom {
                id: number

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: stopWatch.top
                anchors.topMargin: units.gu(0.5)
                text: voicemail.number
                color: "#a0a0a2"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "medium"
            }

            TextCustom {
                id: dialing

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: units.gu(2)
                anchors.top: number.bottom

                text: "Dialing"
                color: "#a0a0a2"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "medium"
                opacity: (call && call.voicemail && call.dialing) ? 1.0 : 0.0
            }

            LocalWidgets.StopWatch {
                id: stopWatch
                time: call && call.voicemail ? call.elapsedTime : 0

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: units.gu(2)
                anchors.bottom: keypad.top
                opacity: (call && call.voicemail && !call.dialing) ? 1.0 : 0.0
            }

            Keypad {
                id: keypad

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                onKeyPressed: {
                    if (call) {
                        call.sendDTMF(label)
                    }
                }
            }

            Row {
                anchors.top: keypad.bottom
                anchors.topMargin: units.gu(3)
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: units.gu(0.5)
                Button {
                    id: dialhangupButton
                    iconSource: isVoicemailActive() ? "../assets/incall_keypad_endcallbutton_icon.png" : "../assets/voicemail_icon.png"
                    width: isVoicemailActive() ? units.gu(8) : units.gu(16)
                    color: isVoicemailActive() ? "#bf400c" : "#268bd2"
                    ItemStyle.class: "dark-button"
                    onClicked: {
                        if(isVoicemailActive())
                            endCall()
                        else
                            telephony.callNumber(voicemail.number)
                    }
                }

                Button {
                    id: speakerButton
                    width: units.gu(8)
                    visible: isVoicemailActive()
                    iconSource: call && call.speaker ? "../assets/incall_keypad_speaker_selected.png" : "../assets/incall_keypad_speaker_unselected.png"
                    color: "#565656"
                    ItemStyle.class: "dark-button"
                    state: call && call.speaker ? "pressed" : ""
                    onClicked: {
                        if (call) {
                            call.speaker = !call.speaker
                        }
                    }
                }
            }
        }
    }
}
