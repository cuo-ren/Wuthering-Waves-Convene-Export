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
            font.pixelSize: 14
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
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            Button {
                text: "取消"
                onClicked: {
                    urlArea.clear()
                    textInputPopup.cancelled()
                    textInputPopup.close()
                }
            }

            Button {
                text: "确定"
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

