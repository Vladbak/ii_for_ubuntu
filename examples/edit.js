//  newEditEntities.js
//  examples
//
//  Created by Brad Hefta-Gaub on 10/2/14.
//  Persist toolbar by HRS 6/11/15.
//  Copyright 2014 High Fidelity, Inc.
//
//  This script allows you to edit entities with a new UI/UX for mouse and trackpad based editing
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

HIFI_PUBLIC_BUCKET = "http://s3.amazonaws.com/hifi-public/";

Script.include([
    "libraries/stringHelpers.js",
    "libraries/dataViewHelpers.js",
    "libraries/toolBars.js",
    "libraries/progressDialog.js",

    "libraries/entitySelectionTool.js",

    "libraries/ToolTip.js",

    "libraries/entityCameraTool.js",
    "libraries/gridTool.js",
    "libraries/entityList.js",
    "libraries/lightOverlayManager.js",
]);

var selectionDisplay = SelectionDisplay;
var selectionManager = SelectionManager;

var lightOverlayManager = new LightOverlayManager();

var cameraManager = new CameraManager();

var grid = Grid();
// gridTool = GridTool({
//     horizontalGrid: grid
// });
// gridTool.setVisible(false);

var entityListTool = EntityListTool();

selectionManager.addEventListener(function() {
    selectionDisplay.updateHandles();
    lightOverlayManager.updatePositions();
});

var toolIconUrl = HIFI_PUBLIC_BUCKET + "images/tools/";
var toolHeight = 50;
var toolWidth = 50;

var DEGREES_TO_RADIANS = Math.PI / 180.0;
var RADIANS_TO_DEGREES = 180.0 / Math.PI;
var epsilon = 0.001;

var MIN_ANGULAR_SIZE = 2;
var MAX_ANGULAR_SIZE = 45;
var allowLargeModels = true;
var allowSmallModels = true;
var wantEntityGlow = false;

var SPAWN_DISTANCE = 1;
var DEFAULT_DIMENSION = 0.20;
var DEFAULT_TEXT_DIMENSION_X = 1.0;
var DEFAULT_TEXT_DIMENSION_Y = 1.0;
var DEFAULT_TEXT_DIMENSION_Z = 0.01;

var DEFAULT_DIMENSIONS = {
    x: DEFAULT_DIMENSION,
    y: DEFAULT_DIMENSION,
    z: DEFAULT_DIMENSION
};

var DEFAULT_LIGHT_DIMENSIONS = Vec3.multiply(20, DEFAULT_DIMENSIONS);

var MENU_AUTO_FOCUS_ON_SELECT = "Auto Focus on Select";
var MENU_EASE_ON_FOCUS = "Ease Orientation on Focus";
var MENU_SHOW_LIGHTS_IN_EDIT_MODE = "Show Lights in Edit Mode";
var MENU_SHOW_ZONES_IN_EDIT_MODE = "Show Zones in Edit Mode";

var SETTING_INSPECT_TOOL_ENABLED = "inspectToolEnabled";
var SETTING_AUTO_FOCUS_ON_SELECT = "autoFocusOnSelect";
var SETTING_EASE_ON_FOCUS = "cameraEaseOnFocus";
var SETTING_SHOW_LIGHTS_IN_EDIT_MODE = "showLightsInEditMode";
var SETTING_SHOW_ZONES_IN_EDIT_MODE = "showZonesInEditMode";

var INSUFFICIENT_PERMISSIONS_ERROR_MSG = "You do not have the necessary permissions to edit on this domain."
var INSUFFICIENT_PERMISSIONS_IMPORT_ERROR_MSG = "You do not have the necessary permissions to place items on this domain."

var modelURLs = [
    "Insert the URL to your FBX"
];

var mode = 0;
var isActive = false;

var placingEntityID = null;

IMPORTING_SVO_OVERLAY_WIDTH = 144;
IMPORTING_SVO_OVERLAY_HEIGHT = 30;
IMPORTING_SVO_OVERLAY_MARGIN = 5;
IMPORTING_SVO_OVERLAY_LEFT_MARGIN = 34;
var importingSVOImageOverlay = Overlays.addOverlay("image", {
    imageURL: HIFI_PUBLIC_BUCKET + "images/hourglass.svg",
    width: 20,
    height: 20,
    alpha: 1.0,
    color: {
        red: 255,
        green: 255,
        blue: 255
    },
    x: Window.innerWidth - IMPORTING_SVO_OVERLAY_WIDTH,
    y: Window.innerHeight - IMPORTING_SVO_OVERLAY_HEIGHT,
    visible: false,
});
var importingSVOTextOverlay = Overlays.addOverlay("text", {
    font: {
        size: 14
    },
    text: "Importing SVO...",
    leftMargin: IMPORTING_SVO_OVERLAY_LEFT_MARGIN,
    x: Window.innerWidth - IMPORTING_SVO_OVERLAY_WIDTH - IMPORTING_SVO_OVERLAY_MARGIN,
    y: Window.innerHeight - IMPORTING_SVO_OVERLAY_HEIGHT - IMPORTING_SVO_OVERLAY_MARGIN,
    width: IMPORTING_SVO_OVERLAY_WIDTH,
    height: IMPORTING_SVO_OVERLAY_HEIGHT,
    backgroundColor: {
        red: 80,
        green: 80,
        blue: 80
    },
    backgroundAlpha: 0.7,
    visible: false,
});

var MARKETPLACE_URL = "https://metaverse.highfidelity.com/marketplace";
var marketplaceWindow = new OverlayWebWindow({
    title: 'Marketplace', 
    source: "about:blank", 
    width: 900, 
    height: 700,
    visible: false
});

function showMarketplace(marketplaceID) {
    var url = MARKETPLACE_URL;
    if (marketplaceID) {
        url = url + "/items/" + marketplaceID;
    }
    print("setting marketplace URL to " + url);
    marketplaceWindow.setURL(url);
    marketplaceWindow.setVisible(true);
    marketplaceWindow.raise();
}

function hideMarketplace() {
    marketplaceWindow.setVisible(false);
    marketplaceWindow.setURL("about:blank");
}

function toggleMarketplace() {
    if (marketplaceWindow.visible) {
        hideMarketplace();
    } else {
        showMarketplace();
    }
}

