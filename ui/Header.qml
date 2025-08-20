import QtQuick 2.3

Item {
    id: header
    width: 600
    height: 32
    Text {
        id: title
        text: qsTr("鸣潮唤取记录导出分析工具")

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        font.pixelSize: 16
    }

    Item{
        //关闭按钮
        id: closeButton
        width: header.height
        height: header.height
        anchors.right: header.right
        anchors.verticalCenter: header.verticalCenter
        anchors.margins: 5

        Rectangle {
            id: closehoverOverlay
            anchors.fill: parent
            radius: width / 2
            color: "black"//"#00000033"  // 半透明黑色
            opacity:0.3
            visible: false
        }

        Image{
            id:closeButtonImage
            anchors.fill:parent
            source:"../resource/closebtn.svg"
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

        MouseArea{
            anchors.fill:parent
            hoverEnabled: true
            onClicked: {Qt.quit()}
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

    Item{
        //最小化按钮按钮
        id: minimizedButton
        width: header.height
        height: header.height
        anchors.right: closeButton.left
        anchors.verticalCenter: header.verticalCenter
        anchors.margins: 5

        Rectangle {
            id: minimizedhoverOverlay
            anchors.fill: parent
            radius: width / 2
            color: "black"//"#00000033"  // 半透明黑色
            opacity: 0.3
            visible: false
        }

        Image{
            id: minimizedButtonImage
            anchors.fill:parent
            source:"../resource/minimized.svg"
            fillMode: Image.PreserveAspectFit
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            onClicked: {
                root.showMinimized()
            }

            onEntered: {
                minimizedhoverOverlay.visible = true
            }
            onExited: {
                minimizedhoverOverlay.visible = false
            }
            onPressed: minimizedhoverOverlay.opacity = 0.2
            onReleased: minimizedhoverOverlay.opacity = 0.3
        }
    }
}


