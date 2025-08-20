import QtQuick 2.9
import QtQuick.Window 2.2
import App 1.0
import QtQuick.Controls
import Global

Window {
    id: root
    visible: true
    width: Screen.width/2
    height: Screen.height/2
    title: "Wuthering Waves Convene Export"
    Text {
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.bold: true
        font.pointSize: 42
        text: "Hello World!"
        visible: false
    }
    Header{
        id: header
        width: root.width
    }
    Component.onCompleted:{
        var obj = Global.gachaType
        obj = JSON.parse( JSON.stringify(obj))
        console.log(typeof obj) 
        console.log(JSON.stringify(obj)) 
console.log(obj["data"])
        var list = obj.data
        console.log(Array.isArray(list)) 

        for (var i = 0; i < list.length; ++i) {
            console.log(list[i].name) 
        }
    }
}