var toolBar = (function() {
    var that = {},
        toolBar,
        activeButton,
        newModelButton,
        newCubeButton,
        newSphereButton,
        newLightButton,
        newTextButton,
        newWebButton,
        newZoneButton,
        newPolyVoxButton;

    function initialize() {
        toolBar = new ToolBar(0, 0, ToolBar.VERTICAL, "highfidelity.edit.toolbar", function(windowDimensions, toolbar) {
            return {
                x: windowDimensions.x - 8 - toolbar.width,
                y: (windowDimensions.y - toolbar.height) / 2
            };
        });

  

        activeButton = toolBar.addTool({
            imageURL: toolIconUrl + "edit-status.svg",
            subImage: {
                x: 0,
                y: Tool.IMAGE_WIDTH,
                width: Tool.IMAGE_WIDTH,
                height: Tool.IMAGE_HEIGHT
            },
            width: toolWidth,
            height: toolHeight,
            alpha: 0.9,
            visible: true
        }, true, false);

        newModelButton = toolBar.addTool({
            imageURL: toolIconUrl + "upload.svg",
            subImage: {
                x: 0,
                y: Tool.IMAGE_WIDTH,
                width: Tool.IMAGE_WIDTH,
                height: Tool.IMAGE_HEIGHT
            },
            width: toolWidth,
            height: toolHeight,
            alpha: 0.9,
            visible: false
        });

        newCubeButton = toolBar.addTool({
            imageURL: toolIconUrl + "add-cube.svg",
            subImage: {
                x: 0,
                y: Tool.IMAGE_WIDTH,
                width: Tool.IMAGE_WIDTH,
                height: Tool.IMAGE_HEIGHT
            },
            width: toolWidth,
            height: toolHeight,
            alpha: 0.9,
            visible: false
        });

        newSphereButton = toolBar.addTool({
            imageURL: toolIconUrl + "add-sphere.svg",
            subImage: {
                x: 0,
                y: Tool.IMAGE_WIDTH,
                width: Tool.IMAGE_WIDTH,
                height: Tool.IMAGE_HEIGHT
            },
            width: toolWidth,
            height: toolHeight,
            alpha: 0.9,
            visible: false
        });

        newLightButton = toolBar.addTool({
            imageURL: toolIconUrl + "light.svg",
            subImage: {
                x: 0,
                y: Tool.IMAGE_WIDTH,
                width: Tool.IMAGE_WIDTH,
                height: Tool.IMAGE_HEIGHT
            },
            width: toolWidth,
            height: toolHeight,
            alpha: 0.9,
            visible: false
        });

        newTextButton = toolBar.addTool({
            imageURL: toolIconUrl + "add-text.svg",
            subImage: {
                x: 0,
                y: Tool.IMAGE_WIDTH,
                width: Tool.IMAGE_WIDTH,
                height: Tool.IMAGE_HEIGHT
            },
            width: toolWidth,
            height: toolHeight,
            alpha: 0.9,
            visible: false
        });

        newWebButton = toolBar.addTool({
            imageURL: "https://hifi-public.s3.amazonaws.com/images/www.svg",
            subImage: {
                x: 0,
                y: 0,
                width: 128,
                height: 128
            },
            width: toolWidth,
            height: toolHeight,
            alpha: 0.9,
            visible: false
        });

        newZoneButton = toolBar.addTool({
            imageURL: toolIconUrl + "zonecube_text.svg",
            subImage: {
                x: 0,
                y: 128,
                width: 128,
                height: 128
            },
            width: toolWidth,
            height: toolHeight,
            alpha: 0.9,
            visible: false
        });

        newPolyVoxButton = toolBar.addTool({
            imageURL: toolIconUrl + "polyvox.svg",
            subImage: {
                x: 0,
                y: 0,
                width: 256,
                height: 256
            },
            width: toolWidth,
            height: toolHeight,
            alpha: 0.9,
            visible: false
        });

        that.setActive(false);
    }

    that.clearEntityList = function() {
        entityListTool.clearEntityList();
    };

    that.setActive = function(active) {
        if (active != isActive) {
            if (active && !Entities.canAdjustLocks()) {
                Window.alert(INSUFFICIENT_PERMISSIONS_ERROR_MSG);
            } else {
                isActive = active;
                if (!isActive) {
                    entityListTool.setVisible(false);
                    // gridTool.setVisible(false);
                    grid.setEnabled(false);
                    propertiesTool.setVisible(false);
                    selectionManager.clearSelections();
                    cameraManager.disable();
                } else {
                    hasShownPropertiesTool = false;
                    entityListTool.setVisible(true);
                    // gridTool.setVisible(true);
                    grid.setEnabled(true);
                    propertiesTool.setVisible(true);
                    Window.setFocus();
                }
                that.showTools(isActive);
            }
        }
        toolBar.selectTool(activeButton, isActive);
        lightOverlayManager.setVisible(isActive && Menu.isOptionChecked(MENU_SHOW_LIGHTS_IN_EDIT_MODE));
        Entities.setDrawZoneBoundaries(isActive && Menu.isOptionChecked(MENU_SHOW_ZONES_IN_EDIT_MODE));
    };

    // Sets visibility of tool buttons, excluding the power button
    that.showTools = function(doShow) {
        toolBar.showTool(newModelButton, doShow);
        toolBar.showTool(newCubeButton, doShow);
        toolBar.showTool(newSphereButton, doShow);
        toolBar.showTool(newLightButton, doShow);
        toolBar.showTool(newTextButton, doShow);
        toolBar.showTool(newWebButton, doShow);
        toolBar.showTool(newZoneButton, doShow);
        toolBar.showTool(newPolyVoxButton, doShow);
    };

    var RESIZE_INTERVAL = 50;
    var RESIZE_TIMEOUT = 120000; // 2 minutes
    var RESIZE_MAX_CHECKS = RESIZE_TIMEOUT / RESIZE_INTERVAL;

    function addModel(url) {
        var entityID = createNewEntity({
            type: "Model",
            modelURL: url
        }, false);

        if (entityID) {
            print("Model added: " + url);
            selectionManager.setSelections([entityID]);
        }
    }

    function createNewEntity(properties, dragOnCreate) {
        // Default to true if not passed in
        dragOnCreate = dragOnCreate == undefined ? true : dragOnCreate;

        var dimensions = properties.dimensions ? properties.dimensions : DEFAULT_DIMENSIONS;
        var position = getPositionToCreateEntity();
        var entityID = null;
        if (position != null) {
            position = grid.snapToSurface(grid.snapToGrid(position, false, dimensions), dimensions),
                properties.position = position;

            entityID = Entities.addEntity(properties);
            if (dragOnCreate) {
                placingEntityID = entityID;
            }
        } else {
            Window.alert("Can't create " + properties.type + ": " + properties.type + " would be out of bounds.");
        }

        return entityID;
    }

    var newModelButtonDown = false;
    that.mousePressEvent = function(event) {
        var clickedOverlay,
            url,
            file;

        if (!event.isLeftButton) {
            // if another mouse button than left is pressed ignore it
            return false;
        }

        clickedOverlay = Overlays.getOverlayAtPoint({
            x: event.x,
            y: event.y
        });

        if (activeButton === toolBar.clicked(clickedOverlay)) {
            that.setActive(!isActive);
            return true;
        }

        // Handle these two buttons in the mouseRelease event handler so that we don't suppress a mouseRelease event from
        // occurring when showing a modal dialog.
        if (newModelButton === toolBar.clicked(clickedOverlay)) {
            newModelButtonDown = true;
            return true;
        }
        
     
        if (newCubeButton === toolBar.clicked(clickedOverlay)) {
            createNewEntity({
                type: "Box",
                dimensions: DEFAULT_DIMENSIONS,
                color: {
                    red: 255,
                    green: 0,
                    blue: 0
                }
            });

            return true;
        }

        if (newSphereButton === toolBar.clicked(clickedOverlay)) {
            createNewEntity({
                type: "Sphere",
                dimensions: DEFAULT_DIMENSIONS,
                color: {
                    red: 255,
                    green: 0,
                    blue: 0
                }
            });

            return true;
        }

        if (newLightButton === toolBar.clicked(clickedOverlay)) {
            createNewEntity({
                type: "Light",
                dimensions: DEFAULT_LIGHT_DIMENSIONS,
                isSpotlight: false,
                color: {
                    red: 150,
                    green: 150,
                    blue: 150
                },

                constantAttenuation: 1,
                linearAttenuation: 0,
                quadraticAttenuation: 0,
                exponent: 0,
                cutoff: 180, // in degrees
            });

            return true;
        }

        if (newTextButton === toolBar.clicked(clickedOverlay)) {
            createNewEntity({
                type: "Text",
                dimensions: {
                    x: 0.65,
                    y: 0.3,
                    z: 0.01
                },
                backgroundColor: {
                    red: 64,
                    green: 64,
                    blue: 64
                },
                textColor: {
                    red: 255,
                    green: 255,
                    blue: 255
                },
                text: "some text",
                lineHeight: 0.06
            });

            return true;
        }

        if (newWebButton === toolBar.clicked(clickedOverlay)) {
            createNewEntity({
                type: "Web",
                dimensions: {
                    x: 1.6,
                    y: 0.9,
                    z: 0.01
                },
                sourceUrl: "https://highfidelity.com/",
            });

            return true;
        }

        if (newZoneButton === toolBar.clicked(clickedOverlay)) {
            createNewEntity({
                type: "Zone",
                dimensions: {
                    x: 10,
                    y: 10,
                    z: 10
                },
            });

            return true;
        }

        if (newPolyVoxButton === toolBar.clicked(clickedOverlay)) {
            var polyVoxId = createNewEntity({
                type: "PolyVox",
                dimensions: {
                    x: 10,
                    y: 10,
                    z: 10
                },
                voxelVolumeSize: {
                    x: 16,
                    y: 16,
                    z: 16
                },
                voxelSurfaceStyle: 2
            });
            for (var x = 1; x <= 14; x++) {
                Entities.setVoxel(polyVoxId, {
                    x: x,
                    y: 1,
                    z: 1
                }, 255);
                Entities.setVoxel(polyVoxId, {
                    x: x,
                    y: 14,
                    z: 1
                }, 255);
                Entities.setVoxel(polyVoxId, {
                    x: x,
                    y: 1,
                    z: 14
                }, 255);
                Entities.setVoxel(polyVoxId, {
                    x: x,
                    y: 14,
                    z: 14
                }, 255);
            }
            for (var y = 2; y <= 13; y++) {
                Entities.setVoxel(polyVoxId, {
                    x: 1,
                    y: y,
                    z: 1
                }, 255);
                Entities.setVoxel(polyVoxId, {
                    x: 14,
                    y: y,
                    z: 1
                }, 255);
                Entities.setVoxel(polyVoxId, {
                    x: 1,
                    y: y,
                    z: 14
                }, 255);
                Entities.setVoxel(polyVoxId, {
                    x: 14,
                    y: y,
                    z: 14
                }, 255);
            }
            for (var z = 2; z <= 13; z++) {
                Entities.setVoxel(polyVoxId, {
                    x: 1,
                    y: 1,
                    z: z
                }, 255);
                Entities.setVoxel(polyVoxId, {
                    x: 14,
                    y: 1,
                    z: z
                }, 255);
                Entities.setVoxel(polyVoxId, {
                    x: 1,
                    y: 14,
                    z: z
                }, 255);
                Entities.setVoxel(polyVoxId, {
                    x: 14,
                    y: 14,
                    z: z
                }, 255);
            }


            return true;
        }

        return false;
    };

    that.mouseReleaseEvent = function(event) {
        var handled = false;
        if (newModelButtonDown) {
            var clickedOverlay = Overlays.getOverlayAtPoint({
                x: event.x,
                y: event.y
            });
            if (newModelButton === toolBar.clicked(clickedOverlay)) {
                url = Window.prompt("Model URL", modelURLs[Math.floor(Math.random() * modelURLs.length)]);
                if (url !== null && url !== "") {
                    addModel(url);
                }
                handled = true;
            }
        }

        newModelButtonDown = false;
     

        return handled;
    }

    Window.domainChanged.connect(function() {
        that.setActive(false);
        that.clearEntityList();
    });

    Entities.canAdjustLocksChanged.connect(function(canAdjustLocks) {
        if (isActive && !canAdjustLocks) {
            that.setActive(false);
        }
    });

    that.cleanup = function() {
        toolBar.cleanup();
    };

    initialize();
    return that;
}());


