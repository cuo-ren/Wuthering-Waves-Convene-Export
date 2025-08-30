import QtQuick

Item {
    id: root
    width: 640
    height: 480

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
    property string usedText: "按钮"
    signal click()

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

    Rectangle{
        id: rec

        width: root.width
        height: root.height

        color: commonFillColor
        border.color: commonBorderColor

        radius: 5

        Text {
            id: text
            text: usedText

            width: parent.width

            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter

            wrapMode: Text.WordWrap
        }

        MouseArea{
            id: btnArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: containsMouse ? root.disabled ? Qt.ForbiddenCursor : Qt.PointingHandCursor:Qt.ArrowCursor
            onClicked: {
                if(!root.disabled){
                    root.click()
                }
            }
            onHoveredChanged: root.isHover = containsMouse
            onPressed: root.isPressed = true
            onReleased: root.isPressed = false
        }
    }
}
