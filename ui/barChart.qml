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
    property bool flag: false

    function moveToEnd(){
        root.contentX = myModel.count * 50 + myModel.count * 5 - 5 - root.width > 0 ? myModel.count * 50 + myModel.count * 5 - 5 - root.width : 0
    }
    ListModel{
        id:myModel
    }/*
    function valueToColor(v) {
        v = Math.max(0, Math.min(80, v));

        let r, g, b;

        if (v <= 40) {
            // lightgreen → yellow
            let t = v / 40;
            r = lerp(144, 255, t);
            g = lerp(238, 255, t);
            b = lerp(144,   0, t);
        } else if (v <= 65) {
            // yellow → orange (#FFA500)
            let t = (v - 40) / (65 - 40);
            r = lerp(255, 255, t);
            g = lerp(255, 165, t);
            b = lerp(  0,   0, t);
        } else {
            // orange → red
            let t = (v - 65) / (80 - 65);
            r = lerp(255, 255, t);
            g = lerp(165,   0, t);
            b = lerp(  0,   0, t);
        }

        return Qt.rgba(r/255, g/255, b/255, 1);
    }*/
    // 输入 v: 0~80 任意值；40~80 做黄→红非线性
    function valueToColor(v) {
        // 关键：仅对 40~80 段做非线性映射
        let r, g, b;

        if (v <= 40) {
            // lightgreen → yellow
            let t = v / 40;
            r = lerp(144, 255, t);
            g = lerp(238, 255, t);
            b = lerp(144,   0, t);
            return Qt.rgba(r/255, g/255, b/255, 1);
        } // lightgreen，可按需替换
        if (v >= 80) return Qt.hsla(0/360,    1.0, 0.5, 1); // red

        // 线性比例
        const t = (v - 40) / 40;

        // 指数形状函数：p≈1.48 让 v=65 时接近橙（H=30°）
        const p = 1.48; // 可当作参数暴露出去
        const u = Math.pow(t, p); // 变形后的比例

        // HSL 插值：H 60°→0°，S/L 保持一致（可按需调）
        const h0 = 60/360, h1 = 0/360;
        const s = 1.0,     l = 0.5;

        const h = h0 + (h1 - h0) * u;
        return Qt.hsla(h, s, l, 1);
    }


    function lerp(a, b, t) {
        return a + (b - a) * t;
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
            if(flag){
                if (contentWidth > width) {
                    contentX = contentWidth - width
                }
            }else{
                flag = true
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
                    flag = false
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
                        to: count * (root.height - 30 - 10 - 12 - 10 - 10 - 50 - 5 - 12 - 30)/80
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
                        onEntered: countRect.width = 16
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
                            text: qsTr(animatedCount + qsTr("抽"))
                        }
                        Rectangle{
                            id: countRect
                            anchors.bottom: parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.margins: 10
                            width: 8
                            height: count * (root.height - 30 - 10 - 12 - 10 - 10 - 50 - 5 -12 - 30)/80
                            color: chart.valueToColor(animatedCount)//animatedCount < 40 ? "lightgreen" : animatedCount <= 65 ? "yellow" : "red"
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
                            //Behavior on color{
                            //    ColorAnimation {
                            //        duration: 300
                            //    }
                            //}
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