function isLocked(properties) {
    // special case to lock the ground plane model in hq.
    if (location.hostname == "hq.highfidelity.io" &&
        properties.modelURL == HIFI_PUBLIC_BUCKET + "ozan/Terrain_Reduce_forAlpha.fbx") {
        return true;
    }
    return false;
}


var selectedEntityID;
var orientation;
var intersection;


var SCALE_FACTOR = 200.0;

function rayPlaneIntersection(pickRay, point, normal) {
    var d = -Vec3.dot(point, normal);
    var t = -(Vec3.dot(pickRay.origin, normal) + d) / Vec3.dot(pickRay.direction, normal);

    return Vec3.sum(pickRay.origin, Vec3.multiply(pickRay.direction, t));
}

function findClickedEntity(event) {
    var pickZones = event.isControl;

    if (pickZones) {
        Entities.setZonesArePickable(true);
    }

    var pickRay = Camera.computePickRay(event.x, event.y);

    var entityResult = Entities.findRayIntersection(pickRay, true); // want precision picking
    var lightResult = lightOverlayManager.findRayIntersection(pickRay);
    lightResult.accurate = true;

    if (pickZones) {
        Entities.setZonesArePickable(false);
    }

    var result;

    if (!entityResult.intersects && !lightResult.intersects) {
        return null;
    } else if (entityResult.intersects && !lightResult.intersects) {
        result = entityResult;
    } else if (!entityResult.intersects && lightResult.intersects) {
        result = lightResult;
    } else {
        if (entityResult.distance < lightResult.distance) {
            result = entityResult;
        } else {
            result = lightResult;
        }
    }

    if (!result.accurate) {
        return null;
    }

    var foundEntity = result.entityID;
    return {
        pickRay: pickRay,
        entityID: foundEntity
    };
}

var mouseHasMovedSincePress = false;
var mousePressStartTime = 0;
var mousePressStartPosition = {
    x: 0,
    y: 0
};
var mouseDown = false;

function mousePressEvent(event) {
    mouseDown = true;
    mousePressStartPosition = {
        x: event.x,
        y: event.y
    };
    mousePressStartTime = Date.now();
    mouseHasMovedSincePress = false;
    mouseCapturedByTool = false;

    if (propertyMenu.mousePressEvent(event) || toolBar.mousePressEvent(event) || progressDialog.mousePressEvent(event)) {
        mouseCapturedByTool = true;
        return;
    }
    if (isActive) {
        if (cameraManager.mousePressEvent(event) || selectionDisplay.mousePressEvent(event)) {
            // Event handled; do nothing.
            return;
        }
    }
}

var highlightedEntityID = null;
var mouseCapturedByTool = false;
var lastMousePosition = null;
var idleMouseTimerId = null;
var CLICK_TIME_THRESHOLD = 500 * 1000; // 500 ms
var CLICK_MOVE_DISTANCE_THRESHOLD = 8;
var IDLE_MOUSE_TIMEOUT = 200;
var DEFAULT_ENTITY_DRAG_DROP_DISTANCE = 2.0;

var lastMouseMoveEvent = null;

function mouseMoveEventBuffered(event) {
    lastMouseMoveEvent = event;
}

