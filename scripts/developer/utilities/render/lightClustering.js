//
//  lightClustering.js
//  examples/utilities/tools/render
//
//  Sam Gateau, created on 9/9/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

// Set up the qml ui
var qml = Script.resolvePath('lightClustering.qml');
var window = new OverlayWindow({
    title: 'Light Clustering',
    source: qml,
    width: 400, 
    height: 300
});
window.setPosition(Window.innerWidth - 420, 50 + 250 + 50 + 250 + 50 );
window.closed.connect(function() { Script.stop(); });