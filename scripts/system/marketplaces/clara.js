"use strict";

//
//  clara.js
//
//  Created by Eric Levin on 8 Jan 2016
//  Edited by Elisa Lupin-Jimenez on 23 Aug 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(function() { // BEGIN LOCAL_SCOPE

var toolIconUrl = Script.resolvePath("../assets/images/tools/");
var qml = Script.resolvePath("../../../resources/qml/Marketplaces.qml")

var MARKETPLACE_URL = "https://metaverse.highfidelity.com/marketplace";

var marketplaceWindow = new OverlayWindow({
    title: "Marketplace",
    source: qml,
    width: 1000,
    height: 900,
    toolWindow: false,
    visible: false,
});

var toolHeight = 50;
var toolWidth = 50;
var TOOLBAR_MARGIN_Y = 0;


function showMarketplace(marketplaceID) {
    var url = MARKETPLACE_URL;
    if (marketplaceID) {
        url = url + "/items/" + marketplaceID;
    }
    marketplaceWindow.setVisible(true);

    UserActivityLogger.openedMarketplace();
}

function hideMarketplace() {
    marketplaceWindow.setVisible(false);
}

function toggleMarketplace() {
    if (marketplaceWindow.visible) {
        hideMarketplace();
    } else {
        showMarketplace();
    }
}

var toolBar = Toolbars.getToolbar("com.highfidelity.interface.toolbar.system");

var browseExamplesButton = toolBar.addButton({
    imageURL: toolIconUrl + "market.svg",
    objectName: "marketplace",
    buttonState: 1,
    defaultState: 1,
    hoverState: 3,
    alpha: 0.9
});

function onExamplesWindowVisibilityChanged() {
    browseExamplesButton.writeProperty('buttonState', marketplaceWindow.visible ? 0 : 1);
    browseExamplesButton.writeProperty('defaultState', marketplaceWindow.visible ? 0 : 1);
    browseExamplesButton.writeProperty('hoverState', marketplaceWindow.visible ? 2 : 3);
}
function onClick() {
    toggleMarketplace();
}
browseExamplesButton.clicked.connect(onClick);
marketplaceWindow.visibleChanged.connect(onExamplesWindowVisibilityChanged);

Script.scriptEnding.connect(function () {
    toolBar.removeButton("marketplace");
    browseExamplesButton.clicked.disconnect(onClick);
    marketplaceWindow.visibleChanged.disconnect(onExamplesWindowVisibilityChanged);
});

}()); // END LOCAL_SCOPE