function mouseMove(event) {
    if (mouseDown && !mouseHasMovedSincePress) {
        var timeSincePressMicro = Date.now() - mousePressStartTime;

        var dX = mousePressStartPosition.x - event.x;
        var dY = mousePressStartPosition.y - event.y;
        var sqDist = (dX * dX) + (dY * dY);

        // If less than CLICK_TIME_THRESHOLD has passed since the mouse click AND the mouse has moved
        // less than CLICK_MOVE_DISTANCE_THRESHOLD distance, then don't register this as a mouse move
        // yet. The goal is to provide mouse clicks that are more lenient to small movements.
        if (timeSincePressMicro < CLICK_TIME_THRESHOLD && sqDist < CLICK_MOVE_DISTANCE_THRESHOLD) {
            return;
        }
        mouseHasMovedSincePress = true;
    }

    if (placingEntityID) {
        var pickRay = Camera.computePickRay(event.x, event.y);
        var distance = cameraManager.enabled ? cameraManager.zoomDistance : DEFAULT_ENTITY_DRAG_DROP_DISTANCE;
        var offset = Vec3.multiply(distance, pickRay.direction);
        var position = Vec3.sum(Camera.position, offset);
        Entities.editEntity(placingEntityID, {
            position: position,
        });
        return;
    }
    if (!isActive) {
        return;
    }
    if (idleMouseTimerId) {
        Script.clearTimeout(idleMouseTimerId);
    }

    // allow the selectionDisplay and cameraManager to handle the event first, if it doesn't handle it, then do our own thing
    if (selectionDisplay.mouseMoveEvent(event) || propertyMenu.mouseMoveEvent(event) || cameraManager.mouseMoveEvent(event)) {
        return;
    }

    lastMousePosition = {
        x: event.x,
        y: event.y
    };

    idleMouseTimerId = Script.setTimeout(handleIdleMouse, IDLE_MOUSE_TIMEOUT);
}

function handleIdleMouse() {
    idleMouseTimerId = null;
}

function highlightEntityUnderCursor(position, accurateRay) {
    var pickRay = Camera.computePickRay(position.x, position.y);
    var entityIntersection = Entities.findRayIntersection(pickRay, accurateRay);
    if (entityIntersection.accurate) {
        if (highlightedEntityID && highlightedEntityID != entityIntersection.entityID) {
            selectionDisplay.unhighlightSelectable(highlightedEntityID);
            highlightedEntityID = {
                id: -1
            };
        }

        var halfDiagonal = Vec3.length(entityIntersection.properties.dimensions) / 2.0;

        var angularSize = 2 * Math.atan(halfDiagonal / Vec3.distance(Camera.getPosition(),
            entityIntersection.properties.position)) * 180 / 3.14;

        var sizeOK = (allowLargeModels || angularSize < MAX_ANGULAR_SIZE) && (allowSmallModels || angularSize > MIN_ANGULAR_SIZE);

        if (entityIntersection.entityID && sizeOK) {
            if (wantEntityGlow) {
                Entities.editEntity(entityIntersection.entityID, {
                    glowLevel: 0.25
                });
            }
            highlightedEntityID = entityIntersection.entityID;
            selectionDisplay.highlightSelectable(entityIntersection.entityID);
        }

    }
}


function mouseReleaseEvent(event) {
    mouseDown = false;

    if (lastMouseMoveEvent) {
        mouseMove(lastMouseMoveEvent);
        lastMouseMoveEvent = null;
    }
    if (propertyMenu.mouseReleaseEvent(event) || toolBar.mouseReleaseEvent(event)) {

        return true;
    }
    if (placingEntityID) {

        if (isActive) {

            selectionManager.setSelections([placingEntityID]);
        }
        placingEntityID = null;
    }
    if (isActive && selectionManager.hasSelection()) {

        tooltip.show(false);
    }
    if (mouseCapturedByTool) {

        return;
    }

    cameraManager.mouseReleaseEvent(event);

    if (!mouseHasMovedSincePress) {
        mouseClickEvent(event);
    }
}

function mouseClickEvent(event) {
    if (isActive && event.isLeftButton) {
        var result = findClickedEntity(event);
        if (result === null) {
            if (!event.isShifted) {
                selectionManager.clearSelections();
            }
            return;
        }
        toolBar.setActive(true);
        var pickRay = result.pickRay;
        var foundEntity = result.entityID;

        var properties = Entities.getEntityProperties(foundEntity);
        if (isLocked(properties)) {
            print("Model locked " + properties.id);
        } else {
            var halfDiagonal = Vec3.length(properties.dimensions) / 2.0;

            print("Checking properties: " + properties.id + " " + " - Half Diagonal:" + halfDiagonal);
            //                P         P - Model
            //               /|         A - Palm
            //              / | d       B - unit vector toward tip
            //             /  |         X - base of the perpendicular line
            //            A---X----->B  d - distance fom axis
            //              x           x - distance from A
            //
            //            |X-A| = (P-A).B
            //            X == A + ((P-A).B)B
            //            d = |P-X|

            var A = pickRay.origin;
            var B = Vec3.normalize(pickRay.direction);
            var P = properties.position;

            var x = Vec3.dot(Vec3.subtract(P, A), B);
            var X = Vec3.sum(A, Vec3.multiply(B, x));
            var d = Vec3.length(Vec3.subtract(P, X));
            var halfDiagonal = Vec3.length(properties.dimensions) / 2.0;

            var angularSize = 2 * Math.atan(halfDiagonal / Vec3.distance(Camera.getPosition(), properties.position)) * 180 / 3.14;

            var sizeOK = (allowLargeModels || angularSize < MAX_ANGULAR_SIZE) && (allowSmallModels || angularSize > MIN_ANGULAR_SIZE);

            if (0 < x && sizeOK) {
                entitySelected = true;
                selectedEntityID = foundEntity;
                orientation = MyAvatar.orientation;
                intersection = rayPlaneIntersection(pickRay, P, Quat.getFront(orientation));


                if (!event.isShifted) {
                    selectionManager.setSelections([foundEntity]);
                } else {
                    selectionManager.addEntity(foundEntity, true);
                }

                print("Model selected: " + foundEntity);
                selectionDisplay.select(selectedEntityID, event);

                if (Menu.isOptionChecked(MENU_AUTO_FOCUS_ON_SELECT)) {
                    cameraManager.enable();
                    cameraManager.focus(selectionManager.worldPosition,
                        selectionManager.worldDimensions,
                        Menu.isOptionChecked(MENU_EASE_ON_FOCUS));
                }
            }
        }
    } else if (event.isRightButton) {
        var result = findClickedEntity(event);
        if (result) {
            var properties = Entities.getEntityProperties(result.entityID);
            if (properties.marketplaceID) {
                propertyMenu.marketplaceID = properties.marketplaceID;
                propertyMenu.updateMenuItemText(showMenuItem, "Show in Marketplace");
            } else {
                propertyMenu.marketplaceID = null;
                propertyMenu.updateMenuItemText(showMenuItem, "No marketplace info");
            }
            propertyMenu.setPosition(event.x, event.y);
            propertyMenu.show();
        } else {
            propertyMenu.hide();
        }
    }
}

Controller.mousePressEvent.connect(mousePressEvent);
Controller.mouseMoveEvent.connect(mouseMoveEventBuffered);
Controller.mouseReleaseEvent.connect(mouseReleaseEvent);


