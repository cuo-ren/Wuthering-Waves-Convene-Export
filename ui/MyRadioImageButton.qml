import QtQuick
import QtQuick.Controls.Basic

RadioButton {
    id: control
    property string iconNormal: ""   // 未选中图标
    property string iconChecked: "" // 选中图标
    property bool showText: false

    property color commonBgColor: "white"
    property color pressBgColor: "#1976D2"
    property color activeBgColor: "#2196F3"

    property color commonTextColor: activeBgColor
    property color pressTextColor: commonBgColor
    property color activeTextColor: commonBgColor

    property color commonBorderColor: commonBgColor
    property color pressBorderColor: pressBgColor
    property color activeBorderColor: activeBgColor

    text: qsTr("")

    implicitWidth: 80
    implicitHeight: showText ? 100 : 80

    indicator: null

    background: Rectangle {
        anchors.fill: parent
        radius: 10
        color: control.checked
               ? (control.pressed ? control.pressBgColor : control.activeBgColor)
               : (control.pressed ? control.pressBgColor : control.commonBgColor)
        border.color: control.checked
                      ? (control.pressed ? control.pressBorderColor : control.activeBorderColor)
                      : (control.pressed ? control.pressBorderColor : control.commonBorderColor)
        border.width: 1
    }


    contentItem:Item{

        Image {
            id: icon
            width: control.width - 5
            height: control.height - 5
            source: control.checked ? control.iconChecked : control.iconNormal
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            visible: control.showText
            anchors.top:icon.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            text: control.text
            font.pixelSize: 14
            color: control.checked
                   ? (control.pressed ? control.pressTextColor : control.activeTextColor)
                   : (control.pressed ? control.pressTextColor : control.commonTextColor)
            horizontalAlignment: Text.AlignHCenter
        }
    }
}

