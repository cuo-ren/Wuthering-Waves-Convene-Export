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
import Update

Window {
    id: root
    visible: true
    width: Screen.width/2
    height: Screen.height/2
    title: qsTr("Wuthering Waves Convene Export")

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
        parentHeight: root.height - header.height
        parentX: 0
        parentY: header.height
        modal: false
        onRefresh: {
            initButtonGroup()
            barChart.maxCount = !ConfigManager.getValue("hiddenStandardItem") ? 80 : 160
            if(myModel.count != 0){
                initBarChartData(myModel.get(0)["key"],myModel.get(0)["name"])
                initInfoData(myModel.get(0)["key"],myModel.get(0)["name"])
            }
            else{
                barChart.key = "0"
                barChart.gacha_data.clear()
                barChart.chartTitle = ""
            }
        }
        onGamePathChanged: {
            if(Path.validatePath(ConfigManager.getValue("path"))){
                updateBtn.flag = true
            }
            else{
                updateBtn.flag = false
                Notifier.notify(2,qsTr("未找到游戏日志"))
            }
        }
        onExportData: {
            exportBtn.disabled = true
            exportFolderDialog.exportMode = "uigf4total"
            if(ConfigManager.getValue("exportToDefaultPath")){
                Data.exportData(exportFolderDialog.exportMode)
            }
            else{
                exportFolderDialog.open()
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
                source: model.icon
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
                case 0: notificationModel.append({ "text": message, "duration": 3000, "color":"green", "icon":"../resource/success.svg"});break;
                case 1: notificationModel.append({ "text": message, "duration": 3000, "color":"blue", "icon":"../resource/info.svg"}); break;
                case 2: notificationModel.append({ "text": message, "duration": 3000, "color":"orange", "icon":"../resource/warning.svg"}); break;
                case 3: notificationModel.append({ "text": message, "duration": 3000, "color":"red", "icon":"../resource/error.svg"}); break;
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
                Notifier.notify(0,qsTr("成功定位到游戏日志"))
                ConfigManager.setValue("path", path)
                popup.updatePath()
            }else{
                Notifier.notify(2,qsTr("未找到游戏日志"))
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
            color:"transparent"
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
/*
            source: "../resource/refresh.svg"
*/
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
                    Notifier.notify(2,qsTr("未找到游戏日志"))
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

            FolderDialog {
                id: exportFolderDialog
                title: qsTr("选择导出的位置")
                property string exportMode: ""
                onAccepted: {
                    var path = exportFolderDialog.selectedFolder.toString().replace("file:///", "")
                    console.log("选择的文件夹:", path, "模式:", exportMode)
                    switch(exportMode) {
                        case "excel": Data.exportData("excel", path); break;
                        case "csv":   Data.exportData("csv", path);   break;
                        case "uigf3": Data.exportData("uigf3", path); break;
                        case "uigf4": Data.exportData("uigf4", path); break;
                        case "uigf4total": Data.exportData("uigf4total", path); break;
                    }
                }
                onRejected: {
                    exportBtn.disabled = false
                }
            }

            onClicked: {
                exportBtn.disabled = true
                exportFolderDialog.exportMode = "excel"
                if(ConfigManager.getValue("exportToDefaultPath")){
                    Data.exportData(exportFolderDialog.exportMode)
                }
                else{
                    exportFolderDialog.open()
                }
            }

            onTriggered: function(index, item) {
                exportBtn.disabled = true
                console.log("开始导出数据 模式:",index)
                switch(index){
                    case 0:exportFolderDialog.exportMode = "excel";break;
                    case 1:exportFolderDialog.exportMode = "csv";break;
                    case 2:exportFolderDialog.exportMode = "uigf3";break;
                    case 3:exportFolderDialog.exportMode = "uigf4";break;
                }
                if(ConfigManager.getValue("exportToDefaultPath")){
                    Data.exportData(exportFolderDialog.exportMode)
                }
                else{
                    exportFolderDialog.open()
                }
            }
        }

        HoverDropdownButton{
            id: settingbtn
            width: 60
            height: 40
            arrowSize: 5
            menuWidth: 100
            menuItemHeight: 28

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
                width: root.width/3*2
                height:root.height/3*2
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
                        initBarChartData(myModel.get(0)["key"],myModel.get(0)["name"])
                        initInfoData(myModel.get(0)["key"],myModel.get(0)["name"])
                    }
                    else{
                        barChart.key = "0"
                        barChart.gacha_data.clear()
                        barChart.chartTitle = ""
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

    MySwipeView{
        id: swipeview

        anchors.top: btnGroup.bottom
        anchors.left: pageArea.right

        width: root.width - pageArea.width
        height: root.height - header.height - btnGroup.height

        orientation: Qt.Vertical
        interactive: false
        wheelEnabled: false
        clip: true

        Item{
            id: chartArea

            clip: true

            BarChart{
                id: barChart
                path: Global.path + "/resource/"
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height - row.height - 10
                width: contentWidth > parent.width ? parent.width : contentWidth
                chartClip: contentWidth > parent.width ? true : false
                maxCount: !ConfigManager.getValue("hiddenStandardItem") ? 80 : 160
                //clip: true
            }

            ButtonGroup {
                id: buttonGroup
            }

            Row {
                id: row
                property int lastclick: 0

                height: 25

                //anchors.top:barChart.bottom
                anchors.horizontalCenter: barChart.horizontalCenter
                anchors.bottom: chartArea.bottom
                anchors.margins: 10

                spacing: 10

                Repeater{
                    model: myModel
                    MyRadioButton {
                        text: model.name
                        ButtonGroup.group: buttonGroup
                        checked: index==0?true:false

                        commonBgColor: "transparent"
                        pressBgColor: Qt.darkGray
                        activeBgColor: "grey"

                        commonTextColor: "grey"
                        pressTextColor: "white"
                        activeTextColor: "white"

                        onClicked: {
                            if(index != row.lastclick){
                                initBarChartData(model.key,model.name)
                                row.lastclick = index
                                console.log("切换卡池","key:",model.key,"name:",model.name)
                            }
                        }
                    }
                }
            }
        }
        Item{
            ListModel{
                id: infoListmodel
                ListElement{data:0}//不歪概率
                ListElement{data:0}//平均出金抽数
                ListElement{data:0}//平均限定抽数
                ListElement{data:0}//总抽数
            }

            Text {
                id: infoText
                property string title: qsTr("标题")
                text: qsTr("%1\n平均出金抽数%2\n平均限定抽数%3\n不歪概率%4%\n总抽数%5\n").arg(title).arg(infoListmodel.get(1).data).arg(infoListmodel.get(2).data).arg(infoListmodel.get(0).data.toFixed(2)).arg(infoListmodel.get(3).data)
            }
        }
    }

    Item{
        id: pageArea
        width: 50
        height: root.height - header.height
        anchors.left: root.left
        anchors.top: header.bottom
        Column{
            id: pageButton
            property int lastclick: 0

            anchors.centerIn: parent
            width: parent.width
            height: childrenRect.height

            MyRadioImageButton{
                width: parent.width - 5
                height: parent.width - 5
                commonBgColor: "transparent"
                pressBgColor: "#00FF00"
                activeBgColor: "transparent"

                commonBorderColor: "transparent"
                pressBorderColor: "#00FF00"
                activeBorderColor: "#00FF00"

                iconNormal: "../resource/barChart.svg"
                iconChecked: "../resource/barChart.svg"

                checked: true

                onClicked: {
                    if(pageButton.lastclick != 0){
                        swipeview.setIndex(0)
                        pageButton.lastclick = 0
                        console.log("切换至柱状图页面")
                    }
                }
            }/*
            MyRadioImageButton{
                width: parent.width - 5
                height: parent.width - 5

                commonBgColor: "transparent"
                pressBgColor: "#00FF00"
                activeBgColor: "transparent"

                commonBorderColor: "transparent"
                pressBorderColor: "#00FF00"
                activeBorderColor: "#00FF00"

                iconNormal: "../resource/infoLightBlue.svg"
                iconChecked: "../resource/infoLightBlue.svg"

                onClicked: {
                    if(pageButton.lastclick != 1){
                        swipeview.setIndex(1)
                        pageButton.lastclick = 1
                        console.log("切换至数据页面")
                    }
                }
            }*/
        }
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
            initBarChartData(myModel.get(0)["key"],myModel.get(0)["name"])
            initInfoData(myModel.get(0)["key"],myModel.get(0)["name"])
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

    function initBarChartData(key,name){
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
            barChart.key = "0"
            barChart.gacha_data.clear()
            barChart.chartTitle = ""
            return
        }
        if(barChart.key == "0"){
            //之前没数据但现在有了
            initBarChartData(myModel.get(0)["key"],myModel.get(0)["name"])
            initInfoData(myModel.get(0)["key"],myModel.get(0)["name"])
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
                if(i+1<barChart.gacha_data.count){
                    barChart.gacha_data.remove(i+1,barChart.gacha_data.count-i-1)
                }
                continue;
            }
        }
    }

    function initInfoData(key,name){
        infoText.title = name
        var l = Data.getInfoData(key)
        for(var i = 0; i < l.length; i++){
            infoListmodel.setProperty(i,"data",l[i])
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