// In order for editVoxels and editModels to play nice together, they each check to see if a "delete" menu item already
// exists. If it doesn't they add it. If it does they don't. They also only delete the menu item if they were the one that
// added it.
var modelMenuAddedDelete = false;
var originalLightsArePickable = Entities.getLightsArePickable();

function setupModelMenus() {
    print("setupModelMenus()");
    // adj our menuitems
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: "Entities",
        isSeparator: true,
        grouping: "Advanced"
    });
    if (!Menu.menuItemExists("Edit", "Delete")) {
        print("no delete... adding ours");
        Menu.addMenuItem({
            menuName: "Edit",
            menuItemName: "Delete",
            shortcutKeyEvent: {
                text: "backspace"
            },
            afterItem: "Entities",
            grouping: "Advanced"
        });
        modelMenuAddedDelete = true;
    } else {
        print("delete exists... don't add ours");
    }

    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: "Entity List...",
        shortcutKey: "CTRL+META+L",
        afterItem: "Entities",
        grouping: "Advanced"
    });
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: "Allow Selecting of Large Models",
        shortcutKey: "CTRL+META+L",
        afterItem: "Entity List...",
        isCheckable: true,
        isChecked: true,
        grouping: "Advanced"
    });
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: "Allow Selecting of Small Models",
        shortcutKey: "CTRL+META+S",
        afterItem: "Allow Selecting of Large Models",
        isCheckable: true,
        isChecked: true,
        grouping: "Advanced"
    });
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: "Allow Selecting of Lights",
        shortcutKey: "CTRL+SHIFT+META+L",
        afterItem: "Allow Selecting of Small Models",
        isCheckable: true,
        grouping: "Advanced"
    });
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: "Select All Entities In Box",
        shortcutKey: "CTRL+SHIFT+META+A",
        afterItem: "Allow Selecting of Lights",
        grouping: "Advanced"
    });
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: "Select All Entities Touching Box",
        shortcutKey: "CTRL+SHIFT+META+T",
        afterItem: "Select All Entities In Box",
        grouping: "Advanced"
    });

    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: "Export Entities",
        shortcutKey: "CTRL+META+E",
        afterItem: "Entities",
        grouping: "Advanced"
    });
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: "Import Entities",
        shortcutKey: "CTRL+META+I",
        afterItem: "Export Entities",
        grouping: "Advanced"
    });
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: "Import Entities from URL",
        shortcutKey: "CTRL+META+U",
        afterItem: "Import Entities",
        grouping: "Advanced"
    });

    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: MENU_AUTO_FOCUS_ON_SELECT,
        isCheckable: true,
        isChecked: Settings.getValue(SETTING_AUTO_FOCUS_ON_SELECT) == "true",
        grouping: "Advanced"
    });
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: MENU_EASE_ON_FOCUS,
        afterItem: MENU_AUTO_FOCUS_ON_SELECT,
        isCheckable: true,
        isChecked: Settings.getValue(SETTING_EASE_ON_FOCUS) == "true",
        grouping: "Advanced"
    });
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: MENU_SHOW_LIGHTS_IN_EDIT_MODE,
        afterItem: MENU_EASE_ON_FOCUS,
        isCheckable: true,
        isChecked: Settings.getValue(SETTING_SHOW_LIGHTS_IN_EDIT_MODE) == "true",
        grouping: "Advanced"
    });
    Menu.addMenuItem({
        menuName: "Edit",
        menuItemName: MENU_SHOW_ZONES_IN_EDIT_MODE,
        afterItem: MENU_SHOW_LIGHTS_IN_EDIT_MODE,
        isCheckable: true,
        isChecked: Settings.getValue(SETTING_SHOW_ZONES_IN_EDIT_MODE) == "true",
        grouping: "Advanced"
    });

    Entities.setLightsArePickable(false);
}

setupModelMenus(); // do this when first running our script.

function cleanupModelMenus() {
    Menu.removeSeparator("Edit", "Entities");
    if (modelMenuAddedDelete) {
        // delete our menuitems
        Menu.removeMenuItem("Edit", "Delete");
    }

    Menu.removeMenuItem("Edit", "Entity List...");
    Menu.removeMenuItem("Edit", "Allow Selecting of Large Models");
    Menu.removeMenuItem("Edit", "Allow Selecting of Small Models");
    Menu.removeMenuItem("Edit", "Allow Selecting of Lights");
    Menu.removeMenuItem("Edit", "Select All Entities In Box");
    Menu.removeMenuItem("Edit", "Select All Entities Touching Box");

    Menu.removeMenuItem("Edit", "Export Entities");
    Menu.removeMenuItem("Edit", "Import Entities");
    Menu.removeMenuItem("Edit", "Import Entities from URL");

    Menu.removeMenuItem("Edit", MENU_AUTO_FOCUS_ON_SELECT);
    Menu.removeMenuItem("Edit", MENU_EASE_ON_FOCUS);
    Menu.removeMenuItem("Edit", MENU_SHOW_LIGHTS_IN_EDIT_MODE);
    Menu.removeMenuItem("Edit", MENU_SHOW_ZONES_IN_EDIT_MODE);
}

Script.scriptEnding.connect(function() {
    Settings.setValue(SETTING_AUTO_FOCUS_ON_SELECT, Menu.isOptionChecked(MENU_AUTO_FOCUS_ON_SELECT));
    Settings.setValue(SETTING_EASE_ON_FOCUS, Menu.isOptionChecked(MENU_EASE_ON_FOCUS));
    Settings.setValue(SETTING_SHOW_LIGHTS_IN_EDIT_MODE, Menu.isOptionChecked(MENU_SHOW_LIGHTS_IN_EDIT_MODE));
    Settings.setValue(SETTING_SHOW_ZONES_IN_EDIT_MODE, Menu.isOptionChecked(MENU_SHOW_ZONES_IN_EDIT_MODE));

    progressDialog.cleanup();
    toolBar.cleanup();
    cleanupModelMenus();
    tooltip.cleanup();
    selectionDisplay.cleanup();
    Entities.setLightsArePickable(originalLightsArePickable);

    Overlays.deleteOverlay(importingSVOImageOverlay);
    Overlays.deleteOverlay(importingSVOTextOverlay);
});

var lastOrientation = null;
var lastPosition = null;

// Do some stuff regularly, like check for placement of various overlays
Script.update.connect(function(deltaTime) {
    progressDialog.move();
    selectionDisplay.checkMove();
    var dOrientation = Math.abs(Quat.dot(Camera.orientation, lastOrientation) - 1);
    var dPosition = Vec3.distance(Camera.position, lastPosition);
    if (dOrientation > 0.001 || dPosition > 0.001) {
        propertyMenu.hide();
        lastOrientation = Camera.orientation;
        lastPosition = Camera.position;
    }
    if (lastMouseMoveEvent) {
        mouseMove(lastMouseMoveEvent);
        lastMouseMoveEvent = null;
    }
});

function insideBox(center, dimensions, point) {
    return (Math.abs(point.x - center.x) <= (dimensions.x / 2.0)) && (Math.abs(point.y - center.y) <= (dimensions.y / 2.0)) && (Math.abs(point.z - center.z) <= (dimensions.z / 2.0));
}

