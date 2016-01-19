//
//  LoadingScreen.qml
//
//  Created by Thijs Wenker on 2016/01/02
//  E-SPACES
//

import Hifi 1.0
import QtQuick 2.4
import QtQuick.Controls 1.2
import "controls"
import "styles"

Item {
    id: root
    HifiConstants { id: hifi ;}
    z: -1
    objectName: "LoadingScreen"
    anchors.fill: parent
    property bool destroyOnInvisible: false
    opacity: 1

    LoadingScreen {
        id: loadingScreen
        implicitWidth: backgroundRectangle.width
        implicitHeight: backgroundRectangle.height
        anchors.fill: parent

        Rectangle {
            id: backgroundRectangle
            width: root.width
            height: root.height
            anchors.fill: parent
            color: "#2c86b1"
            opacity: 0.9
            Image {
                id: utiiIcon
                anchors.rightMargin: 0
                anchors.leftMargin: 0
                anchors.topMargin: 80
                anchors.bottomMargin: 122
                rotation: 0
                fillMode: Image.PreserveAspectFit
                anchors.fill: parent
                source: "../images/infinity-island-logo.svg"
            }
            ProgressBar {
                id: loadingProgress
                height: 23
                anchors.right: parent.right
                anchors.left: parent.left
                rotation: 0
                scale: 1
                orientation: 1
                value: loadingScreen.percentage
                maximumValue: 1
                anchors.bottom: parent.bottom
                anchors.rightMargin: 0
                anchors.leftMargin: 0
                anchors.bottomMargin: 0
            }
        }
        Connections {
            target: loadingScreen.parent
        }
    }
    
}
