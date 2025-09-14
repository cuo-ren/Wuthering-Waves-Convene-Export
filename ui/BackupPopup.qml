import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Data

Popup {
    id: dataManager
    anchors.centerIn: parent
    width: parent.width * 3 / 4
    height: parent.height * 3 / 4
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property ListModel data: ListModel{}
    property string title:qsTr("标题")

    background: Image{
        source: "../resource/bg.jpg"
    }

    enter: Transition {
        NumberAnimation { property: "scale"; from: 0.0; to: 1.0; duration: 400; easing.type: Easing.OutElastic }
    }

    exit: Transition {
        NumberAnimation { property: "scale"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.OutCubic }
    }

    MouseArea{
        z: -1
        hoverEnabled: true
        anchors.fill:table
        onEntered: {
            dataList.hoveredColumn = -1
            dataList.hoveredRow =-1
        }
    }
    ColumnLayout {
        id: table
        anchors.fill: parent
        anchors.margins: 10
        spacing: 0

        // 定义列宽
        property real col1Width: (width - col4Width - col5Width)/10*3.5//文件名
        property real col2Width: (width - col4Width - col5Width)/10*4//日期
        property real col3Width: (width - col4Width - col5Width)/10*2.5//uid
        property real col4Width: width * 0.15 > 100 ? 100 : width * 0.15//状态
        property real col5Width: width * 0.2 > 150 ? 150 : width * 0.2//操作

        Item{Layout.fillWidth: true;height:1}

        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            Text {
                text: dataManager.title
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
                    onClicked: { dataManager.close() }
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
        Item{Layout.fillWidth: true;height:10}

        // 表头
        Row {
            width: parent.width
            height: 40
            spacing: 0

            Rectangle {
                width: table.col1Width
                height: parent.height
                border.width: 1
                border.color: "grey"
                color: "#e0e0e0"
                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("文件名称"); font.bold: true
                }
            }
            Rectangle {
                width: table.col2Width
                height: parent.height
                border.width: 1
                border.color: "grey"
                color: "#e0e0e0"
                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("备份时间"); font.bold: true
                }
            }
            Rectangle {
                width: table.col3Width
                height: parent.height
                border.width: 1
                border.color: "grey"
                color: "#e0e0e0"
                Label {
                    anchors.centerIn: parent
                    text: qsTr("UID"); font.bold: true
                }
            }
            Rectangle {
                width: table.col4Width
                height: parent.height
                border.width: 1
                border.color: "grey"
                color: "#e0e0e0"
                Label {
                    anchors.centerIn: parent
                    text: qsTr("数据状态"); font.bold: true
                }
            }
            Rectangle {
                width: table.col5Width
                height: parent.height
                border.width: 1
                border.color: "grey"
                color: "#e0e0e0"
                Label {
                    anchors.centerIn: parent
                    text: qsTr("操作"); font.bold: true
                }
            }
        }

        ListView {
            id: dataList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 0
            boundsBehavior: Flickable.StopAtBounds

            property int hoveredRow: -1
            property int hoveredColumn: -1

            //ScrollBar.vertical: ScrollBar { policy: ScrollBar.OnDemand }

            model: dataManager.data

            delegate: Row {
                id: rowItem
                Layout.fillWidth: true
                //width: parent.width
                height: 40
                spacing: 0

                Rectangle {
                    width: table.col1Width
                    height: parent.height
                    color: dataList.hoveredRow === index || dataList.hoveredColumn === 0 ? "lightgrey" : index % 2 === 1 ? "#f0f0f0" : "#ffffff"
                    border.width: 1
                    border.color: "grey"
                    Label {
                        anchors.left: parent.left;
                        anchors.leftMargin: 20
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.name
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            dataList.hoveredRow = index
                            dataList.hoveredColumn = 0
                        }

                    }
                }

                Rectangle {
                    width: table.col2Width
                    height: parent.height
                    color: dataList.hoveredRow === index || dataList.hoveredColumn === 1 ? "lightgrey" : index % 2 === 1 ? "#f0f0f0" : "#ffffff"
                    border.width: 1
                    border.color: "grey"
                    Label {
                        anchors.left: parent.left;
                        anchors.leftMargin: 20
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.time
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            dataList.hoveredRow = index
                            dataList.hoveredColumn = 1
                        }

                    }
                }

                Rectangle {
                    width: table.col3Width
                    height: parent.height
                    color: dataList.hoveredRow === index || dataList.hoveredColumn === 2 ? "lightgrey" : index % 2 === 1 ? "#f0f0f0" : "#ffffff"
                    border.width: 1
                    border.color: "grey"
                    ListView {
                        id: uidList
                        clip: true
                        width: parent.width - 5
                        height: parent.height - 5
                        anchors.centerIn: parent
                        model: uids

                        property int itemH: 20

                        delegate: Text {
                            width: parent.width
                            height: uidList.itemH
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            text: uid
                        }

                        // 居中处理
                        header: Item {
                            width: 1
                            height: Math.max(0, Math.floor((uidList.height - uidList.count * uidList.itemH - (uidList.count-1)*uidList.spacing) / 2))
                        }
                        footer: Item {
                            width: 1
                            height: Math.max(0, Math.floor((uidList.height - uidList.count * uidList.itemH - (uidList.count-1)*uidList.spacing) / 2))
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                        onEntered: {
                            dataList.hoveredRow = index
                            dataList.hoveredColumn = 2
                        }

                    }
                }

                Rectangle {
                    width: table.col4Width
                    height: parent.height
                    color: dataList.hoveredRow === index || dataList.hoveredColumn === 3 ? "lightgrey" : index % 2 === 1 ? "#f0f0f0" : "#ffffff"
                    border.width: 1
                    border.color: "grey"
                    Rectangle{
                        width: 40
                        height: 20

                        radius: 4

                        color: status === 0 ? "lightgreen" : status === 1 ? Qt.lighter("orange") : Qt.lighter("red")
                        border.color: status === 0 ? "green" : status === 1 ? "orange" : "red"

                        anchors.centerIn: parent

                        Label {
                            anchors.centerIn: parent;
                            text: status === 0 ? qsTr("正常") : status === 1 ? qsTr("异常") : qsTr("损坏")
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            dataList.hoveredRow = index
                            dataList.hoveredColumn = 3
                        }

                    }
                }

                Rectangle {
                    width: table.col5Width
                    height: parent.height
                    color: dataList.hoveredRow === index || dataList.hoveredColumn === 4 ? "lightgrey" : index % 2 === 1 ? "#f0f0f0" : "#ffffff"
                    border.width: 1
                    border.color: "grey"
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            dataList.hoveredRow = index
                            dataList.hoveredColumn = 4
                        }
                    }
                    Row {
                        anchors.centerIn: parent
                        spacing: 10
                        MyButton {
                            width: 40
                            height: 20

                            radius: 4

                            commonFillColor: "#F0F9EF"
                            commonBorderColor: "#67c23a"
                            commonTextColor: "#67c23a"

                            hoverFillColor: "#67c23a"
                            hoverBorderColor: "#67c23a"

                            text: qsTr("恢复");
                            onClicked: {

                                console.log("恢复 " + time)
                                if(status !=0 ){
                                    console.log("异常数据不能恢复")
                                }
                            }
                        }
                        MyButton {
                            width: 40
                            height: 20

                            radius: 4

                            commonFillColor: Qt.lighter("red")
                            commonBorderColor: "red"
                            commonTextColor: "red"

                            hoverFillColor: "red"
                            hoverBorderColor: "red"
                            text: qsTr("删除");
                            onClicked: console.log("删除 " + time)
                        }
                    }
                }
            }
        }
    }
}
