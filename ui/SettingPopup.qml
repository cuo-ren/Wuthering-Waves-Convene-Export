import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import ConfigManager 1.0
import LanguageManager
import Global
import Path
import Notifier

Popup {
    function updatePath(){
        var path = ConfigManager.getValue("path")
        gameFolderSettingTextField.text = path
    }

    MouseArea{
        anchors.fill: parent
        onClicked: focus = true
    }
    Image{
        anchors.fill:parent
        width:parent.width
        height:parent.height - header.height

        source: "../resource/bg.jpg"
    }
    FolderDialog {
        id: folderDialog
        title: qsTr("选择文件夹")
        //currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        onAccepted: {
            gameFolderSettingTextField.text = folderDialog.selectedFolder.toString().replace("file:///", "")
            console.log("选择的文件夹:", gameFolderSettingTextField.text)
            var path = ConfigManager.getValue("path")
            if(gameFolderSettingTextField.text != path){
                ConfigManager.setValue("path", gameFolderSettingTextField.text)
                settingsPopup.gamePathChanged()
            }
        }
    }

    property int parentWidth: 0
    property int parentHeight: 0
    property int parentX: 0
    property int parentY: 0
    signal refresh()
    signal gamePathChanged()

    padding: 0

    ListModel{
        id: myModel
    }

    Component.onCompleted: {
        //跳过设置
        skipSettingSwitch.checked = ConfigManager.getValue("skip")
        //自动更新设置
        updateSettingSwitch.checked = ConfigManager.getValue("update")

        //语言设置
        var supportLanguages = Global.supportLanguages
        for(var i = 0; i < supportLanguages.length; i++){
            myModel.append({"name": LanguageManager.getValue(supportLanguages[i]), "key": supportLanguages[i]})
        }
        var usedLang = supportLanguages.indexOf(ConfigManager.getValue("language"))
        languageSettingCombobox.currentIndex = usedLang

        //游戏路径设置
        var path = ConfigManager.getValue("path")
        gameFolderSettingTextField.text = path
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

            Item{
                //关闭按钮
                id: closeButton
                width: 30
                height: 30
                anchors.right:parent.right
                anchors.margins: 10

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

                MouseArea{
                    anchors.fill:parent
                    hoverEnabled: true
                    onClicked: {settingsPopup.close()}
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

            MouseArea{
                anchors.fill: parent
                onClicked: focus = true
            }

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
                    MyComboBox{
                        id: languageSettingCombobox
                        height: 25
                        model: myModel
                        textRole: "name"
                        anchors.left:languageSettingText.right
                        anchors.margins: 10
                        anchors.verticalCenter: parent.verticalCenter

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
                                console.log("修改当前语言设置 " + "当前语言 " + currentText)
                                if(!LanguageManager.switchLanguage(myModel.get(currentIndex).key)){
                                    var supportLanguages = Global.supportLanguages
                                    var usedLang = supportLanguages.indexOf(ConfigManager.getValue("language"))
                                    languageSettingCombobox.currentIndex = usedLang
                                }else{
                                    settingsPopup.refresh()
                                }
                            }
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
                    MyTextField {
                        id: gameFolderSettingTextField
                        height: 25
                        anchors.left: gameFolderSettingText.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: 10
                        width: 300
                        placeholderText: qsTr("请输入或选择文件夹路径")
                        verticalAlignment:  Text.AlignVCenter
                        leftPadding: 5

                        selectionColor: "grey"
                        selectedTextColor: "white"

                        background: Rectangle {
                            implicitWidth: 200
                            implicitHeight: 40
                            color: gameFolderSettingTextField.enabled ? "transparent" : "#353637"
                            border.color: gameFolderSettingTextField.enabled ? "darkgrey" : "transparent"
                        }

                        onEditingFinished: {
                            var path = ConfigManager.getValue("path")
                            if(text != path){
                                console.log("修改游戏路径设置 " + "当前设置 " + text)
                                ConfigManager.setValue("path", text)
                                settingsPopup.gamePathChanged()
                            }
                        }
                    }

                    // “...”按钮触发文件夹选择器
                    Button {
                        id: gameFolderSettingOpenBtn
                        height: 25
                        width: 20
                        anchors.left: gameFolderSettingTextField.right
                        anchors.verticalCenter: parent.verticalCenter
                        text: "..."
                        onClicked: folderDialog.open()
                    }

                    Button{
                        id: gameFolderSettingFindBtn
                        width: 90
                        height: 35
                        anchors.left: gameFolderSettingOpenBtn.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: 10

                        text: qsTr("自动查找")

                        onClicked: {
                            if(Path.FindGameLog()){
                                settingsPopup.updatePath()
                                settingsPopup.gamePathChanged()
                            }
                        }
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
                                settingsPopup.refresh()
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

                        text: qsTr("点击获取")

                        TextInput{
                            id: copyUrl
                            width: 0
                            height: 0
                            clip: true
                            visible: false
                        }

                        onClicked: {
                            var urlList = ConfigManager.QgetUrlList()
                            if(urlList.length == 1){
                                copyUrl.text = urlList[0]
                                copyUrl.cursorPosition = 0
                                copyUrl.moveCursorSelection(urlList[0].length, TextInput.SelectCharacters)
                                copyUrl.copy()
                                Notifier.notify(0,qsTr("复制成功"))
                            }
                        }
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
                    text: qsTr("作者：氧化铜<br>版本：%1<br>Github：<a href = 'https://github.com/cuo-ren/Wuthering-Waves-Convene-Export' >github.com/cuo-ren/Wuthering-Waves-Convene-Export</a>").arg(Global.version["version"])
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
