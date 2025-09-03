import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import ConfigManager 1.0

    Popup {

        FolderDialog {
            id: folderDialog
            title: "选择文件夹"
            //currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
            onAccepted: {
                gameFolderSettingTextField.text = folderDialog.selectedFolder.toString().replace("file:///", "")
                console.log("选择的文件夹:", folderDialog.selectedFolder)
            }
        }

        property int parentWidth: 0
        property int parentHeight: 0
        property int parentX: 0
        property int parentY: 0
        Component.onCompleted: {
            skipSettingSwitch.checked = ConfigManager.getValue("skip")
            updateSettingSwitch.checked = ConfigManager.getValue("update")
        }
        id: settingsPopup
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape

        width: parentWidth
        height: parentHeight

        background: Rectangle {
            color: "#ffffff"
            radius: 8
            border.color: "#cccccc"
        }

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 250; easing.type: Easing.OutCubic }
            //NumberAnimation { property: "scale"; from: 0.0; to: 1.0; duration: 100; easing.type: Easing.OutCubic }
            NumberAnimation { property: "y"; from: parentHeight; to: parentY; duration: 250; easing.type: Easing.OutCubic }
        }

        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 250; easing.type: Easing.OutCubic }
            //NumberAnimation { property: "scale"; from: 0.0; to: 1.0; duration: 100; easing.type: Easing.OutCubic }
            NumberAnimation { property: "y"; from: parentY; to: parentHeight; duration: 250; easing.type: Easing.OutCubic }
        }

        Column{
            width: parent.width
            spacing: 10
            //popup标题区域
            Item{
                id: titlerect
                width: parent.width
                height: childrenRect.height
                Text{
                    id: title
                    text: qsTr("设置面板")
                    font.pixelSize: 24
                    anchors.left: parent.left
                    anchors.margins: 10
                }
                Button{
                    id: closeBtn
                    width: 20
                    height: 20
                    anchors.right:parent.right
                    anchors.margins: 10
                    onClicked: settingsPopup.close()
                }
            }
            //分割线

            Rectangle{
                width:parent.width
                height:1
                color:"lightgrey"
            }


            //设置主体
            Flickable{
                flickableDirection:Flickable.VerticalFlick
                width: parent.width
                height: settingsPopup.height - titlerect.height - 1 - parent.spacing*2
                contentHeight: col.height +10
                clip: true
                Column{
                    id: col
                    width: parent.width
                    spacing: 10
                    Text {
                        id: settingTitle
                        text: qsTr("设置")
                        font.pixelSize: 18
                        anchors.left: parent.left
                        anchors.margins: 40
                    }

                    Item{
                        id: languageSetting
                        width: parent.width
                        height: 20
                        Text {
                            id: languageSettingText
                            font.pixelSize: 14
                            width: 150
                            text: qsTr("语言")

                            horizontalAlignment: Text.AlignRight
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                        }
                        ComboBox{
                            id: languageSettingCombobox
                            model:["简体中文","繁体中文"]
                            anchors.left:languageSettingText.right
                            anchors.margins: 10
                            anchors.verticalCenter: parent.verticalCenter
                            onAccepted: {
                                console.log("6")
                            }
                            onActivated:{
                                console.log("9")
                            }
                        }
                    }


                    Item {
                        id: gameFolderSetting
                        width: parent.width
                        height: 35

                        Text {
                            id: gameFolderSettingText
                            font.pixelSize: 14
                            width: 150
                            text: qsTr("游戏路径")

                            horizontalAlignment: Text.AlignRight
                            anchors.verticalCenter: parent.verticalCenter
                            verticalAlignment:  Text.AlignVCenter

                            anchors.left: parent.left
                        }
                        TextField {
                            id: gameFolderSettingTextField
                            height: 25
                            anchors.left: gameFolderSettingText.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 10
                            width: 300
                            placeholderText: "请输入或选择文件夹路径"
                            verticalAlignment:  Text.AlignVCenter
                        }

                        // “...”按钮触发文件夹选择器
                        Button {
                            anchors.left: gameFolderSettingTextField.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 10
                            text: "..."
                            onClicked: folderDialog.open()
                        }
                    }

                    Item{
                        id: dataSetting
                        width:parent.width
                        height: 35
                        Text {
                            id: dataSettingText
                            font.pixelSize: 14
                            width: 150
                            height: 35
                            text: qsTr("数据管理")
                            anchors.verticalCenter: parent.verticalCenter
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment:  Text.AlignVCenter

                            anchors.left: parent.left
                        }
                        Button{
                            width: 90
                            height: 35
                            anchors.left: dataSettingText.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 10
                        }
                    }

                    Item{
                        id: skipSetting
                        width:parent.width
                        height: 20
                        Text {
                            id: skipSettingText
                            font.pixelSize: 14
                            width: 150
                            text: qsTr("跳过一次性卡池")

                            horizontalAlignment: Text.AlignRight
                            anchors.verticalCenter: parent.verticalCenter
                            verticalAlignment:  Text.AlignVCenter

                            anchors.left: parent.left
                        }
                        Switch {
                            id: skipSettingSwitch
                            anchors.left: skipSettingText.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 10
                            onCheckedChanged: {
                                if(ConfigManager.getValue("skip") != checked){
                                    console.log("修改跳过一次性卡池设置 "+ "当前设置 " + checked)
                                    ConfigManager.setValue("skip",checked)
                                }
                            }
                        }
                    }

                    Item{
                        id: updateSetting
                        width: parent.width
                        height: 35
                        Text {
                            id: updateSettingText
                            font.pixelSize: 14
                            width: 150
                            text: qsTr("自动更新")

                            horizontalAlignment: Text.AlignRight
                            anchors.verticalCenter: parent.verticalCenter
                            verticalAlignment:  Text.AlignVCenter

                            anchors.left: parent.left
                        }
                        Switch {
                            id: updateSettingSwitch
                            anchors.left: updateSettingText.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 10
                            onCheckedChanged: {
                                if(ConfigManager.getValue("update") != checked){
                                    console.log("修改自动更新设置 "+ "当前设置 " + checked)
                                    ConfigManager.setValue("update",checked)
                                }
                            }
                        }
                        Button{
                            width: 90
                            height: 35
                            anchors.left: updateSettingSwitch.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 10
                            text: qsTr("检查更新")
                        }
                    }


                    Item{
                        id: urlSetting
                        width:parent.width
                        height: 35
                        Text {
                            id: urlSettingText
                            font.pixelSize: 14
                            width: 150
                            height: 35
                            text: qsTr("查看url")
                            anchors.verticalCenter: parent.verticalCenter
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment:  Text.AlignVCenter

                            anchors.left: parent.left
                        }
                        Button{
                            width: 90
                            height: 35
                            anchors.left: urlSettingText.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 10
                            onClicked: Qt.application.clipboard.setText("ys.mihoyo.com")
                        }
                    }

                    Rectangle{
                        width:parent.width
                        height:1
                        color:"lightgrey"
                    }

                    Text {
                        id: aboutTitle
                        text: qsTr("关于")
                        font.pixelSize: 18
                        anchors.left: parent.left
                        anchors.margins: 40
                    }

                    Text {
                        id: aboutText
                        //height: 60
                        //height: contentHeight
                        width: parent.width
                        text: qsTr("作者: 氧化铜<br>Github: <a href = 'https://github.com/cuo-ren/Wuthering-Waves-Convene-Export' >github.com/cuo-ren/Wuthering-Waves-Convene-Export</a>")
                        anchors.left: parent.left
                        anchors.margins: 80
                        font.pixelSize: 12
                        linkColor: "blue"
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                        onLinkActivated: Qt.openUrlExternally(link)
                    }

                }
            }
        }

    }

