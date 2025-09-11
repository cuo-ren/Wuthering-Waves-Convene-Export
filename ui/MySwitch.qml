import QtQuick
import QtQuick.Controls.Basic
import Qt5Compat.GraphicalEffects

Switch {
    id: control

    width: 48
    height: 26

    text: qsTr("开关")
    font.pixelSize: height*2/3

    property color trackColor: "#ffffff"      // 背景颜色
    property color trackBorderColor: "#cccccc"      // 背景边框颜色
    property color trackActiveColor: "#17a81a" // 背景激活颜色
    property color handleColor: "#ffffff"     // 滑块颜色
    property color handleDownColor: "#cccccc" // 滑块按下颜色
    property color handleBorderColor: "#999999"
    property color handleBorderCheckedColor: "#21be2b"
    property int duration: 100

    indicator: Rectangle {
        implicitWidth: control.width
        implicitHeight: control.height

        x: control.leftPadding
        y: parent.height / 2 - height / 2

        radius: control.height/2

        color: control.trackColor
        border.color: control.trackBorderColor

        clip: true
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                width: control.width
                height: control.height
                radius: control.height/2 // 圆角半径
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: control.visualPosition * parent.width < handle.width/2 ? handle.width/2 : control.visualPosition * parent.width > parent.width - handle.width/2 ? parent.width - handle.width/2 : control.visualPosition * parent.width
            //radius: parent.radius
            color: control.trackActiveColor
            Behavior on width {
                enabled: !control.down
                NumberAnimation { duration: control.duration }
                }
        }

        Rectangle {
            id: handle
            y: (parent.height - height) / 2
            width: control.height
            height: control.height
            radius: control.height/2
            color: control.down ? control.handleDownColor : control.handleColor
            border.color: control.checked ? (control.down ? control.trackActiveColor : control.handleBorderCheckedColor) : control.handleBorderColor

            x: Math.max(0, Math.min(parent.width - width,
                                    control.visualPosition * parent.width - (width / 2)))

            Behavior on x {
                enabled: !control.down
                NumberAnimation { duration: control.duration }
            }
        }
    }



    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: control.down ? control.trackActiveColor : control.handleBorderCheckedColor
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
    }
}

