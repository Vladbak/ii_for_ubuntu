import QtQuick 2.5
import QtWebEngine 1.1

WebEngineView {
    id: root
    property var newUrl;

    profile.httpUserAgent: "Mozilla/5.0 Chrome (HighFidelityInterface)"

    Component.onCompleted: {
        console.log("Connecting JS messaging to Hifi Logging")
        // Ensure the JS from the web-engine makes it to our logging
        root.javaScriptConsoleMessage.connect(function(level, message, lineNumber, sourceID) {
            console.log("Web Window JS message: " + sourceID + " " + lineNumber + " " +  message);
        });

    }

    // FIXME hack to get the URL with the auth token included.  Remove when we move to Qt 5.6
    Timer {
        id: urlReplacementTimer
        running: false
        repeat: false
        interval: 50
        onTriggered: url = newUrl;
    }

    onUrlChanged: {
        var originalUrl = url.toString();
        newUrl = urlHandler.fixupUrl(originalUrl).toString();
        if (newUrl !== originalUrl) {
            root.stop();
            if (urlReplacementTimer.running) {
                console.warn("Replacement timer already running");
                return;
            }
            urlReplacementTimer.start();
        }
    }

    onFeaturePermissionRequested: {
        grantFeaturePermission(securityOrigin, feature, true);
    }

    onLoadingChanged: {
        // Required to support clicking on "hifi://" links
        if (WebEngineView.LoadStartedStatus == loadRequest.status) {
            var url = loadRequest.url.toString();
            if (urlHandler.canHandleUrl(url)) {
                if (urlHandler.handleUrl(url)) {
                    root.stop();
                }
            }
        }
    }

    onNewViewRequested:{
            var component = Qt.createComponent("../Browser.qml");
            var newWindow = component.createObject(desktop);
            request.openIn(newWindow.webView)
    }

    // This breaks the webchannel used for passing messages.  Fixed in Qt 5.6
    // See https://bugreports.qt.io/browse/QTBUG-49521
    //profile: desktop.browserProfile
}
