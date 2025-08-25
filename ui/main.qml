import QtQuick 2.9
import QtQuick.Window 2.2
import Config 1.0
import QtQuick.Controls
import Global
import Error
import LanguageManager
import Data

Window {
    id: root
    visible: true
    width: Screen.width/2
    height: Screen.height/2
    title: "Wuthering Waves Convene Export"
    Button{
        width:50
        height:50
        onClicked: root.initData("1",myModel.get(0).name)
        text: qsTr("更新数据")
    }
    Header{
        id: header
        width: root.width
        path: Global.path
    }
    Component.onCompleted:{
        var gacha_type = Global.gachaType
        for(var i = 0; i < gacha_type["data"].length; i++){
            if(!gacha_type["data"][i]["skip"] || true){
                myModel.append({"key":gacha_type["data"][i]["key"], "name":LanguageManager.getValue(gacha_type["data"][i]["name"])})
            }
        }
    }
    Rectangle{
        x:50
        y:50
        height: root.height - 130
        width: root.width - 60
        color:"lightblue"
    }
    BarChart{
        x:50
        y:50
        id: barChart
        path: Global.path
        height: root.height - 130
        width: root.width - 60
    }

    ListModel{
        id: myModel
    }

    ButtonGroup {
        id: buttonGroup
    }

    Row {
        //visible:false
        y:500
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
        Component.onCompleted: initData("1",myModel.get(0)["name"])
    }
    function initData(key,name){
        barChart.gacha_data.clear()
        barChart.chartTitle = name
        var gacha_data = Data.getBarChartData(key)
        for(var i = 0; i < gacha_data.length; i++){
            barChart.gacha_data.append({"ItemName":gacha_data[i]["ItemName"],"source":Global.path +"/resource/" +gacha_data[i]["source"] + ".png","count":gacha_data[i]["count"],"isOffTarget":gacha_data[i]["isOffTarget"]})
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
