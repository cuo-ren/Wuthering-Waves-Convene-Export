import QtQuick 2.15
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Notifier

Popup {
    id: root
    anchors.centerIn: parent
    width: parent.width/3*2
    height: parent.height/3*2
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property ListModel textModel: ListModel {}
    property string title: qsTr("标题")

    background: Image{
        source: "../resource/bg.jpg"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            Text {
                text: root.title
                font.pixelSize: 20
                Layout.fillWidth: true   // 占满剩余空间
                elide: Text.ElideRight   // 太长时省略
                verticalAlignment: Text.AlignVCenter
                height: 34
            }

            // 关闭按钮
            Item {
                id: closeButton
                width: 30
                height: 30

                Rectangle {
                    id: closehoverOverlay
                    anchors.fill: parent
                    radius: width / 2
                    color: "black"
                    opacity: 0.3
                    visible: false
                }

                Image {
                    id: closeButtonImage
                    anchors.fill: parent
                    source: "../resource/closebtn.svg"
                    fillMode: Image.PreserveAspectFit

                    transformOrigin: Item.Center
                    property int targetRotation: 0
                    property bool isAnimating: false

                    RotationAnimation {
                        id: closeButtonrotateAnim
                        target: closeButtonImage
                        property: "rotation"
                        duration: 200
                        onStarted: closeButtonImage.isAnimating = true
                        onStopped: {
                            closeButtonImage.isAnimating = false
                            // 自动对齐到最近的整90°
                            let snapped = Math.round(closeButtonImage.rotation / 90) * 90
                            closeButtonImage.rotation = snapped
                            closeButtonImage.targetRotation = snapped
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: { root.close() }
                    onEntered: {
                        closehoverOverlay.visible = true
                        closeButtonImage.targetRotation += 90
                        closeButtonrotateAnim.from = closeButtonImage.rotation
                        closeButtonrotateAnim.to = closeButtonImage.targetRotation
                        closeButtonrotateAnim.start()
                    }
                    onExited: {
                        closehoverOverlay.visible = false
                        closeButtonImage.targetRotation += 90
                        closeButtonrotateAnim.from = closeButtonImage.rotation
                        closeButtonrotateAnim.to = closeButtonImage.targetRotation
                        closeButtonrotateAnim.start()
                    }
                    onPressed: closehoverOverlay.opacity = 0.2
                    onReleased: closehoverOverlay.opacity = 0.3
                }
            }
        }

        ListView {
            id: listview

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true

            model: textview.textModel

            TextInput {
                // 隐藏 TextInput 用于复制
                id: copyInput
                width: 0
                height: 0
                visible: false
                clip: true
            }

            delegate: Rectangle {
                width: listview.width
                // 高度 = 文本高度 + 上下 padding
                height: textItem.implicitHeight + 10 < 40 ? 40 : textItem.implicitHeight + 10
                color: index % 2 === 0 ? "#f0f0f0" : "#ffffff"

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        copyInput.text = model.url
                        copyInput.cursorPosition = 0
                        copyInput.moveCursorSelection(model.url.length, TextInput.SelectCharacters)
                        copyInput.copy()
                        Notifier.notify(0,qsTr("复制成功"))
                    }
                }

                Text {
                    id: textItem
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.text
                    font.pixelSize: 14
                    wrapMode: Text.Wrap   // 自动换行
                }
            }
            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }

    enter: Transition {
        NumberAnimation { property: "scale"; from: 0.0; to: 1.0; duration: 400; easing.type: Easing.OutElastic }
    }

    exit: Transition {
        NumberAnimation { property: "scale"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.OutCubic }
    }
}





