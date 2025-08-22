import QtQuick 2.9
import QtQuick.Window 2.2
import Config 1.0
import QtQuick.Controls
import Global
import Error
import LanguageManager

Window {
    id: root
    visible: true
    width: Screen.width/2
    height: Screen.height/2
    title: "Wuthering Waves Convene Export"
    Button{
        width:50
        height:50
        onClicked:ConfigManager.setWarning()
    }
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
        path: Global.path
    }
    Component.onCompleted:{
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
    }/*
Connections {
    target: ErrorNotifier
    onErrorOccurred: {
        errorBanner.errorMessage = message
        errorBanner.open()
    }
}
Popup {
    id: errorBanner
    width: parent ? parent.width : 400
    height: implicitHeight
    y: 0
    modal: false
    focus: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
    parent: Overlay.overlay
    z: 999

    property string errorMessage: ""

    background: Rectangle {
        color: "#f44336" // 红色背景
        radius: 4
        border.color: "#b71c1c"
        border.width: 1
    }

    contentItem: Row {
        spacing: 10
        anchors.fill: parent
        anchors.margins: 12

        Image {
            source: "qrc:/icons/error.svg"
            width: 24
            height: 24
        }

        Text {
            text: errorBanner.errorMessage
            color: "white"
            font.bold: true
            wrapMode: Text.Wrap
            elide: Text.ElideRight
        }

   Item {
    width: parent.width - (icon.width + text.width + button.width + spacing * 3)
    height: 1 // 不影响布局
}


        Button {
            text: "关闭"
            onClicked: errorBanner.close()
            background: Rectangle { color: "#d32f2f"; radius: 4 }
            contentItem: Text { color: "white"; text: "关闭" }
        }
    }

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
        NumberAnimation { property: "y"; from: -errorBanner.height; to: 0; duration: 200 }
    }

    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
        NumberAnimation { property: "y"; from: 0; to: -errorBanner.height; duration: 200 }
    }
}*/
    Column {
        anchors.centerIn: parent
        spacing: 10

        Text {
            text: qsTr("Hello World")
        }

        Button {
            text: qsTr("Confirm")
        }

        ComboBox {
            model: [
                { "name": "中文", "lang": "zh_CN" },
                { "name": "English", "lang": "en_US" }
            ]
            textRole: "name"
            onCurrentIndexChanged: {
                LanguageManager.switchLanguage(model[currentIndex].lang)
            }
        }
    }
}