function selectAllEtitiesInCurrentSelectionBox(keepIfTouching) {
    if (selectionManager.hasSelection()) {
        // Get all entities touching the bounding box of the current selection
        var boundingBoxCorner = Vec3.subtract(selectionManager.worldPosition,
            Vec3.multiply(selectionManager.worldDimensions, 0.5));
        var entities = Entities.findEntitiesInBox(boundingBoxCorner, selectionManager.worldDimensions);

        if (!keepIfTouching) {
            var isValid;
            if (selectionManager.localPosition === null) {
                isValid = function(position) {
                    return insideBox(selectionManager.worldPosition, selectionManager.worldDimensions, position);
                }
            } else {
                isValid = function(position) {
                    var localPosition = Vec3.multiplyQbyV(Quat.inverse(selectionManager.localRotation),
                        Vec3.subtract(position,
                            selectionManager.localPosition));
                    return insideBox({
                        x: 0,
                        y: 0,
                        z: 0
                    }, selectionManager.localDimensions, localPosition);
                }
            }
            for (var i = 0; i < entities.length; ++i) {
                var properties = Entities.getEntityProperties(entities[i]);
                if (!isValid(properties.position)) {
                    entities.splice(i, 1);
                    --i;
                }
            }
        }
        selectionManager.setSelections(entities);
    }
}

function deleteSelectedEntities() {
    if (SelectionManager.hasSelection()) {
        print("  Delete Entities");
        SelectionManager.saveProperties();
        var savedProperties = [];
        for (var i = 0; i < selectionManager.selections.length; i++) {
            var entityID = SelectionManager.selections[i];
            var initialProperties = SelectionManager.savedProperties[entityID];
            SelectionManager.savedProperties[entityID];
            savedProperties.push({
                entityID: entityID,
                properties: initialProperties
            });
            Entities.deleteEntity(entityID);
        }
        SelectionManager.clearSelections();
        pushCommandForSelections([], savedProperties);
    } else {
        print("  Delete Entity.... not holding...");
    }
}

function handeMenuEvent(menuItem) {
    if (menuItem == "Allow Selecting of Small Models") {
        allowSmallModels = Menu.isOptionChecked("Allow Selecting of Small Models");
    } else if (menuItem == "Allow Selecting of Large Models") {
        allowLargeModels = Menu.isOptionChecked("Allow Selecting of Large Models");
    } else if (menuItem == "Allow Selecting of Lights") {
        Entities.setLightsArePickable(Menu.isOptionChecked("Allow Selecting of Lights"));
    } else if (menuItem == "Delete") {
        deleteSelectedEntities();
    } else if (menuItem == "Export Entities") {
        if (!selectionManager.hasSelection()) {
            Window.alert("No entities have been selected.");
        } else {
            var filename = "entities__" + Window.location.hostname + ".svo.json";
            filename = Window.save("Select where to save", filename, "*.json")
            if (filename) {
                var success = Clipboard.exportEntities(filename, selectionManager.selections);
                if (!success) {
                    Window.alert("Export failed.");
                }
            }
        }
    } else if (menuItem == "Import Entities" || menuItem == "Import Entities from URL") {

        var importURL = null;
        if (menuItem == "Import Entities") {
            var fullPath = Window.browse("Select models to import", "", "*.json");
            if (fullPath) {
                importURL = "file:///" + fullPath;
            }
        } else {
            importURL = Window.prompt("URL of SVO to import", "");
        }

        if (importURL) {
            importSVO(importURL);
        }
    } else if (menuItem == "Entity List...") {
        entityListTool.toggleVisible();
    } else if (menuItem == "Select All Entities In Box") {
        selectAllEtitiesInCurrentSelectionBox(false);
    } else if (menuItem == "Select All Entities Touching Box") {
        selectAllEtitiesInCurrentSelectionBox(true);
    } else if (menuItem == MENU_SHOW_LIGHTS_IN_EDIT_MODE) {
        lightOverlayManager.setVisible(isActive && Menu.isOptionChecked(MENU_SHOW_LIGHTS_IN_EDIT_MODE));
    } else if (menuItem == MENU_SHOW_ZONES_IN_EDIT_MODE) {
        Entities.setDrawZoneBoundaries(isActive && Menu.isOptionChecked(MENU_SHOW_ZONES_IN_EDIT_MODE));
    }
    tooltip.show(false);
}

// This function tries to find a reasonable position to place a new entity based on the camera
// position. If a reasonable position within the world bounds can't be found, `null` will
// be returned. The returned position will also take into account grid snapping settings.
function getPositionToCreateEntity() {
    var distance = cameraManager.enabled ? cameraManager.zoomDistance : DEFAULT_ENTITY_DRAG_DROP_DISTANCE;
    var direction = Quat.getFront(Camera.orientation);
    var offset = Vec3.multiply(distance, direction);
    var placementPosition = Vec3.sum(Camera.position, offset);

    var cameraPosition = Camera.position;

    var HALF_TREE_SCALE = 16384;

    var cameraOutOfBounds = Math.abs(cameraPosition.x) > HALF_TREE_SCALE || Math.abs(cameraPosition.y) > HALF_TREE_SCALE || Math.abs(cameraPosition.z) > HALF_TREE_SCALE;
    var placementOutOfBounds = Math.abs(placementPosition.x) > HALF_TREE_SCALE || Math.abs(placementPosition.y) > HALF_TREE_SCALE || Math.abs(placementPosition.z) > HALF_TREE_SCALE;

    if (cameraOutOfBounds && placementOutOfBounds) {
        return null;
    }

    placementPosition.x = Math.min(HALF_TREE_SCALE, Math.max(-HALF_TREE_SCALE, placementPosition.x));
    placementPosition.y = Math.min(HALF_TREE_SCALE, Math.max(-HALF_TREE_SCALE, placementPosition.y));
    placementPosition.z = Math.min(HALF_TREE_SCALE, Math.max(-HALF_TREE_SCALE, placementPosition.z));

    return placementPosition;
}

function importSVO(importURL) {
    print("Import URL requested: " + importURL)
    if (!Entities.canAdjustLocks()) {
        Window.alert(INSUFFICIENT_PERMISSIONS_IMPORT_ERROR_MSG);
        return;
    }

    Overlays.editOverlay(importingSVOTextOverlay, {
        visible: true
    });
    Overlays.editOverlay(importingSVOImageOverlay, {
        visible: true
    });

    var success = Clipboard.importEntities(importURL);

    if (success) {
        var VERY_LARGE = 10000;
        var position = {
            x: 0,
            y: 0,
            z: 0
        };
        if (Clipboard.getClipboardContentsLargestDimension() < VERY_LARGE) {
            position = getPositionToCreateEntity();
        }
        if (position != null) {
            var pastedEntityIDs = Clipboard.pasteEntities(position);

            if (isActive) {
                selectionManager.setSelections(pastedEntityIDs);
            }

            Window.raiseMainWindow();
        } else {
            Window.alert("Can't import objects: objects would be out of bounds.");
        }
    } else {
        Window.alert("There was an error importing the entity file.");
    }

    Overlays.editOverlay(importingSVOTextOverlay, {
        visible: false
    });
    Overlays.editOverlay(importingSVOImageOverlay, {
        visible: false
    });
}
Window.svoImportRequested.connect(importSVO);

Menu.menuItemEvent.connect(handeMenuEvent);

Controller.keyPressEvent.connect(function(event) {
    if (isActive) {
        cameraManager.keyPressEvent(event);
    }
});

