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
    property ListModel menuModel: ListModel {}
    signal clicked()
    signal triggered(int index, var item)

    // 样式
    property int radius: 8
    property int padding: 12

    property color bgCommonColor: "#ffffff"
    property color bgHoverColor: "#f2f3f5"
    property color bgPressedColor: "#f2f3f5"
    property color bgDisabledColor: "grey"

    property color borderCommonColor: "#e5e7eb"
    property color borderHoverColor: "#d0d5dd"
    property color borderPressedColor: "#d0d5dd"
    property color borderDisabledColor: Qt.darker("grey")

    property color textCommonColor: "#1f2937"
    property color textHoverColor: "#1f2937"
    property color textPressedColor: "#1f2937"
    property color textDisabledColor: "#1f2937"

    property color menuBgCommonColor: "#ffffff"
    property color menuBgHoverColor: "#ffffff"
    property color menuBgPressedColor: "#ffffff"
    property color menuBgDisabledColor: "#ffffff"

    property color menuBorderCommonColor: "#e5e7eb"
    property color menuBorderHoverColor: "#e5e7eb"
    property color menuBorderPressedColor: "#e5e7eb"
    property color menuBorderDisabledColor: "#e5e7eb"

    property color itemCommonColor: "transparent"
    property color itemHoverColor: "#f3f4f6"
    property color itemPressedColor: "#f3f4f6"
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

        state: root.disabled ? "disabled" : (!root.hovered ? "common" : root.pressed ? "pressed" : "hover")

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
                font.pixelSize: 14
                verticalAlignment: Text.AlignVCenter
            }
        }
        MouseArea{
            id: btnArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: containsMouse ? root.disabled ? Qt.ForbiddenCursor : Qt.PointingHandCursor:Qt.ArrowCursor
            acceptedButtons: Qt.NoButton
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onPressedChanged: root.pressed = pressed

        onTapped: {
            if(!root.disabled){
                root.clicked()
            }
        }
    }

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
                                color: itemCommonColor
                            }
                            PropertyChanges {
                                target: itemText
                                color: textCommonColor
                            }
                        },
                        State {
                            name: "pressed"
                            PropertyChanges {
                                target: itemRec
                                color: itemPressedColor
                            }
                            PropertyChanges {
                                target: itemText
                                color: textPressedColor
                            }
                        },
                        State {
                            name: "hover"
                            PropertyChanges {
                                target: itemRec
                                color: itemHoverColor
                            }
                            PropertyChanges {
                                target: itemText
                                color: textHoverColor
                            }
                        },
                        State {
                            name: "disabled"
                            PropertyChanges {
                                target: itemRec
                                color: itemDisabledColor
                            }
                            PropertyChanges {
                                target: itemText
                                color: textDisabledColor
                            }
                        }
                    ]

                    state: root.disabled ? "disabled" : (!isHover ? "common" : isPressed ? "pressed" : "hover")

                    Text {
                        id: itemText
                        anchors.centerIn: parent
                        text: model.text
                        color: "#111827"
                        font.pixelSize: 14
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
