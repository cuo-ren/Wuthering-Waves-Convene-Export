// HoverDropdownButton.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: Math.max(120, buttonRow.implicitWidth + padding * 2)
    height: 36

    property alias text: label.text
    property alias iconSource: icon.source
    property int pixelSize: 12
    property ListModel menuModel: ListModel {}
    signal clicked()
    signal triggered(int index, var item)

    // 样式
    property int radius: 8
    property int padding: 12

    property color bgCommonColor: "#ffffff"
    property color borderCommonColor: "#e5e7eb"
    property color textCommonColor: "#1f2937"
    property color menuBgCommonColor: "#ffffff"
    property color menuBorderCommonColor: "#e5e7eb"
    property color itemCommonColor: "transparent"

    property color bgHoverColor: borderCommonColor
    property color borderHoverColor: "#d0d5dd"
    property color textHoverColor: "#1f2937"
    property color menuBgHoverColor: menuBorderCommonColor
    property color menuBorderHoverColor: "#e5e7eb"
    property color itemHoverColor: "#f3f4f6"

    property color bgPressedColor: Qt.darker(borderCommonColor)
    property color borderPressedColor: Qt.darker(borderCommonColor)
    property color textPressedColor: textHoverColor
    property color menuBgPressedColor: menuBorderCommonColor
    property color menuBorderPressedColor: menuBorderCommonColor
    property color itemPressedColor: Qt.darker(borderCommonColor)

    property color bgDisabledColor: "grey"
    property color borderDisabledColor: Qt.darker("grey")
    property color textDisabledColor: "#1f2937"
    property color menuBgDisabledColor: "#ffffff"
    property color menuBorderDisabledColor: Qt.darker("grey")
    property color itemDisabledColor: "grey"

    property int menuRadius: 8
    property int menuWidth: root.width       // ✅ 默认和按钮一样宽
    property int menuItemHeight: 36
    property int arrowSize: 10               // 三角形高度
    property int arrowOffsetX: root.width/2 - arrowSize//24            // 三角形距离左边的偏移

    property bool hovered: false
    property bool pressed: false
    property bool menuHovered: false
    property bool disabled: false

    // ===== 按钮 =====
    Rectangle {
        id: buttonBg
        anchors.fill: parent
        radius: root.radius
        color: root.bgCommonColor
        border.color: root.borderCommonColor

        states: [
            State {
                name: "common"
                PropertyChanges {
                    target: buttonBg
                    color: bgCommonColor
                    border.color: borderCommonColor
                }
                PropertyChanges {
                    target: label
                    color: textCommonColor
                }
            },
            State {
                name: "pressed"
                PropertyChanges {
                    target: buttonBg
                    color: bgPressedColor
                    border.color: borderPressedColor
                }
                PropertyChanges {
                    target: label
                    color: textPressedColor
                }
            },
            State {
                name: "hover"
                PropertyChanges {
                    target: buttonBg
                    color: bgHoverColor
                    border.color: borderHoverColor
                }
                PropertyChanges {
                    target: label
                    color: textHoverColor
                }
            },
            State {
                name: "disabled"
                PropertyChanges {
                    target: buttonBg
                    color: bgDisabledColor
                    border.color: borderDisabledColor
                }
                PropertyChanges {
                    target: label
                    color: textDisabledColor
                }
            }
        ]

        state: root.disabled ? "disabled" : (!btnArea.containsMouse ? "common" : btnArea.containsPress ? "pressed" : "hover")

        Row {
            id: buttonRow
            spacing: 8
            anchors.centerIn: parent

            Image {
                id: icon
                visible: source !== ""
                sourceSize.width: 18
                sourceSize.height: 18
            }

            Text {
                id: label
                text: "Action"
                color: root.textCommonColor
                font.pixelSize: root.pixelSize
                verticalAlignment: Text.AlignVCenter
            }
        }
        MouseArea{
            id: btnArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: containsMouse ? root.disabled ? Qt.ForbiddenCursor : Qt.PointingHandCursor:Qt.ArrowCursor
            onClicked: {
                if(!root.disabled){
                    root.clicked()
                }
            }
        }
    }
