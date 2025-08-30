import QtQuick 2.9
import QtQuick.Window 2.2
import Config 1.0
import QtQuick.Controls
import Global
import Notifier
import LanguageManager
import Data

Window {
    id: root
    visible: true
    width: Screen.width/2
    height: Screen.height/2
    title: "Wuthering Waves Convene Export"

    Header{
        id: header
        width: root.width
        path: Global.path
    }

    ListModel { id: notificationModel }

    // 通知容器（顶部居中）
    Column {
        z: 9999

        id: notificationColumn
        anchors.top: header.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 8
        width: parent.width * 0.6 > 400 ? 400 : parent.width * 0.6
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

    Item{
        id: btnGroup
        Rectangle{
            anchors.fill:parent
            color:"yellow"
        }
        anchors.top: header.bottom
        anchors.horizontalCenter: root.horizontalCenter

        width: root.width
        height: 60

        MyButton{
            id: btn
            width: 100
            height: 40

            anchors.top: parent.top
            anchors.left:parent.left
            anchors.margins: 5
            onClick: {
                notificationModel.append({ "text": "测试文本", "duration": 3000, "color":"orange"})
                btn.disabled = true
                Data.update_data(1)
                loading.visible = true
            }
            usedText: qsTr("更新数据")
        }
        Item{
            id: loading
            visible: false
            anchors.top:btn.bottom
            anchors.left: btn.left

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
            color:"lightblue"
        }
        BarChart{
            id: barChart
            path: Global.path
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height - row.height
            width: contentWidth > parent.width ? parent.width : contentWidth
            chartClip: contentWidth > parent.width ? true : false
            //clip: true
        }

        ButtonGroup {
            id: buttonGroup
        }

        Row {
            //visible:false
            anchors.top:barChart.bottom
            anchors.horizontalCenter: barChart.horizontalCenter
            id: row
            anchors.bottom:root.bottom
            property int lastclick: 0
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
            color:"red"
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
            console.log("错误的输入");
        }
        function onQUpdateComplete(){
            btn.disabled = false
            loading.visible = false
            updateData()
        }
    }

    ListModel{
        id: myModel
    }

    Component.onCompleted:{
        var gacha_type = Global.gachaType
        for(var i = 0; i < gacha_type["data"].length; i++){
            if(!gacha_type["data"][i]["skip"] || !ConfigManager.getValue("skip")){
                myModel.append({"key":gacha_type["data"][i]["key"], "name":LanguageManager.getValue(gacha_type["data"][i]["name"])})
            }
        }
        initData(myModel.get(0)["key"],myModel.get(0)["name"])
    }

    function initData(key,name){
        barChart.key = key
        barChart.gacha_data.clear()
        barChart.chartTitle = name
        var gacha_data = Data.getBarChartData(key)
        for(var i = 0; i < gacha_data.length; i++){
            barChart.gacha_data.append({"ItemName":gacha_data[i]["ItemName"],"source":Global.path +"/resource/" +gacha_data[i]["source"] + ".png","count":gacha_data[i]["count"],"isOffTarget":gacha_data[i]["isOffTarget"]})
        }
    }

    function updateData(){
        var gacha_data = Data.getBarChartData(barChart.key)
        for(var i = 0; i < gacha_data.length; i++){
            if(i>=barChart.gacha_data.count){
                //多余部分
                barChart.gacha_data.append({"ItemName":gacha_data[i]["ItemName"],"source":Global.path +"/resource/" +gacha_data[i]["source"] + ".png","count":gacha_data[i]["count"],"isOffTarget":gacha_data[i]["isOffTarget"]})
                continue;
            }
            var item = barChart.gacha_data.get(i)
            //全部相等，继续遍历
            if(item.ItemName == gacha_data[i]["ItemName"] && item.source == Global.path +"/resource/" +gacha_data[i]["source"] + ".png" && item.count == gacha_data[i]["count"] && item.isOffTarget == gacha_data[i]["isOffTarget"]){
                continue;
            }
            else{
                    //最后一项
                    barChart.gacha_data.setProperty(i,"source",Global.path +"/resource/" +gacha_data[i]["source"] + ".png")
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
