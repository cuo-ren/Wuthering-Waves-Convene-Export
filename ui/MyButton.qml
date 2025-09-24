import QtQuick

Item {
    id: root
    width: 100
    height: 40

    property color commonBorderColor: "blue"
    property color commonFillColor: "lightblue"
    property color commonTextColor: "blue"

    property color hoverBorderColor: "blue"
    property color hoverFillColor: "blue"
    property color hoverTextColor: "white"

    property color pressedBorderColor: Qt.darker(hoverBorderColor)
    property color pressedFillColor: Qt.darker(hoverFillColor)
    property color pressedTextColor: "white"

    property color disabledBorderColor: "grey"
    property color disabledFillColor: "lightgrey"
    property color disabledTextColor: "white"

    property bool isPressed: false
    property bool isHover: false
    property bool disabled: false

    property url source:""
    property string text: "按钮"
    property int radius: 5
    signal clicked()
/*
    states: [
        State {
            name: "common"
            PropertyChanges {
                target: rec
                color: commonFillColor
                border.color: commonBorderColor
            }
            PropertyChanges {
                target: text
                color: commonTextColor
            }
        },
        State {
            name: "pressed"
            PropertyChanges {
                target: rec
                color: pressedFillColor
                border.color: pressedBorderColor
            }
            PropertyChanges {
                target: text
                color: pressedTextColor
            }
        },
        State {
            name: "hover"
            PropertyChanges {
                target: rec
                color: hoverFillColor
                border.color: hoverBorderColor
            }
            PropertyChanges {
                target: text
                color: hoverTextColor
            }
        },
        State {
            name: "disabled"
            PropertyChanges {
                target: rec
                color: disabledFillColor
                border.color: disabledBorderColor
            }
            PropertyChanges {
                target: text
                color: disabledTextColor
            }
        }
    ]

    state: root.disabled ? "disabled" : (!isHover ? "common" : isPressed ? "pressed" : "hover")
*/
    Rectangle{
        id: rec

        width: root.width
        height: root.height

        color: root.disabled ? disabledFillColor : (!isHover ? commonFillColor : isPressed ? pressedFillColor : hoverFillColor)
        border.color: root.disabled ? disabledBorderColor : (!isHover ? commonBorderColor : isPressed ? pressedBorderColor : hoverBorderColor)

        radius: root.radius

        Image{
            id: icon
            width: parent.height - 20
            height: parent.height - 20
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 10
            source: root.source === "" ? undefined : root.source
        }

        Text {
            id: text
            text: root.text

            color: root.disabled ? disabledTextColor : (!isHover ? commonTextColor : isPressed ? pressedTextColor : hoverTextColor)

            width: parent.width - 0 - icon.width

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: root.source === "" ? undefined : icon.right
            anchors.margins: 10
            anchors.centerIn: root.source === "" ? parent: undefined
            horizontalAlignment: root.source === "" ? Text.AlignHCenter : Text.AlignLeft

            wrapMode: Text.WordWrap
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
            onHoveredChanged: root.isHover = containsMouse
            onPressed: root.isPressed = true
            onReleased: root.isPressed = false
        }
    }
}
