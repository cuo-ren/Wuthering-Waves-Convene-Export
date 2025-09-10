import QtQuick 2.9
import QtQuick.Window 2.2
import ConfigManager 1.0
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import Global
import Notifier
import LanguageManager
import Data
import Path

Window {
    id: root
    visible: true
    width: Screen.width/2
    height: Screen.height/2
    title: "Wuthering Waves Convene Export"

    Header{
        id: header
        width: root.width
        //parent: Overlay.overlay
        path: Global.path
    }

    ListModel { id: notificationModel }
    //设置窗口

    SettingPopup{
        id: popup
        parentWidth: root.width
        parentHeight: root.width - header.height
        parentX: 0
        parentY: header.height
        modal: false
        onRefresh: {
            initButtonGroup()
            if(myModel.count != 0){
                initData(myModel.get(0)["key"],myModel.get(0)["name"])
            }
        }
        onGamePathChanged: {
            if(Path.validatePath(ConfigManager.getValue("path"))){
                updateBtn.flag = true
            }
            else{
                updateBtn.flag = false
            }
        }
    }

    // 通知容器（顶部居中）
    Column {
        z: 9999
        parent: Overlay.overlay

        x: (root.width - width) / 2
        y: header.y + header.height

        id: notificationColumn
        //anchors.top: header.bottom
        //anchors.horizontalCenter: parent.horizontalCenter
        spacing: 8
        width: root.width * 0.6 > 400 ? 400 : root.width * 0.6
        padding: 10

        Repeater {
            model: notificationModel
            delegate: NotificationItem {
                // 把 model 的 role 映射到组件属性上（更稳妥）
                notifactiontext: model.text
                notifactioncolor: model.color
                duration: model.duration
                path: Global.path
                enterDelay: index * 80   // 每条错开 80ms 入场
                onClosed: notificationModel.remove(index)
            }
        }
    }

    Connections{
        target: Notifier;
        function onMessageOccurred(mode,message){
            switch(mode){
                case 0: notificationModel.append({ "text": message, "duration": 3000, "color":"green"});break;
                case 1: notificationModel.append({ "text": message, "duration": 3000, "color":"blue"}); break;
                case 2: notificationModel.append({ "text": message, "duration": 3000, "color":"orange"}); break;
                case 3: notificationModel.append({ "text": message, "duration": 3000, "color":"red"}); break;
                default:break;
            }
        }
    }

    FolderDialog {
        id: folderDialog
        title: qsTr("选择文件夹")
        onAccepted: {
            var path = folderDialog.selectedFolder.toString().replace("file:///", "")
            console.log("选择的文件夹:", path)
            if(Path.validatePath(path)){
                notificationModel.append({"text": qsTr("已定位到游戏日志"), "duration": 3000, "color":"green"})
                ConfigManager.setValue("path", path)
                popup.updatePath()
            }else{
                notificationModel.append({"text": qsTr("未定位到游戏日志"), "duration": 3000, "color":"orange"})
            }
        }
    }

    Image{
        anchors.top: header.bottom
        anchors.left: parent.left
        width:parent.width
        height:parent.height - header.height

        source: "../resource/bg.jpg"
    }

    Item{
        id: btnGroup
        Rectangle{
            anchors.fill:parent
            color:"transparent"//color:"yellow"
        }
        anchors.top: header.bottom
        anchors.horizontalCenter: root.horizontalCenter

        width: root.width
        height: 70

        MyButton{
            id: updateBtn
            width: 100
            height: 40
            radius: 5

            anchors.top: parent.top
            anchors.left:parent.left
            anchors.topMargin: 10
            anchors.leftMargin: 20

            commonFillColor: "#ECF5FF"
            commonBorderColor: "#409eff"
            commonTextColor: "#409eff"

            hoverFillColor: "#409eff"
            hoverBorderColor: "#409eff"

            property bool flag: false
            text: flag ? qsTr("更新数据") : qsTr("查找游戏")

            onClicked: {
                if(flag){
                    updateBtn.disabled = true
                    loading.visible = true
                    loadingImage.start()
                    Data.update_data(1)
                }
                else{
                    if(Path.findGameLog()){
                        flag = true
                        popup.updatePath()
                    }else{
                        folderDialog.open()
                    }
                }
            }

            Component.onCompleted: {
                if(Path.validatePath(ConfigManager.getValue("path"))){
                    flag = true
                }
                else{
                    flag = false
                }
            }
        }

        HoverDropdownButton{
            id: exportBtn
            width: 100
            height: 40
            radius: 5

            menuItemHeight: 28
            arrowSize: 8

            text: qsTr("导出数据")

            anchors.top: parent.top
            anchors.left: updateBtn.right
            anchors.topMargin: 10
            anchors.leftMargin: 10

            bgCommonColor: "#F0F9EF"
            borderCommonColor: "#67c23a"
            textCommonColor: "#67c23a"
            menuBgCommonColor: "#F0F9EF"
            menuBorderCommonColor: "#67c23a"

            bgHoverColor: "#67c23a"
            borderHoverColor: "#67c23a"
            itemHoverColor:"#67c23a"
            textHoverColor:"white"

            itemPressedColor:Qt.darker("#67c23a")

            menuModel: ListModel {
                ListElement { text: qsTr("导出为excel")}
                ListElement { text: qsTr("导出为csv") }
                ListElement { text: qsTr("导出为UIGF3") }
                ListElement { text: qsTr("导出为UIGF4") }
            }

            onClicked: {
                exportBtn.disabled = true
                Data.exportToExcel()
            }

            onTriggered: function(index, item) {
                exportBtn.disabled = true
                switch(index){
                    case 0:Data.exportToExcel();break;
                    case 1:Data.exportToCsv();break;
                    case 2:Data.exportToUIGF3();break;
                    case 3:Data.exportToUIGF4(false);break;
                }
            }
        }
        HoverDropdownButton{
            id: settingbtn
            width: 60
            height: 40
            arrowSize: 5
            menuWidth: 100

            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 10
            anchors.rightMargin: 20

            text: qsTr("设置")

            menuModel:ListModel{
                ListElement{ text : qsTr("使用url更新数据")}
            }

            bgCommonColor: "lightgrey"
            borderCommonColor: "grey"
            textCommonColor: "grey"
            menuBgCommonColor: bgCommonColor
            menuBorderCommonColor: borderCommonColor

            borderHoverColor: "grey"
            textHoverColor: "white"

            menuBgHoverColor: menuBorderCommonColor
            menuBorderHoverColor: "grey"
            itemHoverColor: "grey"

            onClicked: {
                popup.open()
            }

            onTriggered: function(index, item) {
                switch(index){
                    case 0:urlInputPopup.open();break;
                }
            }

            TextInputPopup{
                id: urlInputPopup
                parent:Overlay.overlay
                width: root.width/2
                height:root.height/2
                onAccepted:(text)=> {
                    console.log(text)
                    updateBtn.disabled = true
                    loading.visible = true
                    loadingImage.start()
                    Data.update_data(2,text)
                }
            }
        }
        MyComboBox{
            id: uidList
            width: 150
            height: 25

            //anchors.top: parent.top
            anchors.right: settingbtn.left
            //anchors.topMargin: 10
            anchors.verticalCenter: settingbtn.verticalCenter
            anchors.rightMargin: 10

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
            property string lastUid: ""

            ListModel{
                id: uidModel
            }

            textRole: "uid"
            model: uidModel

            onActivated:{
                if(currentText != lastUid){
                    lastUid = currentText
                    ConfigManager.setValue("active_uid",currentText)
                    console.log("切换uid为 " + currentText)
                    initButtonGroup()
                    if(myModel.count != 0){
                        initData(myModel.get(0)["key"],myModel.get(0)["name"])
                    }
                }
            }

            Component.onCompleted: {
                initUidList(ConfigManager.getValue("active_uid"))
            }

            function initUidList(uid){
                uidModel.clear()
                var uidlist = Data.getUidList()
                for(var i = 0; i < uidlist.length; i++){
                    uidModel.append({"uid": uidlist[i]})
                }
                if(uidlist.length == 0){
                    if(uid.length){
                        console.log("切换uid为空")
                        ConfigManager.setValue("active_uid","")
                        lastUid = ""
                    }
                    return
                }
                if(uidlist.includes(uid)){
                    currentIndex = uidlist.indexOf(uid)
                    lastUid = uid
                }else{
                    currentIndex = 0
                    ConfigManager.setValue("active_uid",uidlist[0])
                    lastUid = uidlist[0]
                }
            }
        }

        Item{
            id: loading
            visible: false
            anchors.top: updateBtn.bottom
            anchors.left: updateBtn.left
            anchors.margins: 3

            height: 10
            Loading {
                usedColor: "black"
                id: loadingImage
                width: parent.height
                height: parent.height
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 5
            }
            Text {
                height: 50
                id: loadingText
                anchors.left: loadingImage.right
                anchors.horizontalCenter: loadingImage.horizontalCenter
                anchors.margins: 5
                text: qsTr("正在加载")
            }
        }
    }

    Item{
        id: chartArea
        anchors.top: btnGroup.bottom
        anchors.left: pageArea.right
        clip: true

        width: root.width - pageArea.width
        height: root.height - header.height - btnGroup.height
        Rectangle{
            anchors.fill:parent
            color:"transparent"//color:"lightblue"
        }
        BarChart{
            id: barChart
            path: Global.path + "/resource/"
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height - row.height - 10
            width: contentWidth > parent.width ? parent.width : contentWidth
            chartClip: contentWidth > parent.width ? true : false
            //clip: true
        }

        ButtonGroup {
            id: buttonGroup
        }

        Row {
            //visible:false
            id: row
            property int lastclick: 0

            //anchors.top:barChart.bottom
            anchors.horizontalCenter: barChart.horizontalCenter
            anchors.bottom: chartArea.bottom
            anchors.margins: 10

            Repeater{
                model: myModel
                RadioButton {
                    text: model.name
                    ButtonGroup.group: buttonGroup
                    checked: index==0?true:false
                    onClicked: {
                        if(index != row.lastclick){
                            initData(model.key,model.name)
                            row.lastclick = index
                        }
                    }
                }
            }
        }
    }

    Item{
        id: pageArea
        width: 50
        height: root.height - header.height
        Rectangle{
            anchors.fill:parent
            color:"transparent"//color:"red"
        }
        anchors.left: root.left
        anchors.top: btnGroup.bottom
    }

    Connections{
        target: Data
        function onProssessChanged(text){
            loadingText.text = text;
        }
        function onWrongInput(){
            Notifier.notify(2, qsTr("输入的url有误"))
            updateBtn.disabled = false
            loadingImage.stop()
            loading.visible = false
        }
        function onQUpdateComplete(){
            updateBtn.disabled = false
            loadingImage.stop()
            loading.visible = false
            updateData()
        }
        function onUpdateFail(){
            updateBtn.disabled = false
            loadingImage.stop()
            loading.visible = false
        }
        function onLogNotFound(){
            Notifier.notify(2, qsTr("找不到游戏日志"))
            updateBtn.disabled = false
            updateBtn.flag = false
            loadingImage.stop()
            loading.visible = false
        }
        function onUidChanged(uid){
            uidList.initUidList(uid)
        }
        function onExportCompleted(){
            exportBtn.disabled = false
            Notifier.notify(0, qsTr("导出成功"))
        }
        function onExportFail(){
            exportBtn.disabled = false
        }
    }

    ListModel{
        id: myModel
    }

    Component.onCompleted:{
        initButtonGroup()
        if(myModel.count != 0){
            initData(myModel.get(0)["key"],myModel.get(0)["name"])
        }
    }

    function initButtonGroup(){
        myModel.clear()

        var gacha_type = Global.gachaType
        for(var i = 0; i < gacha_type["data"].length; i++){
            var gacha_data = Data.getBarChartData(gacha_type["data"][i]["key"])
            if(gacha_data.length != 0 && (!gacha_type["data"][i]["skip"] || !ConfigManager.getValue("skip"))){
                myModel.append({"key":gacha_type["data"][i]["key"], "name":LanguageManager.getValue(gacha_type["data"][i]["name"])})
            }
        }
    }

    function initData(key,name){
        barChart.key = key
        barChart.gacha_data.clear()
        barChart.chartTitle = name
        var gacha_data = Data.getBarChartData(key)
        for(var i = 0; i < gacha_data.length; i++){
            barChart.gacha_data.append({"ItemName":gacha_data[i]["ItemName"],"source":gacha_data[i]["source"] + ".png","count":gacha_data[i]["count"],"isOffTarget":gacha_data[i]["isOffTarget"]})
        }
    }

    function updateData(){
        //重新加载
        initButtonGroup()
        if(myModel.count == 0){
            //更新后还是无数据，之间返回
            return
        }
        if(barChart.key == "0"){
            //之前没数据但现在有了
            initData(myModel.get(0)["key"],myModel.get(0)["name"])
        }
        var gacha_data = Data.getBarChartData(barChart.key)
        for(var i = 0; i < gacha_data.length; i++){
            if(i>=barChart.gacha_data.count){
                //多余部分
                barChart.gacha_data.append({"ItemName":gacha_data[i]["ItemName"],"source":gacha_data[i]["source"] + ".png","count":gacha_data[i]["count"],"isOffTarget":gacha_data[i]["isOffTarget"]})
                continue;
            }
            var item = barChart.gacha_data.get(i)
            //全部相等，继续遍历
            if(item.ItemName == gacha_data[i]["ItemName"] && item.source == gacha_data[i]["source"] + ".png" && item.count == gacha_data[i]["count"] && item.isOffTarget == gacha_data[i]["isOffTarget"]){
                continue;
            }
            else{
                //最后一项
                barChart.gacha_data.setProperty(i,"source",gacha_data[i]["source"] + ".png")
                barChart.gacha_data.setProperty(i,"ItemName",gacha_data[i]["ItemName"])
                barChart.gacha_data.setProperty(i,"count",gacha_data[i]["count"])
                barChart.gacha_data.setProperty(i,"isOffTarget",gacha_data[i]["isOffTarget"])
                //如果有别的数据，清除
                for(var j = i + 1; j < barChart.gacha_data.count; j++){
                    barChart.gacha_data.remove(i+1,1)
                }
                continue;
            }
        }
    }
}
/*
var obj = Global.gachaType
obj = JSON.parse( JSON.stringify(obj))
console.log(typeof obj)
console.log(JSON.stringify(obj))
console.log(obj["data"])
var list = obj.data
console.log(Array.isArray(list))

for (var i = 0; i < list.length; ++i) {
    console.log(list[i].name)
}*/
