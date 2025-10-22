import QtQuick
import QtQuick.Controls.Basic

RadioButton {
    id: control
    text: qsTr("选项")

    property color commonBgColor: "white"
    property color pressBgColor: "#1976D2"
    property color activeBgColor: "#2196F3"

    property color commonTextColor: activeBgColor
    property color pressTextColor: commonBgColor
    property color activeTextColor: commonBgColor

    property color commonBorderColor: commonBgColor
    property color pressBorderColor: pressBgColor
    property color activeBorderColor: activeBgColor

    checked: false

    // 设置最小尺寸
    implicitWidth: contentItem.implicitWidth + 20
    implicitHeight: contentItem.implicitHeight + 10

    // 去掉默认的indicator
    indicator: null

    background: Rectangle {
        anchors.fill: parent
        radius: 8
        color: control.checked
               ? (control.pressed ? pressBgColor : activeBgColor) // 按下更深蓝
               : (control.pressed ? pressBgColor : commonBgColor)
        border.color: control.checked
                      ? (control.pressed ? pressBorderColor : activeBorderColor) // 按下更深蓝
                      : (control.pressed ? pressBorderColor : commonBorderColor)
        border.width: 1
    }

    contentItem: Text {
        anchors.centerIn: parent
        text: control.text
        font: control.font
        color: control.checked
               ? (control.pressed ? pressTextColor : activeTextColor) // 按下更深蓝
               : (control.pressed ? pressTextColor : commonTextColor)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
