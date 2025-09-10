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
    property int arrowOffsetX: root.menuWidth/2 - arrowSize//24            // 三角形距离左边的偏移

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
        x: root.width/2 - root.menuWidth/2
        y: root.height + 4
        width: root.menuWidth
        padding: 0
        modal: false
        focus: false
        clip:true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Item {
            id: bg
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
            id: content
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

        enter: Transition {
            ParallelAnimation {
                NumberAnimation { property: "scale"; from: 0; to: 1; duration: 150; easing.type: Easing.OutQuad }
                NumberAnimation { property: "height"; from: 0; to: arrow.height + menuItemHeight * menuModel.count; duration: 150; easing.type: Easing.OutQuad }
            }
        }

        exit: Transition {
            ParallelAnimation {
                NumberAnimation { property: "scale"; from: 1; to: 0; duration: 150; easing.type: Easing.OutQuad }
                NumberAnimation { property: "height"; from: arrow.height + menuItemHeight * menuModel.count; to: 0; duration: 150; easing.type: Easing.OutQuad }
            }
        }
    }/*
    Popup {
        id: menuPopup
        x: root.width/2 - root.menuWidth/2
        y: root.height + 4
        width: root.menuWidth
        padding: 0
        modal: false
        focus: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        // 把背景和内容都放到 contentItem 中，便于统一动画与裁剪
        contentItem: Item {
            id: contentRoot
            width: parent.width
            clip: true

            // 背景组（箭头 + 边框矩形），从顶部伸展开
            Item {
                id: bgGroup
                anchors.left: parent.left
                anchors.right: parent.right
                transformOrigin: Item.Top
                transform: Scale { id: bgScale; yScale: 0 }   // 初始折叠

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
                        ctx.clearRect(0,0,width,height)
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

                // 矩形主体（背景）
                Rectangle {
                    id: bgRect
                    anchors.top: arrow.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    radius: root.menuRadius
                    color: root.menuBgCommonColor
                    border.color: root.menuBorderCommonColor
                }
            }

            // 列表内容组，裁剪并从顶部展开；文字 opacity 由 contentOpacity 控制（用于延迟淡入）
            Item {
                id: listGroup
                anchors.top: bgGroup.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                transformOrigin: Item.Top
                transform: Scale { id: contentScale; yScale: 0 } // 初始折叠
                property real contentOpacity: 0
                clip: true

                Column {
                    id: contentColumn
                    spacing: 0
                    // 占位高度使得箭头位置正确
                    Rectangle { height: arrow.height; width: parent.width; color: "transparent" }

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
                                    PropertyChanges { target: itemRec; color: root.itemCommonColor }
                                    PropertyChanges { target: itemText; color: root.textCommonColor }
                                },
                                State {
                                    name: "pressed"
                                    PropertyChanges { target: itemRec; color: root.itemPressedColor }
                                    PropertyChanges { target: itemText; color: root.textPressedColor }
                                },
                                State {
                                    name: "hover"
                                    PropertyChanges { target: itemRec; color: root.itemHoverColor }
                                    PropertyChanges { target: itemText; color: root.textHoverColor }
                                },
                                State {
                                    name: "disabled"
                                    PropertyChanges { target: itemRec; color: root.itemDisabledColor }
                                    PropertyChanges { target: itemText; color: root.textDisabledColor }
                                }
                            ]

                            state: root.disabled ? "disabled" : (!isHover ? "common" : isPressed ? "pressed" : "hover")

                            Text {
                                id: itemText
                                anchors.centerIn: parent
                                text: model.text
                                color: "#111827"
                                font.pixelSize: root.pixelSize
                                opacity: listGroup.contentOpacity   // 由外部控制淡入
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
            }
        }

        // enter/exit 动画：先展开背景和列表容器，再让文字淡入；收起时先文字淡出再收缩
        enter: Transition {
            ParallelAnimation {
                // 背景从 0->1
                NumberAnimation { target: bgScale; property: "yScale"; from: 0; to: 1; duration: 200; easing.type: Easing.OutQuad }
                // 内容容器从 0->1（和背景同步）
                NumberAnimation { target: contentScale; property: "yScale"; from: 0; to: 1; duration: 200; easing.type: Easing.OutQuad }
                // 文字淡入：稍微延迟，等边框差不多展开了再显示文字
                PauseAnimation { duration: 160 }
                NumberAnimation { target: listGroup; property: "contentOpacity"; from: 0; to: 1; duration: 120 }
            }
        }

        exit: Transition {
            ParallelAnimation {
                // 文字先淡出
                NumberAnimation { target: listGroup; property: "contentOpacity"; from: 1; to: 0; duration: 80 }
                // 然后收缩内容容器 & 背景（加一点错峰）
                PauseAnimation { duration: 80 }
                NumberAnimation { target: contentScale; property: "yScale"; from: 1; to: 0; duration: 160; easing.type: Easing.InQuad }
                PauseAnimation { duration: 120 }
                NumberAnimation { target: bgScale; property: "yScale"; from: 1; to: 0; duration: 160; easing.type: Easing.InQuad }
            }
        }
    }
*/
    Timer {
        id: openTimer
        interval: 80; repeat: false
        onTriggered: {
            if (root.hovered && !menuPopup.opened && root.menuModel.count > 0) {
                menuPopup.open()
            }
        }
    }

    Timer {
        id: closeTimer
        interval: 150; repeat: false
        onTriggered: if (!root.hovered && !root.menuHovered && menuPopup.opened) menuPopup.close()
    }
}
