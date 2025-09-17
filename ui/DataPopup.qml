import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Dialogs
import Data
import Notifier

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
    signal exportData()
    signal refresh()

    background: Image{
        source: "../resource/bg.jpg"
    }

    enter: Transition {
        NumberAnimation { property: "scale"; from: 0.0; to: 1.0; duration: 400; easing.type: Easing.OutElastic }
    }

    exit: Transition {
        NumberAnimation { property: "scale"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.OutCubic }
    }

    Connections{
        target: Data
        function onExportCompleted(){
            exportBtn.disabled = false
        }
        function onExportFail(){
            exportBtn.disabled = false
        }
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
        property real col1Width: width * 0.3 > 100 ? 100 : width * 0.3 //UID
        property real col2Width: (width - col3Width - col4Width - col1Width) //更新时间
        property real col3Width: width * 0.25 > 200 ? 200 : width * 0.25 //时区
        property real col4Width: width * 0.15 > 100 ? 100 : width * 0.15 //操作

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
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            FileDialog {
                id: fileDialog
                title: qsTr("选择文件")
                onAccepted: {
                    var  filepath = selectedFile.toString()
                    filepath = filepath.replace("file:///", "")
                    Data.importUIGF(filepath)
                    var gachaInfo = Data.getDataInfo()
                    dataManager.data.clear()
                    for(var i = 0; i < gachaInfo.length; i++){
                        dataManager.data.append({"uid":gachaInfo[i]["uid"],"time":gachaInfo[i]["time"],"timezone":gachaInfo[i]["timezone"]})
                    }
                    dataManager.refresh()
                }
            }
            MyButton{
                id: importBtn
                width: 80
                height: 32
                radius: 5

                commonFillColor: "#ECF5FF"
                commonBorderColor: "#409eff"
                commonTextColor: "#409eff"

                hoverFillColor: "#409eff"
                hoverBorderColor: "#409eff"

                text: qsTr("导入数据")

                onClicked: {
                    fileDialog.open()
                }
            }

            MyButton{
                id: exportBtn
                width: 80
                height: 32
                radius: 5

                text: qsTr("导出数据")

                commonFillColor: "#F0F9EF"
                commonBorderColor: "#67c23a"
                commonTextColor: "#67c23a"

                hoverFillColor: "#67c23a"
                hoverBorderColor: "#67c23a"

                onClicked: {
                    dataManager.exportData()
                    exportBtn.disabled = true
                }
            }
        }
        Item{Layout.fillWidth: true;height:20}
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
                    text: qsTr("UID"); font.bold: true
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
                    text: qsTr("更新时间"); font.bold: true
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
                    text: qsTr("时区"); font.bold: true
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
                        text: model.uid
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

                    MyComboBox{
                        id: timezoneCombobox
                        height: 25
                        model: [{name:"UTC-5",value:-5},{name:"UTC+1",value:1},{name:"UTC+8",value:8}]
                        textRole: "name"

                        anchors.centerIn: parent

                        property string lastText: ""

                        borderCommonColor: "grey"
                        borderPressedColor: "darkgrey"

                        canvasCommonColor: "grey"
                        canvasPressedColor: "darkgrey"

                        textCommonColor: "black"
                        textPressedColor: "black"

                        popupColor: "lightgrey"
                        popupBorderColor: "grey"

                        highlightColor: "grey"
                        commonColor: "transparent"

                        optionsTextCommonColor: textCommonColor
                        optionsTextPressedColor: textPressedColor

                        onActivated:{
                            if(currentText != lastText){
                                lastText = currentText
                                console.log("更改时区设置 ",currentText)
                                Data.setTimezone(uid,timezoneCombobox.model[currentIndex].value)
                            }
                        }

                        Component.onCompleted: {
                            for (var i = 0; i < model.length; ++i) {
                                    if (model[i].value === timezone) {
                                        timezoneCombobox.currentIndex = i
                                        timezoneCombobox.lastText = model[i].name
                                        break
                                    }
                                }
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

                            commonFillColor: Qt.lighter("red")
                            commonBorderColor: "red"
                            commonTextColor: "red"

                            hoverFillColor: "red"
                            hoverBorderColor: "red"
                            text: qsTr("删除");
                            onClicked: {
                                Data.deleteUid(uid)
                                console.log("删除UID " + uid)
                                dataManager.data.remove(index,1)
                                dataManager.refresh()
                            }
                        }
                    }
                }
            }
        }
    }
}
