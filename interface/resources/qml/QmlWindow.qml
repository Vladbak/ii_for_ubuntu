
import QtQuick 2.3
import QtQuick.Controls 1.2
import QtWebChannel 1.0
import QtWebSockets 1.0
import "qrc:///qtwebchannel/qwebchannel.js" as WebChannel

import "windows" as Windows
import "controls"
import "styles"

Windows.Window {
    id: root
    HifiConstants { id: hifi }
    title: "QmlWindow"
    resizable: true
    visible: false
    focus: true
    property var channel;
    // Don't destroy on close... otherwise the JS/C++ will have a dangling pointer
    destroyOnCloseButton: false
    property var source;
    property var eventBridge;
    property var component;
    property var dynamicContent;
    onSourceChanged: {
        if (dynamicContent) {
            dynamicContent.destroy();
            dynamicContent = null; 
        }
        component = Qt.createComponent(source);
        console.log("Created component " + component + " from source " + source);
    }

    onComponentChanged: {
        console.log("Component changed to " + component)
        populate();
    }
        
    function populate() {
        console.log("Populate called: dynamicContent " + dynamicContent + " component " + component);
        if (!dynamicContent && component) {
            if (component.status == Component.Error) {
                console.log("Error loading component:", component.errorString());
            } else if (component.status == Component.Ready) {
                console.log("Building dynamic content");
                dynamicContent = component.createObject(contentHolder);
            } else {
                console.log("Component not yet ready, connecting to status change");
                component.statusChanged.connect(populate);
            }
        }
    }

    // Handle message traffic from the script that launched us to the loaded QML
    function fromScript(message) {
        if (root.dynamicContent && root.dynamicContent.fromScript) {
            root.dynamicContent.fromScript(message);
        }
    }
    
    // Handle message traffic from our loaded QML to the script that launched us
    signal sendToScript(var message);
    onDynamicContentChanged: {
        if (dynamicContent && dynamicContent.sendToScript) {
            dynamicContent.sendToScript.connect(sendToScript);
        }
    }


    Item {
        id: contentHolder
        anchors.fill: parent
    }
}
