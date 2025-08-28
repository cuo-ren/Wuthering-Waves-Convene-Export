import QtQuick 2.15

Item {
    id: notificationItem
    width: parent ? parent.width : 300
    height: 40//rect.implicitHeight

    property string notifactiontext: "通知文本"
    property int duration: 3000
    property int enterDelay: 0
    property color notifactioncolor: "blue"
    property url path
    signal closed()

    Rectangle {
        id: rect
        anchors.fill: parent
        radius: 8
        color: Qt.lighter(notifactioncolor)
        border.color: notifactioncolor
        opacity: 0

        Text {
            id: label
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 10

            width: parent.width - closebtn.width -10
            text: notifactiontext
            color: "white"
            font.pixelSize: 14
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }
        Item{
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 10

            width: parent.height
            height: parent.height

            Rectangle{
                id: closebtnoverlay
                anchors.fill: parent
                anchors.margins: 5

                color: "grey"
                radius: 5
                visible: false
                opacity: 0.3
            }
            Image {
                id: closebtn
                source: path + "/resource/closebtn.svg"
                anchors.fill: parent
                anchors.margins: 7
            }
            MouseArea{
                hoverEnabled: true
                onEntered: {
                    closebtnoverlay.visible = true
                }
                onExited: {
                    closebtnoverlay.visible = false
                }
                onReleased: {
                    closebtnoverlay.opacity = 0.3
                }
                onPressed: {
                    closebtnoverlay.opacity = 0.2
                }
                anchors.fill:parent
                onClicked: notificationItem.dismiss()
            }
        }
    }

    Timer { id: autoCloseTimer; interval: duration; running: false; onTriggered: notificationItem.dismiss() }
    Timer { id: enterDelayTimer; interval: enterDelay; running: false; onTriggered: enterAnim.start() }

    Component.onCompleted: {
        if (enterDelay > 0) enterDelayTimer.start()
        else enterAnim.start()
    }

    function dismiss() {
        autoCloseTimer.stop()
        enterDelayTimer.stop()
        exitAnim.start()
    }

    // 入场：渐显 + 缩放
    SequentialAnimation {
        id: enterAnim
        ParallelAnimation {
            PropertyAnimation { target: rect; property: "opacity"; from: 0; to: 1; duration: 250 }
            PropertyAnimation { target: rect; property: "scale"; from: 0.9; to: 1; duration: 250; easing.type: Easing.OutCubic }
        }
        ScriptAction { script: autoCloseTimer.start() }
    }

    // 离场：渐隐 + 缩放
    SequentialAnimation {
        id: exitAnim
        ParallelAnimation {
            PropertyAnimation { target: rect; property: "opacity"; to: 0; duration: 200 }
            PropertyAnimation { target: rect; property: "scale"; to: 0.9; duration: 200; easing.type: Easing.InCubic }
        }
        ScriptAction { script: closed() }
    }
}