Controller.keyReleaseEvent.connect(function(event) {
    if (isActive) {
        cameraManager.keyReleaseEvent(event);
    }
    // since sometimes our menu shortcut keys don't work, trap our menu items here also and fire the appropriate menu items
    if (event.text == "BACKSPACE" || event.text == "DELETE") {
        deleteSelectedEntities();
    } else if (event.text == "ESC") {
        selectionManager.clearSelections();
    } else if (event.text == "TAB") {
        selectionDisplay.toggleSpaceMode();
    } else if (event.text == "f") {
        if (isActive) {
            if (selectionManager.hasSelection()) {
                cameraManager.enable();
                cameraManager.focus(selectionManager.worldPosition,
                    selectionManager.worldDimensions,
                    Menu.isOptionChecked(MENU_EASE_ON_FOCUS));
            }
        }
    } else if (event.text == '[') {
        if (isActive) {
            cameraManager.enable();
        }
    } else if (event.text == 'g') {
        if (isActive && selectionManager.hasSelection()) {
            var newPosition = selectionManager.worldPosition;
            newPosition = Vec3.subtract(newPosition, {
                x: 0,
                y: selectionManager.worldDimensions.y * 0.5,
                z: 0
            });
            grid.setPosition(newPosition);
        }
    }
});

// When an entity has been deleted we need a way to "undo" this deletion.  Because it's not currently
// possible to create an entity with a specific id, earlier undo commands to the deleted entity
// will fail if there isn't a way to find the new entity id.
DELETED_ENTITY_MAP = {}

function applyEntityProperties(data) {
    var properties = data.setProperties;
    var selectedEntityIDs = [];
    for (var i = 0; i < properties.length; i++) {
        var entityID = properties[i].entityID;
        if (DELETED_ENTITY_MAP[entityID] !== undefined) {
            entityID = DELETED_ENTITY_MAP[entityID];
        }
        Entities.editEntity(entityID, properties[i].properties);
        selectedEntityIDs.push(entityID);
    }
    for (var i = 0; i < data.createEntities.length; i++) {
        var entityID = data.createEntities[i].entityID;
        var properties = data.createEntities[i].properties;
        var newEntityID = Entities.addEntity(properties);
        DELETED_ENTITY_MAP[entityID] = newEntityID;
        if (data.selectCreated) {
            selectedEntityIDs.push(newEntityID);
        }
    }
    for (var i = 0; i < data.deleteEntities.length; i++) {
        var entityID = data.deleteEntities[i].entityID;
        if (DELETED_ENTITY_MAP[entityID] !== undefined) {
            entityID = DELETED_ENTITY_MAP[entityID];
        }
        Entities.deleteEntity(entityID);
    }

    selectionManager.setSelections(selectedEntityIDs);
};

// For currently selected entities, push a command to the UndoStack that uses the current entity properties for the
// redo command, and the saved properties for the undo command.  Also, include create and delete entity data.
function pushCommandForSelections(createdEntityData, deletedEntityData) {
    var undoData = {
        setProperties: [],
        createEntities: deletedEntityData || [],
        deleteEntities: createdEntityData || [],
        selectCreated: true,
    };
    var redoData = {
        setProperties: [],
        createEntities: createdEntityData || [],
        deleteEntities: deletedEntityData || [],
        selectCreated: false,
    };
    for (var i = 0; i < SelectionManager.selections.length; i++) {
        var entityID = SelectionManager.selections[i];
        var initialProperties = SelectionManager.savedProperties[entityID];
        var currentProperties = Entities.getEntityProperties(entityID);
        undoData.setProperties.push({
            entityID: entityID,
            properties: {
                position: initialProperties.position,
                rotation: initialProperties.rotation,
                dimensions: initialProperties.dimensions,
            },
        });
        redoData.setProperties.push({
            entityID: entityID,
            properties: {
                position: currentProperties.position,
                rotation: currentProperties.rotation,
                dimensions: currentProperties.dimensions,
            },
        });
    }
    UndoStack.pushCommand(applyEntityProperties, undoData, applyEntityProperties, redoData);
}

PropertiesTool = function(opts) {
    var that = {};

    var url = Script.resolvePath('html/entityProperties.html');
    var webView = new OverlayWebWindow({
        title: 'Entity Properties', 
        source: url, 
        toolWindow: true
    });

    var visible = false;

    webView.setVisible(visible);

    that.setVisible = function(newVisible) {
        visible = newVisible;
        webView.setVisible(visible);
    };

    selectionManager.addEventListener(function() {
        data = {
            type: 'update',
        };
        var selections = [];
        for (var i = 0; i < selectionManager.selections.length; i++) {
            var entity = {};
            entity.id = selectionManager.selections[i];
            entity.properties = Entities.getEntityProperties(selectionManager.selections[i]);
            if (entity.properties.rotation !== undefined) {
                entity.properties.rotation = Quat.safeEulerAngles(entity.properties.rotation);
            }
            if (entity.properties.keyLight !== undefined && entity.properties.keyLight.direction !== undefined) {
                entity.properties.keyLight.direction = Vec3.multiply(RADIANS_TO_DEGREES, Vec3.toPolar(entity.properties.keyLight.direction));
                entity.properties.keyLight.direction.z = 0.0;
            }
            selections.push(entity);
        }
        data.selections = selections;
        webView.eventBridge.emitScriptEvent(JSON.stringify(data));
    });

    webView.eventBridge.webEventReceived.connect(function(data) {
        data = JSON.parse(data);
        if (data.type == "print") {
            if (data.message) {
                print(data.message);
            }
        } else if (data.type == "update") {
            selectionManager.saveProperties();
            if (selectionManager.selections.length > 1) {
                properties = {
                    locked: data.properties.locked,
                    visible: data.properties.visible,
                };
                for (var i = 0; i < selectionManager.selections.length; i++) {
                    Entities.editEntity(selectionManager.selections[i], properties);
                }
            } else {
                if (data.properties.dynamic === false) {
                    // this object is leaving dynamic, so we zero its velocities
                    data.properties["velocity"] = {x: 0, y: 0, z: 0};
                    data.properties["angularVelocity"] = {x: 0, y: 0, z: 0};
                }
                if (data.properties.rotation !== undefined) {
                    var rotation = data.properties.rotation;
                    data.properties.rotation = Quat.fromPitchYawRollDegrees(rotation.x, rotation.y, rotation.z);
                }
                if (data.properties.keyLight !== undefined && data.properties.keyLight.direction !== undefined) {
                    data.properties.keyLight.direction = Vec3.fromPolar(
                        data.properties.keyLight.direction.x * DEGREES_TO_RADIANS, data.properties.keyLight.direction.y * DEGREES_TO_RADIANS);
                }
                Entities.editEntity(selectionManager.selections[0], data.properties);
                if (data.properties.name != undefined) {
                    entityListTool.sendUpdate();
                }
            }
            pushCommandForSelections();
            selectionManager._update();
        } else if (data.type == "showMarketplace") {
            showMarketplace();
        } else if (data.type == "action") {
            if (data.action == "moveSelectionToGrid") {
                if (selectionManager.hasSelection()) {
                    selectionManager.saveProperties();
                    var dY = grid.getOrigin().y - (selectionManager.worldPosition.y - selectionManager.worldDimensions.y / 2);
                    var diff = {
                        x: 0,
                        y: dY,
                        z: 0
                    };
                    for (var i = 0; i < selectionManager.selections.length; i++) {
                        var properties = selectionManager.savedProperties[selectionManager.selections[i]];
                        var newPosition = Vec3.sum(properties.position, diff);
                        Entities.editEntity(selectionManager.selections[i], {
                            position: newPosition,
                        });
                    }
                    pushCommandForSelections();
                    selectionManager._update();
                }
            } else if (data.action == "moveAllToGrid") {
                if (selectionManager.hasSelection()) {
                    selectionManager.saveProperties();
                    for (var i = 0; i < selectionManager.selections.length; i++) {
                        var properties = selectionManager.savedProperties[selectionManager.selections[i]];
                        var bottomY = properties.boundingBox.center.y - properties.boundingBox.dimensions.y / 2;
                        var dY = grid.getOrigin().y - bottomY;
                        var diff = {
                            x: 0,
                            y: dY,
                            z: 0
                        };
                        var newPosition = Vec3.sum(properties.position, diff);
                        Entities.editEntity(selectionManager.selections[i], {
                            position: newPosition,
                        });
                    }
                    pushCommandForSelections();
                    selectionManager._update();
                }
            } else if (data.action == "resetToNaturalDimensions") {
                if (selectionManager.hasSelection()) {
                    selectionManager.saveProperties();
                    for (var i = 0; i < selectionManager.selections.length; i++) {
                        var properties = selectionManager.savedProperties[selectionManager.selections[i]];
                        var naturalDimensions = properties.naturalDimensions;

                        // If any of the natural dimensions are not 0, resize
                        if (properties.type == "Model" && naturalDimensions.x == 0 && naturalDimensions.y == 0 && naturalDimensions.z == 0) {
                            Window.alert("Cannot reset entity to its natural dimensions: Model URL" + " is invalid or the model has not yet been loaded.");
                        } else {
                            Entities.editEntity(selectionManager.selections[i], {
                                dimensions: properties.naturalDimensions,
                            });
                        }
                    }
                    pushCommandForSelections();
                    selectionManager._update();
                }
            } else if (data.action == "previewCamera") {
                if (selectionManager.hasSelection()) {
                    Camera.mode = "entity";
                    Camera.cameraEntity = selectionManager.selections[0];
                }
            } else if (data.action == "rescaleDimensions") {
                var multiplier = data.percentage / 100;
                if (selectionManager.hasSelection()) {
                    selectionManager.saveProperties();
                    for (var i = 0; i < selectionManager.selections.length; i++) {
                        var properties = selectionManager.savedProperties[selectionManager.selections[i]];
                        Entities.editEntity(selectionManager.selections[i], {
                            dimensions: Vec3.multiply(multiplier, properties.dimensions),
                        });
                    }
                    pushCommandForSelections();
                    selectionManager._update();
                }
            } else if (data.action == "reloadScript") {
                if (selectionManager.hasSelection()) {
                    var timestamp = Date.now();
                    for (var i = 0; i < selectionManager.selections.length; i++) {
                        Entities.editEntity(selectionManager.selections[i], {
                            scriptTimestamp: timestamp,
                        });
                    }
                }
            }
        }
    });

    return that;
};

