import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Popup {
    id: textInputPopup
    anchors.centerIn: parent
    width: 450
    height: 200
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property alias text: urlArea.text
    signal accepted(string text)
    signal cancelled()

    background: Image{
        source: "../resource/bg.jpg"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
            text: qsTr("请输入url")
            font.pixelSize: 20
            //color: "#333333"
        }

        TextArea {
            id: urlArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            wrapMode: TextEdit.WrapAnywhere
            placeholderText: qsTr("输入抽卡记录页面url")
            font.pixelSize: 12
            selectByMouse: true
            background: Rectangle{
                color:"transparent"
                border.color: "black"
            }
            ContextMenu.menu: null
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            MyButton {
                text: "取消"
                commonFillColor: "lightgrey"
                commonBorderColor: "grey"
                commonTextColor: "grey"

                hoverFillColor: "grey"
                hoverBorderColor: "grey"
                onClicked: {
                    urlArea.clear()
                    textInputPopup.cancelled()
                    textInputPopup.close()
                }
            }

            MyButton {
                text: "确定"
                commonFillColor: "#F0F9EF"
                commonBorderColor: "#67c23a"
                commonTextColor: "#67c23a"

                hoverFillColor: "#67c23a"
                hoverBorderColor: "#67c23a"
                onClicked: {
                    textInputPopup.accepted(urlArea.text)
                    urlArea.clear()
                    textInputPopup.close()
                }
            }
        }
    }

    enter: Transition {
        NumberAnimation { property: "scale"; from: 0.0; to: 1.0; duration: 400; easing.type: Easing.OutElastic }
    }

    exit: Transition {
        NumberAnimation { property: "scale"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.OutCubic }
    }
}
