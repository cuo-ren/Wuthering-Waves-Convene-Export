import QtQuick 2.15
import QtQuick.Window 2.15

Item{
    id: root
    width: 80
    height: 80
    property color usedColor: "blue"
    property int callCount: 0
    property int duration: 5500

    Component.onCompleted:{
        callCount = 0
        intervalTimer.start()
    }
    Timer {
        id: intervalTimer
        interval: root.duration/5500*200 // 毫秒
        repeat: true
        running: false
        onTriggered: {
            switch(callCount){
                case 0:obj1.visible = true;ranim1.start();oanim1.start();callCount++;break;
                case 1:obj2.visible = true;ranim2.start();oanim2.start();callCount++;break;
                case 2:obj3.visible = true;ranim3.start();oanim3.start();callCount++;break;
                case 3:obj4.visible = true;ranim4.start();oanim4.start();callCount++;break;
                case 4:obj5.visible = true;ranim5.start();oanim5.start();callCount++;break;
                case 5:obj6.visible = true;ranim6.start();oanim6.start();callCount++;break;
                default:intervalTimer.stop();break;
            }
        }
    }
    Item{
        visible: false
        id: obj1
        width: parent.width
        height: parent.height
        Rectangle {
           id: rec1
           width: (root.height>root.width?root.width:root.height) / 8
           height: (root.height>root.width?root.width:root.height) / 8
           radius: width / 2
           color: root.usedColor
           anchors.top: parent.top
           anchors.horizontalCenter: parent.horizontalCenter
        }
        SequentialAnimation on rotation {
            id: ranim1
            loops: Animation.Infinite
            // 对应 0% -> 7%
            NumberAnimation { from: 190; to: 300; duration: root.duration*0.07; easing.type: Easing.Bezier; easing.bezierCurve: [0.29,0.44,0.32,0.74] }
            // 7% -> 30%
            NumberAnimation { from: 300; to: 450; duration: root.duration*0.23; easing.type: Easing.Linear }
            // 30% -> 39%
            NumberAnimation { from: 450; to: 645; duration: root.duration*0.09; easing.type: Easing.Bezier; easing.bezierCurve: [0.53,0.27,0.37,0.81] }
            // 39% -> 63%
            NumberAnimation { from: 645; to: 800; duration: root.duration*0.24; easing.type: Easing.Linear }
            // 63% -> 68%
            NumberAnimation { from: 800; to: 920; duration: root.duration*0.05; easing.type: Easing.Bezier; easing.bezierCurve: [0.5,0.32,0.82,0.54] }
            // 68% -> 69%
            NumberAnimation { from: 920; to: 930; duration: root.duration*0.01; easing.type: Easing.InQuad }
            NumberAnimation { from: 930; to: 190; duration: root.duration*0.31; easing.type: Easing.Linear }
            running: false
        }

        SequentialAnimation on opacity {
            id: oanim1
            loops: Animation.Infinite
            NumberAnimation { from: 1; to: 1; duration: root.duration*0.68 } // 保持不变直到68%
            NumberAnimation { from: 1; to: 0; duration: root.duration*0.01; easing.type: Easing.InQuad }  // 69%时渐隐
            NumberAnimation { from: 0; to: 0; duration: root.duration*0.31 }
            running: false
        }
    }
    Item{
        visible: false
        id: obj2
        width: parent.width
        height: parent.height
        Rectangle {
           id: rec2
           width: (root.height>root.width?root.width:root.height) / 8
           height: (root.height>root.width?root.width:root.height) / 8
           radius: width / 2
           color: root.usedColor
           anchors.top: parent.top
           anchors.horizontalCenter: parent.horizontalCenter
        }
        SequentialAnimation on rotation {
            id: ranim2
            loops: Animation.Infinite
            // 对应 0% -> 7%
            NumberAnimation { from: 180; to: 300; duration: root.duration*0.07; easing.type: Easing.Bezier; easing.bezierCurve: [0.29,0.44,0.32,0.74] }
            // 7% -> 30%
            NumberAnimation { from: 300; to: 450; duration: root.duration*0.23; easing.type: Easing.Linear }
            // 30% -> 39%
            NumberAnimation { from: 450; to: 645; duration: root.duration*0.09; easing.type: Easing.Bezier; easing.bezierCurve: [0.53,0.27,0.37,0.81] }
            // 39% -> 63%
            NumberAnimation { from: 645; to: 800; duration: root.duration*0.24; easing.type: Easing.Linear }
            // 63% -> 68%
            NumberAnimation { from: 800; to: 910; duration: root.duration*0.05; easing.type: Easing.Bezier; easing.bezierCurve: [0.5,0.32,0.82,0.54] }
            // 68% -> 69%
            NumberAnimation { from: 910; to: 920; duration: root.duration*0.01; easing.type: Easing.InQuad }
            NumberAnimation { from: 920; to: 180; duration: root.duration*0.31; easing.type: Easing.Linear }
            running: false
        }

        SequentialAnimation on opacity {
            id: oanim2
            loops: Animation.Infinite
            NumberAnimation { from: 1; to: 1; duration: root.duration*0.68 } // 保持不变直到68%
            NumberAnimation { from: 1; to: 0; duration: root.duration*0.01; easing.type: Easing.InQuad }  // 69%时渐隐
            NumberAnimation { from: 0; to: 0; duration: root.duration*0.31 }
            running: false
        }
    }
    Item{
        visible: false
        id: obj3
        width: parent.width
        height: parent.height
        Rectangle {
           id: rec3
           width: (root.height>root.width?root.width:root.height) / 8
           height: (root.height>root.width?root.width:root.height) / 8
           radius: width / 2
           color: root.usedColor
           anchors.top: parent.top
           anchors.horizontalCenter: parent.horizontalCenter
        }
        SequentialAnimation on rotation {
            id: ranim3
            loops: Animation.Infinite
            // 对应 0% -> 7%
            NumberAnimation { from: 170; to: 300; duration: root.duration*0.07; easing.type: Easing.Bezier; easing.bezierCurve: [0.29,0.44,0.32,0.74] }
            // 7% -> 30%
            NumberAnimation { from: 300; to: 450; duration: root.duration*0.23; easing.type: Easing.Linear }
            // 30% -> 39%
            NumberAnimation { from: 450; to: 645; duration: root.duration*0.09; easing.type: Easing.Bezier; easing.bezierCurve: [0.53,0.27,0.37,0.81] }
            // 39% -> 63%
            NumberAnimation { from: 645; to: 800; duration: root.duration*0.24; easing.type: Easing.Linear }
            // 63% -> 68%
            NumberAnimation { from: 800; to: 900; duration: root.duration*0.05; easing.type: Easing.Bezier; easing.bezierCurve: [0.5,0.32,0.82,0.54] }
            // 68% -> 69%
            NumberAnimation { from: 900; to: 910; duration: root.duration*0.01; easing.type: Easing.InQuad }
            NumberAnimation { from: 910; to: 170; duration: root.duration*0.31; easing.type: Easing.Linear }
            running: false
        }

        SequentialAnimation on opacity {
            id: oanim3
            loops: Animation.Infinite
            NumberAnimation { from: 1; to: 1; duration: root.duration*0.68 } // 保持不变直到68%
            NumberAnimation { from: 1; to: 0; duration: root.duration*0.01; easing.type: Easing.InQuad }  // 69%时渐隐
            NumberAnimation { from: 0; to: 0; duration: root.duration*0.31 }
            running: false
        }
    }
    Item{
        visible: false
        id: obj4
        width: parent.width
        height: parent.height
        Rectangle {
           id: rec4
           width: (root.height>root.width?root.width:root.height) / 8
           height: (root.height>root.width?root.width:root.height) / 8
           radius: width / 2
           color: root.usedColor
           anchors.top: parent.top
           anchors.horizontalCenter: parent.horizontalCenter
        }
        SequentialAnimation on rotation {
            id: ranim4
            loops: Animation.Infinite
            // 对应 0% -> 7%
            NumberAnimation { from: 160; to: 300; duration: root.duration*0.07; easing.type: Easing.Bezier; easing.bezierCurve: [0.29,0.44,0.32,0.74] }
            // 7% -> 30%
            NumberAnimation { from: 300; to: 450; duration: root.duration*0.23; easing.type: Easing.Linear }
            // 30% -> 39%
            NumberAnimation { from: 450; to: 645; duration: root.duration*0.09; easing.type: Easing.Bezier; easing.bezierCurve: [0.53,0.27,0.37,0.81] }
            // 39% -> 63%
            NumberAnimation { from: 645; to: 800; duration: root.duration*0.24; easing.type: Easing.Linear }
            // 63% -> 68%
            NumberAnimation { from: 800; to: 890; duration: root.duration*0.05; easing.type: Easing.Bezier; easing.bezierCurve: [0.5,0.32,0.82,0.54] }
            // 68% -> 69%
            NumberAnimation { from: 890; to: 900; duration: root.duration*0.01; easing.type: Easing.InQuad }
            NumberAnimation { from: 900; to: 160; duration: root.duration*0.31; easing.type: Easing.Linear }
            running: false
        }

        SequentialAnimation on opacity {
            id: oanim4
            loops: Animation.Infinite
            NumberAnimation { from: 1; to: 1; duration: root.duration*0.68 } // 保持不变直到68%
            NumberAnimation { from: 1; to: 0; duration: root.duration*0.01; easing.type: Easing.InQuad }  // 69%时渐隐
            NumberAnimation { from: 0; to: 0; duration: root.duration*0.31 }
            running: false
        }
    }
    Item{
        visible: false
        id: obj5
        width: parent.width
        height: parent.height
        Rectangle {
           id: rec5
           width: (root.height>root.width?root.width:root.height) / 8
           height: (root.height>root.width?root.width:root.height) / 8
           radius: width / 2
           color: root.usedColor
           anchors.top: parent.top
           anchors.horizontalCenter: parent.horizontalCenter
        }
        SequentialAnimation on rotation {
            id: ranim5
            loops: Animation.Infinite
            // 对应 0% -> 7%
            NumberAnimation { from: 150; to: 300; duration: root.duration*0.07; easing.type: Easing.Bezier; easing.bezierCurve: [0.29,0.44,0.32,0.74] }
            // 7% -> 30%
            NumberAnimation { from: 300; to: 450; duration: root.duration*0.23; easing.type: Easing.Linear }
            // 30% -> 39%
            NumberAnimation { from: 450; to: 645; duration: root.duration*0.09; easing.type: Easing.Bezier; easing.bezierCurve: [0.53,0.27,0.37,0.81] }
            // 39% -> 63%
            NumberAnimation { from: 645; to: 800; duration: root.duration*0.24; easing.type: Easing.Linear }
            // 63% -> 68%
            NumberAnimation { from: 800; to: 880; duration: root.duration*0.05; easing.type: Easing.Bezier; easing.bezierCurve: [0.5,0.32,0.82,0.54] }
            // 68% -> 69%
            NumberAnimation { from: 880; to: 880; duration: root.duration*0.01; easing.type: Easing.InQuad }
            NumberAnimation { from: 880; to: 150; duration: root.duration*0.31; easing.type: Easing.Linear }
            running: false
        }

        SequentialAnimation on opacity {
            id: oanim5
            loops: Animation.Infinite
            NumberAnimation { from: 1; to: 1; duration: root.duration*0.68 } // 保持不变直到68%
            NumberAnimation { from: 1; to: 0; duration: root.duration*0.01; easing.type: Easing.InQuad }  // 69%时渐隐
            NumberAnimation { from: 0; to: 0; duration: root.duration*0.31 }
            running: false
        }
    }
    Item{
        visible: false
        id: obj6
        width: parent.width
        height: parent.height
        Rectangle {
           id: rec6
           width: (root.height>root.width?root.width:root.height) / 8
           height: (root.height>root.width?root.width:root.height) / 8
           radius: width / 2
           color: root.usedColor
           anchors.top: parent.top
           anchors.horizontalCenter: parent.horizontalCenter
        }
        SequentialAnimation on rotation {
            id: ranim6
            loops: Animation.Infinite
            // 对应 0% -> 7%
            NumberAnimation { from: 140; to: 300; duration: root.duration*0.07; easing.type: Easing.Bezier; easing.bezierCurve: [0.29,0.44,0.32,0.74] }
            // 7% -> 30%
            NumberAnimation { from: 300; to: 450; duration: root.duration*0.23; easing.type: Easing.Linear }
            // 30% -> 39%
            NumberAnimation { from: 450; to: 645; duration: root.duration*0.09; easing.type: Easing.Bezier; easing.bezierCurve: [0.53,0.27,0.37,0.81] }
            // 39% -> 63%
            NumberAnimation { from: 645; to: 800; duration: root.duration*0.24; easing.type: Easing.Linear }
            // 63% -> 68%
            NumberAnimation { from: 800; to: 870; duration: root.duration*0.05; easing.type: Easing.Bezier; easing.bezierCurve: [0.5,0.32,0.82,0.54] }
            // 68% -> 69%
            NumberAnimation { from: 870; to: 880; duration: root.duration*0.01; easing.type: Easing.InQuad }
            NumberAnimation { from: 880; to: 140; duration: root.duration*0.31; easing.type: Easing.Linear }
            running: false
        }

        SequentialAnimation on opacity {
            id: oanim6
            loops: Animation.Infinite
            NumberAnimation { from: 1; to: 1; duration: root.duration*0.68 } // 保持不变直到68%
            NumberAnimation { from: 1; to: 0; duration: root.duration*0.01; easing.type: Easing.InQuad }  // 69%时渐隐
            NumberAnimation { from: 0; to: 0; duration: root.duration*0.31 }
            running: false
        }
    }
}
