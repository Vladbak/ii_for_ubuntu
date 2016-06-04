//
//  whiteboardSpawner.js
//  examples/homeContent/whiteboardV2
//
//  Created by Eric Levina on 2/17/16
//  Copyright 2016 High Fidelity, Inc.
//
//  Run this script to spawn a whiteboard, markers, and an eraser.
//  To draw on the whiteboard, equip a marker and hold down trigger with marker tip pointed at whiteboard 
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
var ERASER_SCRIPT_URL = Script.resolvePath("eraserEntityScript.js");

var MARKER_SCRIPT_URL = Script.resolvePath("markerEntityScript.js");

Whiteboard = function(spawnPosition, spawnRotation) {

    var orientation = Quat.fromPitchYawRollDegrees(spawnRotation.x, spawnRotation.y, spawnRotation.z);
    var markers = [];
    var markerRotation = Quat.fromVec3Degrees({
        x: spawnRotation.x + 10,
        y: spawnRotation.y - 90,
        z: spawnRotation.z
    });
    var whiteboardPosition = spawnPosition;
    var whiteboardRotation = orientation;
   
    var WHITEBOARD_MODEL_URL = "atp:/whiteboard/Whiteboard-6.fbx";
    var WHITEBOARD_COLLISION_HULL_URL = "atp:/whiteboard/whiteboardCollisionHull.obj";
   
    var whiteboard = Entities.addEntity({
        type: "Model",
        name: "home_model_whiteboard",
        modelURL: WHITEBOARD_MODEL_URL,
        position: whiteboardPosition,
        rotation: whiteboardRotation,
        shapeType: 'compound',
        compoundShapeURL: WHITEBOARD_COLLISION_HULL_URL,
        dimensions: {
            x: 1.86,
            y: 2.7,
            z: 0.4636
        },
        userData: JSON.stringify({
            'hifiHomeKey': {
                'reset': true
            }
        }),
    });



    var whiteboardSurfacePosition = Vec3.sum(whiteboardPosition, {
        x: 0.0,
        y: 0.45,
        z: 0.0
    });
    whiteboardSurfacePosition = Vec3.sum(whiteboardSurfacePosition, Vec3.multiply(-0.02, Quat.getRight(whiteboardRotation)));
    var moveForwardDistance = 0.02;
    whiteboardFrontSurfacePosition = Vec3.sum(whiteboardSurfacePosition, Vec3.multiply(-moveForwardDistance, Quat.getFront(whiteboardRotation)));
    var WHITEBOARD_SURFACE_NAME = "home_box_whiteboardDrawingSurface";
    var whiteboardSurfaceSettings = {
        type: "Box",
        name: WHITEBOARD_SURFACE_NAME,
        dimensions: {
            x: 1.82,
            y: 1.8,
            z: 0.01
        },
        color: {
            red: 200,
            green: 10,
            blue: 200
        },
        position: whiteboardFrontSurfacePosition,
        rotation: whiteboardRotation,
        visible: false,
        parentID: whiteboard,
        userData: JSON.stringify({
            'hifiHomeKey': {
                'reset': true
            }
        }),
    }
    var whiteboardFrontDrawingSurface = Entities.addEntity(whiteboardSurfaceSettings);


    whiteboardBackSurfacePosition = Vec3.sum(whiteboardSurfacePosition, Vec3.multiply(moveForwardDistance, Quat.getFront(whiteboardRotation)));
    whiteboardSurfaceSettings.position = whiteboardBackSurfacePosition;

    var whiteboardBackDrawingSurface = Entities.addEntity(whiteboardSurfaceSettings);


    var WHITEBOARD_RACK_DEPTH = 1.9;



    // ************ ERASER ************************************************
    var ERASER_MODEL_URL = "atp:/whiteboard/eraser-2.fbx";



    var eraserPosition = Vec3.sum(spawnPosition, Vec3.multiply(Quat.getFront(whiteboardRotation), -0.1));
    eraserPosition = Vec3.sum(eraserPosition, Vec3.multiply(-0.5, Quat.getRight(whiteboardRotation)));
    var eraserRotation = markerRotation;

    var eraserProps = {
        type: "Model",
        name: "home_model_whiteboardEraser",
        modelURL: ERASER_MODEL_URL,
        position: eraserPosition,
        script: ERASER_SCRIPT_URL,
        shapeType: "box",
        dimensions: {
            x: 0.0858,
            y: 0.0393,
            z: 0.2083
        },
        rotation: eraserRotation,
        dynamic: true,
        gravity: {
            x: 0,
            y: -10,
            z: 0
        },
        velocity: {
            x: 0,
            y: -0.1,
            z: 0
        },
        userData: JSON.stringify({
            'hifiHomeKey': {
                'reset': true
            },
            originalPosition: eraserPosition,
            originalRotation: eraserRotation,
            wearable: {
                joints: {
                    RightHand: [{
                        x: 0.020,
                        y: 0.120,
                        z: 0.049
                    }, {
                        x: 0.1004,
                        y: 0.6424,
                        z: 0.717,
                        w: 0.250
                    }],
                    LeftHand: [{
                        x: -0.005,
                        y: 0.1101,
                        z: 0.053
                    }, {
                        x: 0.723,
                        y: 0.289,
                        z: 0.142,
                        w: 0.610
                    }]
                }
            }
        })
    }


 // ************************************************************************************************* 

    function createMarkers() {
        var modelURLS = [
            "atp:/whiteboard/marker-blue.fbx",
            "atp:/whiteboard/marker-red.fbx",
            "atp:/whiteboard/marker-black.fbx",
        ];


        var markerPosition = Vec3.sum(spawnPosition, Vec3.multiply(Quat.getFront(whiteboardRotation), -0.1));

        createMarker(modelURLS[0], markerPosition, {
            red: 10,
            green: 10,
            blue: 200
        });

        markerPosition = Vec3.sum(markerPosition, Vec3.multiply(-0.2, Quat.getFront(markerRotation)));
        createMarker(modelURLS[1], markerPosition, {
            red: 200,
            green: 10,
            blue: 10
        });

        markerPosition = Vec3.sum(markerPosition, Vec3.multiply(0.4, Quat.getFront(markerRotation)));
        createMarker(modelURLS[2], markerPosition, {
            red: 10,
            green: 10,
            blue: 10
        });
    }


    function createMarker(modelURL, markerPosition, markerColor) {
        var marker = Entities.addEntity({
            type: "Model",
            modelURL: modelURL,
            rotation: markerRotation,
            shapeType: "box",
            name: "home_model_marker",
            dynamic: true,
            gravity: {
                x: 0,
                y: -5,
                z: 0
            },
            velocity: {
                x: 0,
                y: -0.1,
                z: 0
            },
            position: markerPosition,
            dimensions: {
                x: 0.027,
                y: 0.027,
                z: 0.164
            },
            script: MARKER_SCRIPT_URL,
            userData: JSON.stringify({
                'hifiHomeKey': {
                    'reset': true
                },
                originalPosition: markerPosition,
                originalRotation: markerRotation,
                markerColor: markerColor,
                wearable: {
                    joints: {
                        RightHand: [{
                            x: 0.001,
                            y: 0.139,
                            z: 0.050
                        }, {
                            x: -0.73,
                            y: -0.043,
                            z: -0.108,
                            w: -0.666
                        }],
                        LeftHand: [{
                            x: 0.007,
                            y: 0.151,
                            z: 0.061
                        }, {
                            x: -0.417,
                            y: 0.631,
                            z: -0.389,
                            w: -0.525
                        }]
                    }
                }
            })
        });

        markers.push(marker);

    }
    var eraser;
    Script.setTimeout(function() {
        eraser = Entities.addEntity(eraserProps);
        createMarkers();
    }, 1500)

    function cleanup() {
        print('WHITEBOARD CLEANUP')
        Entities.deleteEntity(whiteboard);
        Entities.deleteEntity(whiteboardFrontDrawingSurface);
        Entities.deleteEntity(whiteboardBackDrawingSurface);
        Entities.deleteEntity(eraser);
        markers.forEach(function(marker) {
            Entities.deleteEntity(marker);
        });
    }

    this.cleanup = cleanup;

    print('CREATED WHITEBOARD')
}