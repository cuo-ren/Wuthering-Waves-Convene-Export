import QtQuick 2.9
import QtQuick.Window 2.2
import App 1.0

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
    }
    Header{
        id: header
        width: root.width
    }
Component.onCompleted:{
    console.log(ConfigManager.getValue("skip"))
    var l =[]
    l = ConfigManager.QgetUrlList()
    console.log(l)
}
    
}
