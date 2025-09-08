pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic

ComboBox {
    id: control

    property color bgCommonColor: "transparent"
    property color bgPressedColor: "transparent"

    property color canvasCommonColor: borderCommonColor
    property color canvasPressedColor: borderPressedColor

    property color borderCommonColor: "#21be2b"
    property color borderPressedColor: "#17a81a"

    property color textCommonColor: "#21be2b"
    property color textPressedColor: "#17a81a"

    property color popupColor: bgCommonColor
    property color popupBorderColor: borderCommonColor

    property color highlightColor: "#c1f7c3"
    property color commonColor: "transparent"

    property color optionsTextCommonColor: textCommonColor
    property color optionsTextPressedColor: textPressedColor

    property int contentHeight: height

    delegate: ItemDelegate {
        background: Rectangle {
            color: control.highlightedIndex === index ? control.highlightColor : control.commonColor
            radius: 2
        }
        id: delegate
        padding: 0
        required property var model
        required property int index

        width: control.width
        height: control.contentHeight
        contentItem: Text {
            leftPadding: 15
            text: delegate.model[control.textRole]
            color: delegate.pressed ? control.optionsTextPressedColor : control.optionsTextCommonColor
            font.bold: control.currentIndex === index ? true : false
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        highlighted: control.highlightedIndex === index
    }

    indicator: Canvas {
        id: canvas
        x: control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 12
        height: 8
        contextType: "2d"

        rotation: control.popup.visible ? 180 : 0

        Behavior on rotation{
            RotationAnimation{
                duration: 100
            }
        }

        Connections {
            target: control
            function onPressedChanged() { canvas.requestPaint(); }
        }

        onPaint: {
            var mycontext = getContext(contextType);
            mycontext.reset();
            mycontext.moveTo(0, 0);
            mycontext.lineTo(width, 0);
            mycontext.lineTo(width / 2, height);
            mycontext.closePath();
            mycontext.fillStyle = control.pressed ? control.canvasPressedColor : control.canvasCommonColor;
            mycontext.fill();
        }
    }

    contentItem: Text {
        leftPadding: 10
        rightPadding: control.indicator.width + control.spacing
        text: control.displayText
        color: control.pressed ? control.textPressedColor : control.textCommonColor
        verticalAlignment: Text.AlignVCenter
        //horizontalAlignment:Text.AlignHCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 40
        color: control.pressed ? control.bgPressedColor : control.bgCommonColor
        border.color: control.pressed ? control.borderPressedColor : control.borderCommonColor
        border.width: control.visualFocus ? 2 : 1
        radius: 2
    }

    popup: Popup {
        y: control.height - 1
        width: control.width
        height: Math.min(contentItem.implicitHeight, control.Window.height - topMargin - bottomMargin) + padding*2
        padding: 1

        contentItem: ListView {
            clip: false
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            color: control.popupColor
            border.color: control.popupBorderColor
            radius: 2
        }
    }
}


