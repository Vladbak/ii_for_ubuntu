import QtQuick 2.5
import QtQuick.Controls 1.4
import QtWebEngine 1.1

import "../../windows" as Windows
import "../../controls" as Controls
import "../../styles"

Windows.Window {
    id: root
    HifiConstants { id: hifi }
    width: 900; height: 700
    resizable: true
    modality: Qt.ApplicationModal

    Item {
        anchors.fill: parent

        Controls.WebView {
            id: webview
            anchors { top: parent.top; left: parent.left; right: parent.right; bottom: closeButton.top; margins: 8 }
            url: "https://metaverse.highfidelity.com/marketplace?category=avatars"
            focus: true
        }

        Button {
            id: closeButton
            anchors { bottom: parent.bottom; right: parent.right; margins: 8 }
            text: "Close"
            onClicked: root.destroy();
        }
    }
}
