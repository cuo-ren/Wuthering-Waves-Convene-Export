import QtQuick
//import Qt.labs.animation
import QtQuick.Controls

Item {
    id: chart
    width: 800
    height: 400
    visible: true

    property alias gacha_data: myModel
    required property url path
    property string chartTitle: qsTr("卡池名称")
    property string key: "0"
    property alias contentWidth: root.contentWidth
    property alias chartClip: root.clip

    property bool hiddenImage: false

    function moveToEnd(){
        root.contentX = myModel.count * 50 + myModel.count * 5 - 5 - root.width > 0 ? myModel.count * 50 + myModel.count * 5 - 5 - root.width : 0
    }
    ListModel{
        id:myModel
    }

    Text {
        id: title
        text: chartTitle
        anchors.top: chart.top
        anchors.horizontalCenter: chart.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 30
    }

    Text {
        id: noData
        text: qsTr("暂无数据")
        anchors.centerIn:chart
        font.pixelSize: 20
        visible: myModel.count == 0 ? true : false
    }

    Flickable{
        width: chart.width
        height: chart.height - 30
        contentWidth: chartRow.width
        id: root
        flickableDirection:Flickable.HorizontalFlick
        anchors.horizontalCenter: chart.horizontalCenter
        anchors.bottom: chart.bottom

        clip: true
        onContentWidthChanged: {
            if (contentWidth > width) {
                contentX = contentWidth - width
            }
        }
        WheelHandler {
            id: wheelHandler
            target: root
            acceptedModifiers: Qt.NoModifier
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: (event) => {
                root.flick(event.angleDelta.y * 7, 0)
            }
        }

        WheelHandler {
            target: root
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            acceptedModifiers: Qt.ControlModifier
            onWheel: (event) => {
                    // Ctrl + 滚轮通常表示缩放
                    chartRow.spacing += event.angleDelta.y > 0 ? 1 : chartRow.spacing > 1 ? -1 : 0
            }
        }

        Row{
            id: chartRow
            spacing: 5

            Repeater{
                model: myModel

                Item{
                    id: bar
                    height: root.height
                    width: 50

                    property int animatedCount: count

                    Binding {
                        target: bar
                        property: "animatedCount"
                        value: model.count
                    }

                    Behavior on animatedCount{
                        NumberAnimation {
                            duration: 1000
                        }
                    }

                    NumberAnimation {
                        id: textAnimation
                        target: bar
                        property: "animatedCount"
                        from: 0
                        to: count
                        duration: 1000
                        easing.type: Easing.InOutCubic
                    }

                    NumberAnimation {
                        id: rectAnimation
                        target: countRect
                        property: "height"
                        from: 0
                        to: count * (root.height - 130)/80
                        duration: 1000
                        easing.type: Easing.InOutCubic
                    }

                    Component.onCompleted: {
                        textAnimation.start()
                        rectAnimation.start()
                    }

                    MouseArea{
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: countRect.width = 12
                        onExited: countRect.width = 8
                    }

                    Item{
                        anchors.bottom: bottomPart.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        Image {
                            id: name
                            source: path + "/resource/" + "wai.png"
                            anchors.bottom: itemCount.top
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.margins: 10
                            width: 30
                            height: 30
                            visible: model.isOffTarget
                        }
                        Text {
                            anchors.bottom: countRect.top
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.margins: 10
                            id: itemCount
                            text: qsTr(animatedCount+"抽")
                        }
                        Rectangle{
                            id: countRect
                            anchors.bottom: parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.margins: 10
                            width: 8
                            height: count * (root.height - 130)/80
                            color: animatedCount < 40 ? "lightgreen" : animatedCount <= 60 ? "yellow" : "red"
                            radius: width/2
                            Behavior on height{
                                NumberAnimation {
                                    duration: 1000
                                    easing.type: Easing.InOutCubic
                                }
                            }
                            Behavior on width{
                                NumberAnimation {
                                    duration: 100
                                }
                            }
                            Behavior on color{
                                ColorAnimation {
                                    duration: 300
                                }
                            }
                        } 
                    }

                    Item{
                        id: bottomPart

                        height: hiddenImage ? 30 : 80
                        width: 50

                        anchors.bottom: bar.bottom
                        anchors.horizontalCenter: bar.horizontalCenter

                        Image{
                            id: imagebackground

                            height: parent.width - 5
                            width: parent.width - 5

                            anchors.top: parent.top
                            anchors.horizontalCenter: parent.horizontalCenter

                            visible: !hiddenImage
                            source: path + "/resource/background5.png"

                            Image {
                                id: previewImage
                                source: model.source

                                anchors.fill: parent

                                fillMode: Image.PreserveAspectFit

                                height: bottomPart.width - 5
                                width: bottomPart.width - 5

                                onStatusChanged : {
                                    if(this.status == Image.Error){
                                        this.source = path + "/resource/" + "unknown.png"
                                    }
                                }
                            }
                        }
                        Text {
                            id: itemName

                            anchors.top: hiddenImage ? parent.top : imagebackground.bottom
                            anchors.margins: 5
                            //anchors.bottom: hiddenImage ? undefined : parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            horizontalAlignment: Text.AlignHCenter

                            wrapMode: Text.WordWrap
                            width: parent.width + chartRow.spacing
                            text: ItemName
                        }
                    }
                }
            }
        }
    }
}