PopupMenu = function() {
    var self = this;

    var MENU_ITEM_HEIGHT = 21;
    var MENU_ITEM_SPACING = 1;
    var TEXT_MARGIN = 7;

    var overlays = [];
    var overlayInfo = {};

    var upColor = {
        red: 0,
        green: 0,
        blue: 0
    };
    var downColor = {
        red: 192,
        green: 192,
        blue: 192
    };
    var overColor = {
        red: 128,
        green: 128,
        blue: 128
    };

    self.onSelectMenuItem = function() {};

    self.addMenuItem = function(name) {
        var id = Overlays.addOverlay("text", {
            text: name,
            backgroundAlpha: 1.0,
            backgroundColor: upColor,
            topMargin: TEXT_MARGIN,
            leftMargin: TEXT_MARGIN,
            width: 210,
            height: MENU_ITEM_HEIGHT,
            font: {
                size: 12
            },
            visible: false,
        });
        overlays.push(id);
        overlayInfo[id] = {
            name: name
        };
        return id;
    };

    self.updateMenuItemText = function(id, newText) {
        Overlays.editOverlay(id, {
            text: newText
        });
    };

    self.setPosition = function(x, y) {
        for (var key in overlayInfo) {
            Overlays.editOverlay(key, {
                x: x,
                y: y,
            });
            y += MENU_ITEM_HEIGHT + MENU_ITEM_SPACING;
        }
    };

    self.onSelected = function() {};

    var pressingOverlay = null;
    var hoveringOverlay = null;

    self.mousePressEvent = function(event) {
        if (event.isLeftButton) {
            var overlay = Overlays.getOverlayAtPoint({
                x: event.x,
                y: event.y
            });
            if (overlay in overlayInfo) {
                pressingOverlay = overlay;
                Overlays.editOverlay(pressingOverlay, {
                    backgroundColor: downColor
                });
            } else {
                self.hide();
            }
            return false;
        }
    };
    self.mouseMoveEvent = function(event) {
        if (visible) {
            var overlay = Overlays.getOverlayAtPoint({
                x: event.x,
                y: event.y
            });
            if (!pressingOverlay) {
                if (hoveringOverlay != null && overlay != hoveringOverlay) {
                    Overlays.editOverlay(hoveringOverlay, {
                        backgroundColor: upColor
                    });
                    hoveringOverlay = null;
                }
                if (overlay != hoveringOverlay && overlay in overlayInfo) {
                    Overlays.editOverlay(overlay, {
                        backgroundColor: overColor
                    });
                    hoveringOverlay = overlay;
                }
            }
        }
        return false;
    };
    self.mouseReleaseEvent = function(event) {
        var overlay = Overlays.getOverlayAtPoint({
            x: event.x,
            y: event.y
        });
        if (pressingOverlay != null) {
            if (overlay == pressingOverlay) {
                self.onSelectMenuItem(overlayInfo[overlay].name);
            }
            Overlays.editOverlay(pressingOverlay, {
                backgroundColor: upColor
            });
            pressingOverlay = null;
            self.hide();
        }
    };

    var visible = false;

    self.setVisible = function(newVisible) {
        if (newVisible != visible) {
            visible = newVisible;
            for (var key in overlayInfo) {
                Overlays.editOverlay(key, {
                    visible: newVisible
                });
            }
        }
    }
    self.show = function() {
        self.setVisible(true);
    }
    self.hide = function() {
        self.setVisible(false);
    }

    function cleanup() {
        for (var i = 0; i < overlays.length; i++) {
            Overlays.deleteOverlay(overlays[i]);
        }
    }

    Controller.mousePressEvent.connect(self.mousePressEvent);
    Controller.mouseMoveEvent.connect(self.mouseMoveEvent);
    Controller.mouseReleaseEvent.connect(self.mouseReleaseEvent);
    Script.scriptEnding.connect(cleanup);

    return this;
};

var propertyMenu = PopupMenu();

propertyMenu.onSelectMenuItem = function(name) {
    if (propertyMenu.marketplaceID) {
        showMarketplace(propertyMenu.marketplaceID);
    }
};

var showMenuItem = propertyMenu.addMenuItem("Show in Marketplace");

propertiesTool = PropertiesTool();