/*
    TapHandler {
        acceptedButtons: Qt.LeftButton
        onPressedChanged: root.pressed = pressed

        onTapped: {

        }
    }
*/
    HoverHandler {
        onHoveredChanged: {
            root.hovered = hovered
            if(!root.disabled){
                if (hovered) openTimer.restart()
                else closeTimer.restart()
            }
        }
    }

    // ===== 菜单 Popup =====
    Popup {
        id: menuPopup
        x: 0
        y: root.height + 4
        width: root.menuWidth
        padding: 0
        modal: false
        focus: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Item {
            // 三角形
            Canvas {
                id: arrow
                width: root.arrowSize * 2
                height: root.arrowSize
                anchors.left: parent.left
                anchors.leftMargin: root.arrowOffsetX
                anchors.top: parent.top
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.beginPath()
                    ctx.moveTo(width/2, 0)
                    ctx.lineTo(0, height)
                    ctx.lineTo(width, height)
                    ctx.closePath()
                    ctx.fillStyle = root.menuBgCommonColor
                    ctx.fill()
                    ctx.strokeStyle = root.menuBorderCommonColor
                    ctx.stroke()
                }
            }

            // 矩形主体
            Rectangle {
                anchors.top: arrow.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                radius: root.menuRadius
                color: root.menuBgCommonColor
                border.color: root.menuBorderCommonColor
            }
        }

        contentItem: Column {
            spacing: 0
            Rectangle { height: arrow.height; width: parent.width; color: "transparent" } // 占位
            Repeater {
                model: root.menuModel
                delegate: Rectangle {
                    id: itemRec
                    width: menuPopup.width
                    height: root.menuItemHeight
                    radius: root.menuRadius
                    color: root.itemCommonColor

                    property bool isHover: ma.containsMouse
                    property bool isPressed: ma.containsPress

                    states: [
                        State {
                            name: "common"
                            PropertyChanges {
                                target: itemRec
                                color: root.itemCommonColor
                            }
                            PropertyChanges {
                                target: itemText
                                color: root.textCommonColor
                            }
                        },
                        State {
                            name: "pressed"
                            PropertyChanges {
                                target: itemRec
                                color: root.itemPressedColor
                            }
                            PropertyChanges {
                                target: itemText
                                color: root.textPressedColor
                            }
                        },
                        State {
                            name: "hover"
                            PropertyChanges {
                                target: itemRec
                                color: root.itemHoverColor
                            }
                            PropertyChanges {
                                target: itemText
                                color: root.textHoverColor
                            }
                        },
                        State {
                            name: "disabled"
                            PropertyChanges {
                                target: itemRec
                                color: root.itemDisabledColor
                            }
                            PropertyChanges {
                                target: itemText
                                color: root.textDisabledColor
                            }
                        }
                    ]

                    state: root.disabled ? "disabled" : (!isHover ? "common" : isPressed ? "pressed" : "hover")

                    Text {
                        id: itemText
                        anchors.centerIn: parent
                        text: model.text
                        color: "#111827"
                        font.pixelSize: root.pixelSize
                    }

                    MouseArea {
                        id: ma
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            if(!root.disabled){
                                root.triggered(index, root.menuModel.get(index))
                                menuPopup.close()
                            }
                        }
                    }
                }
            }
        }

        HoverHandler {
            onHoveredChanged: {
                root.menuHovered = hovered
                if (!hovered) closeTimer.restart()
            }
        }
    }

    Timer {
        id: openTimer
        interval: 80; repeat: false
        onTriggered: if (root.hovered && !menuPopup.opened) menuPopup.open()
    }

    Timer {
        id: closeTimer
        interval: 150; repeat: false
        onTriggered: if (!root.hovered && !root.menuHovered && menuPopup.opened) menuPopup.close()
    }
}
