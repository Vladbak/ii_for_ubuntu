//  Copyright 2016 High Fidelity, Inc.
//
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

var version = 1211;

var WORLD_OFFSET = {
    x: 0,
    y: 0,
    z: 0
}

var WORLD_SCALE_AMOUNT = 1.0;

function offsetVectorToWorld(vector) {
    var newVector;

    newVector = Vec3.sum(vector, WORLD_OFFSET);

    return newVector
}

function scaleVectorToWorld(vector) {
    var newVector;

    newVector = Vec3.multiply(vector, WORLD_SCALE_AMOUNT);

    return newVector
}

function transformToSmallerWorld(vector) {
    var newVector = offsetVectorToWorld(vector);
    // newVector = scaleVectorToWorld(newVector);
    return newVector;
}

var cellLayout;
var baseLocation = "https://hifi-content.s3.amazonaws.com/DomainContent/CellScience/";

var utilsScript = Script.resolvePath('Scripts/utils.js');
Script.include(utilsScript);

assignVariables();

var locations = {
    cellLayout: [offsetVectorToWorld({
        x: 3000,
        y: 13500,
        z: 3000
    }), offsetVectorToWorld({
        x: 3276.6,
        y: 13703.3,
        z: 4405.6
    }), 1800],
    cells: [offsetVectorToWorld({
        x: 13500,
        y: 13500,
        z: 13500
    }), offsetVectorToWorld({
        x: 13501,
        y: 13501,
        z: 13501
    }), 400],
    ribosome: [offsetVectorToWorld({
        x: 13500,
        y: 3000,
        z: 3000
    }), offsetVectorToWorld({
        x: 13685,
        y: 3248,
        z: 2861
    }), 1000],
    hexokinase: [offsetVectorToWorld({
        x: 3000,
        y: 3000,
        z: 13500
    }), offsetVectorToWorld({
        x: 2755,
        y: 3121,
        z: 13501
    }), 2000],
    mitochondria: [offsetVectorToWorld({
        x: 3000,
        y: 13500,
        z: 3000
    }), offsetVectorToWorld({
        x: 3240,
        y: 13519,
        z: 3874
    }), 1000],
    translation: [offsetVectorToWorld({
        x: 3000,
        y: 13500,
        z: 3000
    }), offsetVectorToWorld({
        x: 2962,
        y: 13492,
        z: 3342
    }), 1000]
};

print('JBP locations locations' + JSON.stringify(locations))

var scenes = [{
    name: "Cells",
    objects: "",
    location: locations.cells[0],
    entryPoint: locations.cells[1],
    zone: {
        dimensions: {
            x: 4000,
            y: 4000,
            z: 4000
        },
        light: {
            r: 255,
            g: 200,
            b: 200
        },
        intensity: 1.1,
        ambient: 0.7,
        sun: true,
        skybox: "cells_skybox_cross"
    },
    instances: [{
        model: "Cell",
        dimensions: {
            x: 500,
            y: 570,
            z: 500
        },
        offset: {
            x: 0,
            y: 0,
            z: 0
        },
        radius: 500,
        number: 10,
        userData: JSON.stringify({
            entryPoint: locations.cellLayout[1],
            target: locations.cellLayout[1],
            location: locations.cellLayout[1],
            baseURL: baseLocation
        }),
        script: "zoom.js?" + version,
        visible: true
    }],
    boundary: {
        radius: locations.cells[2],
        center: locations.cells[0],
        location: locations.cellLayout[1],
        target: locations.cellLayout[0]
    }
}, {
    name: "CellLayout",
    objects: cellLayout,
    location: locations.cellLayout[0],
    entryPoint: locations.cellLayout[1],
    zone: {
        dimensions: {
            x: 4000,
            y: 4000,
            z: 4000
        },
        light: {
            r: 247,
            g: 233,
            b: 220
        },
        intensity: 2.3,
        ambient: 0.7,
        sun: true,
        skybox: "cosmos_skybox_blurred"
    },
    instances: [{
        model: "translation",
        dimensions: {
            x: 10,
            y: 16,
            z: 10
        },
        offset: {
            x: 0,
            y: 0,
            z: 0
        },
        radius: 300,
        number: 7,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            },
            target: locations.ribosome[1],
            location: locations.ribosome[0],
            baseURL: baseLocation
        }),
        script: "zoom.js?" + version,
        visible: true
    }, {
        model: "vesicle",
        dimensions: {
            x: 60,
            y: 60,
            z: 60
        },
        randomSize: 10,
        offset: {
            x: 0,
            y: 0,
            z: 0
        },
        radius: 1000,
        number: 22,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            }
        }),
        visible: true
    }, { //golgi vesicles
        model: "vesicle",
        dimensions: {
            x: 10,
            y: 10,
            z: 10
        },
        randomSize: 10,
        offset: {
            x: -319,
            y: 66,
            z: 976
        },
        radius: 140,
        number: 10,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            }
        }),
        script: "",
        visible: true
    }, { //golgi vesicles
        model: "vesicle",
        dimensions: {
            x: 15,
            y: 15,
            z: 15
        },
        randomSize: 10,
        offset: {
            x: -319,
            y: 66,
            z: 976
        },
        radius: 115,
        number: 7,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            }
        }),
        visible: true
    }, {
        model: "vesicle",
        dimensions: {
            x: 50,
            y: 50,
            z: 50
        },
        randomSize: 10,
        offset: {
            x: 0,
            y: 0,
            z: 0
        },
        radius: 600,
        number: 15,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            }
        }),
        script: "",
        visible: true
    }, { //outer vesicles
        model: "vesicle",
        dimensions: {
            x: 60,
            y: 60,
            z: 60
        },
        randomSize: 10,
        offset: {
            x: 0,
            y: 0,
            z: 0
        },
        radius: 1600,
        number: 22,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            }
        }),
        script: "",
        visible: true
    }, { //outer vesicles
        model: "vesicle",
        dimensions: {
            x: 40,
            y: 40,
            z: 40
        },
        randomSize: 10,
        offset: {
            x: 0,
            y: 0,
            z: 0
        },
        radius: 1400,
        number: 22,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            }
        }),
        visible: true
    }, { //outer vesicles
        model: "vesicle",
        dimensions: {
            x: 80,
            y: 80,
            z: 80
        },
        randomSize: 10,
        offset: {
            x: 0,
            y: 0,
            z: 0
        },
        radius: 1800,
        number: 22,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            }
        }),
        visible: true
    }, {
        model: "hexokinase",
        dimensions: {
            x: 3,
            y: 4,
            z: 3
        },
        randomSize: 10,
        offset: {
            x: 236,
            y: 8,
            z: 771
        },
        radius: 80,
        number: 7,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            },
            target: locations.hexokinase[1],
            location: locations.hexokinase[0],
            baseURL: baseLocation
        }),
        script: "zoom.js?" + version,
        visible: true
    }, {
        model: "pfructo_kinase",
        dimensions: {
            x: 3,
            y: 4,
            z: 3
        },
        randomSize: 10,
        offset: {
            x: 236,
            y: 8,
            z: 771
        },
        radius: 60,
        number: 7,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            },
            target: locations.hexokinase[1],
            location: locations.hexokinase[0],
        }),
        script: "zoom.js?" + version,
        visible: true
    }, {
        model: "glucose_isomerase",
        dimensions: {
            x: 3,
            y: 4,
            z: 3
        },
        randomSize: 10,
        offset: {
            x: 236,
            y: 8,
            z: 771
        },
        radius: 70,
        number: 7,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            },
            target: locations.hexokinase[1],
            location: locations.hexokinase[0],
        }),
        script: "zoom.js?" + version,
        visible: true
    }],
    boundary: {
        radius: locations.cellLayout[2],
        center: locations.cellLayout[0],
        location: locations.cells[1],
        target: locations.cells[0]
    }
}, {
    name: "Ribosome",
    objects: "",
    location: locations.ribosome[0],
    entryPoint: locations.ribosome[1],
    zone: {
        dimensions: {
            x: 4000,
            y: 4000,
            z: 4000
        },
        light: {
            r: 250,
            g: 185,
            b: 182
        },
        intensity: 0.6,
        ambient: 2.9,
        sun: true,
        skybox: "ribosome_skybox"
    },
    instances: [{
        model: "translation_highres",
        dimensions: {
            x: 500,
            y: 500,
            z: 200
        },
        offset: {
            x: 0,
            y: 0,
            z: 0
        },
        radius: 1,
        number: 1,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            }
        }),
        script: "",
        visible: true
    }],
    boundary: {
        radius: locations.ribosome[2],
        center: locations.ribosome[0],
        location: locations.translation[1],
        target: locations.translation[0]
    }
}, {
    name: "Hexokinase",
    objects: "",
    location: locations.hexokinase[0],
    entryPoint: locations.hexokinase[1],
    zone: {
        dimensions: {
            x: 4000,
            y: 4000,
            z: 4000
        },
        light: {
            r: 255,
            g: 255,
            b: 255
        },
        intensity: 0.6,
        ambient: 0.6,
        sun: true,
        skybox: "hexokinase_skybox"
    },
    instances: [{
        model: "hexokinase_highres",
        dimensions: {
            x: 600,
            y: 600,
            z: 600
        },
        offset: {
            x: 0,
            y: 0,
            z: 0
        },
        radius: 1,
        number: 1,
        userData: JSON.stringify({
            grabbableKey: {
                grabbable: false
            }
        }),
        script: "",
        visible: true
    }],
    boundary: {
        radius: locations.hexokinase[2],
        center: locations.hexokinase[0],
        location: locations.mitochondria[1],
        target: locations.mitochondria[0]
    }
}];


function ImportScene(scene) {

    var sceneDataLines = scene.objects.split(";");
    for (var i = 1; i < sceneDataLines.length; i++) {
        var data = sceneDataLines[i].split(",");
        var posX = Number(data[1]) + scene.location.x;
        var posY = Number(data[2]) + scene.location.y;
        var posZ = Number(data[3]) + scene.location.z;
        var url = baseLocation + scene.name + "/" + data[0] + ".fbx?" + version;
        var position = {
            x: posX,
            y: posY,
            z: posZ
        };
        var dimensions = {
            x: data[4],
            y: data[5],
            z: data[6]
        };
        var rotation = Quat.fromPitchYawRollDegrees(data[7], data[8], data[9]);
        var name = data[0].replace(/[0-9]/g, '');

        var labelDistance = (Number(data[4]) + Number(data[5]) + Number(data[6])) / 3 + 50;
        var idDimensions = {
            x: Number(dimensions.x) + 200,
            y: Number(dimensions.y) + 200,
            z: Number(dimensions.z) + 200
        };



        //print ("Label distance is " + labelDistance + " for " + name);
        if (name != "" && name != "NPC" && name != "undefined" && name != "microtubule")
            CreateIdentification(name, position, {
                x: 0,
                y: 0,
                z: 0
            }, idDimensions, labelDistance);

        CreateEntity(data[0], position, rotation, dimensions, url, "", "", true);

    }



    if (scene.name == "CellLayout") {
        MakeMTEnds();
        MakeMTLabels();
    }
    //create zone and instances

    CreateZone(scene);
    CreateInstances(scene);
    // CreateBoundary(scene);

    // print("done " + scene.name);

}

function createLayoutLights() {
    Entities.addEntity({
        type: "Light",
        name: "Cell layout light",
        position: offsetVectorToWorld({
            x: 3110,
            y: 10660,
            z: 3785
        }),
        dimensions: {
            x: 1500,
            y: 1500,
            z: 1500
        },
        intensity: 0.5,
        color: {
            red: 240,
            green: 165,
            blue: 240
        }
    })

}

function CreateNavigationButton(scene, number) {

    var nav = Entities.addEntity({
        type: "Box",
        name: scene.name + " navigation button",
        color: {
            red: 200,
            green: 0,
            blue: 0
        },
        dimensions: {
            x: 16000,
            y: 16000,
            z: 16000
        },
        visible: false,
        userData: JSON.stringify({
            name: scene.name,
            entryPoint: scene.entryPoint,
            target: scene.location,
            offset: number,
            baseURL: baseLocation,
            grabbableKey: {
                grabbable: false
            }
        }),
        position: {
            x: 0,
            y: 0,
            z: 0
        },
        script: baseLocation + "Scripts/navigationButton.js?" + version,
        collisionless: true,
    });
    print('JBP CREATE NAV AT::' + nav + " name: " + scene.name + ": " + JSON.stringify(scene.location))
}

function CreateBoundary(scene) {
    // print('CREATING BOUNDARY FOR SCENE '+ JSON.stringify(scene));
    var pts = getEvenlyDistributedPointsOnSphere(80);

    for (var i = 0; i < pts.length; i++) {

        var boundPos = {
            x: scene.boundary.center.x + scene.boundary.radius * pts[i].x,
            y: scene.boundary.center.y + scene.boundary.radius * pts[i].y,
            z: scene.boundary.center.z + scene.boundary.radius * pts[i].z
        };
        var script = "zoom.js?" + version;
        var dimensions = {
            x: 0.6 * scene.boundary.radius,
            y: 0.6 * scene.boundary.radius,
            z: 100
        };
        var boundRot = getLookRotation(boundPos, scene.boundary.center);
        var data = JSON.stringify({
            target: scene.boundary.target,
            location: scene.boundary.location,
            baseURL: baseLocation,
        });

        Entities.addEntity({
            type: "Sphere",
            name: "boundary",
            color: {
                red: 200,
                green: 0,
                blue: 0
            },
            position: boundPos,
            rotation: boundRot,
            dimensions: dimensions,
            visible: false,
            userData: data,
            script: baseLocation + "Scripts/" + script,
            collisionless: true,

        });
    }

}

function getLookRotation(loc, targetPos) {

    var direction = Vec3.normalize(Vec3.subtract(loc, targetPos));

    var pitch = Quat.angleAxis(Math.asin(-direction.y) * 180.0 / Math.PI, {
        x: 1,
        y: 0,
        z: 0
    });
    var yaw = Quat.angleAxis(Math.atan2(direction.x, direction.z) * 180.0 / Math.PI, {
        x: 0,
        y: 1,
        z: 0
    });

    return Quat.multiply(yaw, pitch);
}


function getEvenlyDistributedPointsOnSphere(numPoints) {

    var points = [];

    var inc = Math.PI * (3 - Math.sqrt(5));
    var off = 2 / numPoints;

    for (var i = 0; i < numPoints; i++) {
        var y = i * off - 1 + (off / 2);
        var r = Math.sqrt(1 - y * y);
        var phi = i * inc;

        points[i] = {
            x: Math.cos(phi) * r,
            y: y,
            z: Math.sin(phi) * r
        };
    }

    return points;
}

function deleteAllInRadius(position, radius) {
    var n = 0;
    var arrayFound = Entities.findEntities(position, radius);
    for (var i = 0; i < arrayFound.length; i++) {

        Entities.deleteEntity(arrayFound[i]);
    }
    // print("deleted " + arrayFound.length + " entities");
}

function CreateInstances(scene) {
    for (var i = 0; i < scene.instances.length; i++) {

        for (var j = 0; j < scene.instances[i].number; j++) {
            var point = getPointOnSphereOfRadius(scene.instances[i].radius, j + 2, scene.instances[i].number);
            // print(scene.name + " point is " + point.x + " " + point.y + " " + point.z);

            var posX = scene.location.x + point.x + scene.instances[i].offset.x + Math.random() * (scene.instances[i].radius / 10);
            var posY = scene.location.y + point.y + scene.instances[i].offset.y + Math.random() * (scene.instances[i].radius / 10);
            var posZ = scene.location.z + point.z + scene.instances[i].offset.z + Math.random() * (scene.instances[i].radius / 10);
            var position = {
                x: posX,
                y: posY,
                z: posZ
            };
            var url = baseLocation + "Instances/" + scene.instances[i].model + ".fbx?" + version;
            var script = scene.instances[i].script;
            var rotX = 360 * Math.random() - 180;
            var rotY = 360 * Math.random() - 180;
            var rotZ = 360 * Math.random() - 180;
            var rotation = Quat.fromPitchYawRollDegrees(rotX, rotY, rotZ);
            var dimensions;
            if (scene.instances[i].script === "") {
                script = "";
            }
            //is there a smarter way to randomize size?
            //          if (scene.instances[i].randomSize){
            //              dimensions = {x:scene.instances[i].dimensions.x }
            //          }
            if (scene.instances[i].model == "vesicle") {
                var idBounds = {
                    x: scene.instances[i].dimensions.x + 100,
                    y: scene.instances[i].dimensions.y + 100,
                    z: scene.instances[i].dimensions.z + 100
                };
                CreateIdentification(scene.instances[i].model, position, {
                    x: 0,
                    y: 0,
                    z: 0
                }, idBounds, 150, scene.instances[i]);

            }
            //print('SCRIPT AT CREATE ENTITY: ' + script)
            CreateEntity(scene.instances[i].model, position, rotation, scene.instances[i].dimensions, url, script, scene.instances[i].userData, scene.instances[i].visible);
        }
    }
}



function CreateIdentification(name, position, rotation, dimensions, showDistance) {
    //print ("creating ID for " + name);

    Entities.addEntity({
        type: "Sphere",
        name: "ID for " + name,
        color: {
            red: 200,
            green: 0,
            blue: 0
        },
        dimensions: dimensions,
        position: position,
        rotation: rotation,
        visible: false,
        userData: JSON.stringify({
            showDistance: showDistance,
            name: name,
            baseURL: baseLocation,
        }),
        script: baseLocation + "Scripts/showIdentification.js?" + version,
        collisionless: true,

    });

}


function getPointOnSphereOfRadius(radius, number, totalNumber) {

    //  var phi = 2 * Math.PI * Math.random();
    //  var theta = Math.PI * Math.random();
    //  return {
    //      x: radius * Math.sin(theta) * Math.cos(phi),
    //      y: radius * Math.sin(theta) * Math.sin(phi),
    //      z: radius * Math.cos(theta)
    //  };



    var inc = Math.PI * (3 - Math.sqrt(5));
    var off = 2 / totalNumber;


    var y = number * off - 1 + (off / 2);
    var r = Math.sqrt(1 - y * y);
    var phi = number * inc;

    // print("inc " + inc + " off " + off + " y " + y + " r " + r + " phi " + phi);

    if (isNaN(r)) {
        //print("r is not a number");
        r = 1;
    }

    // print("cos phi is " + Math.cos(phi));
    return {
        x: Math.cos(phi) * r * radius,
        y: y * radius,
        z: Math.sin(phi) * r * radius
    };



}

function CreateEntity(name, position, rotation, dimensions, url, script, userData, visible) {
    var scriptLocation;
    if (script === "") {
        scriptLocation = "";
    } else {
        scriptLocation = baseLocation + "Scripts/" + script;
    }

    //print(' SCRIPT LOCATION IN CREATE ENTITY' + scriptLocation)
    Entities.addEntity({
        type: "Model",
        name: name,
        position: position,
        rotation: rotation,
        dimensions: dimensions,
        modelURL: url,
        visible: visible,
        animationURL: url,
        animationIsPlaying: true,
        animationFrame: Math.random() * 1000,
        angularDamping: 0,
        linearDamping: 0,
        animationSettings: JSON.stringify({
            firstFrame: 0,
            lastFrame: 1000,
            fps: 30,
            frameIndex: Math.random() * 1000,
            hold: false,
            loop: true,
            running: true,
            startAutomatically: true
        }),
        userData: userData,
        script: scriptLocation,
        collisionless: true,
        shapeType: "compound",
        compoundShapeURL: url
    });

    //  print("added " + name + " at (" + position.x + ", " + position.y + ", " + position.z + ")");

}

function CreateZone(scene) {
    //  print ("Creating " + scene.name + " zone.........................");
    Entities.addEntity({
        type: "Zone",
        name: scene.name + " Zone",
        position: {
            x: scene.location.x,
            y: scene.location.y,
            z: scene.location.z
        },
        dimensions: {
            x: scene.zone.dimensions.x,
            y: scene.zone.dimensions.y,
            z: scene.zone.dimensions.z
        },
        keyLightColor: {
            red: scene.zone.light.r,
            green: scene.zone.light.g,
            blue: scene.zone.light.b
        },
        keyLightDirection: {
            x: 0,
            y: -1.0,
            z: 0
        },
        keyLightIntensity: scene.zone.intensity,
        keyLightAmbientIntensity: scene.zone.ambient,
        backgroundMode: "skybox",
        skybox: {
            color: {
                red: 232,
                green: 209,
                blue: 235
            },
            url: baseLocation + "Skyboxes/" + scene.zone.skybox + ".jpg?" + version
        },
        stage: {
            latitude: 37.777,
            longitude: 122.407,
            altitude: 0.03,
            day: 60,
            hour: 12,
            sunModelEnabled: true
        }
    });
}


function MakeMTEnds() {

    for (var mt = 0; mt < curves.length; mt++) {
        // print("creating MT end " + mt);
        MakeMTEnd(curves[mt][0], 0, true);
        MakeMTEnd(curves[mt][curves[mt].length - 1], 1, false);
    }

}

var mtEndURL = "Tubulin/tubulin.fbx?" + version;
var mtEndDimensions = {
    x: 12,
    y: 12,
    z: 24
};

function MakeMTEnd(segment, t, end) {

    var endOffset = 8;
    if (end) {
        endOffset = -8;
    }

    var pos = GetPositionOnMT(segment, t);
    var rotation = GetRotationOnMT(segment, t);

    //make it
    Entities.addEntity({
        type: "Model",
        name: "MT end",
        position: {
            x: pos.x + endOffset * rotation.tangent.x,
            y: pos.y + endOffset * rotation.tangent.y,
            z: pos.z + endOffset * rotation.tangent.z
        },
        rotation: rotation.rot,
        dimensions: mtEndDimensions,
        modelURL: baseLocation + mtEndURL
    });
}


function MakeMTLabels() {

    for (var mt = 0; mt < curves.length; mt++) {
        for (var segment = 0; segment < curves[mt].length; segment++) {
            if (segment == 0) {
                MakeMTLabel(curves[mt][segment], 0);
            }
            MakeMTLabel(curves[mt][segment], 1);
        }
    }

}

function MakeMTLabel(segment, t) {

    labelDistance = 150;
    idDimensions = {
        x: 75,
        y: 75,
        z: 100
    };

    CreateIdentification("microtubule", GetPositionOnMT(segment, t), GetRotationOnMT(segment, t).rot, idDimensions, labelDistance);

}

function GetPositionOnMT(segment, t) {

    return pos = {
        x: (1 - t) * (1 - t) * (1 - t) * (locations.cellLayout[0].x + segment[0].x) + 3 * t * (1 - t) * (1 - t) * (locations.cellLayout[0].x + segment[1].x) + 3 * t * t * (1 - t) * (locations.cellLayout[0].x + segment[2].x) + t * t * t * (locations.cellLayout[0].x + segment[3].x),
        y: (1 - t) * (1 - t) * (1 - t) * (locations.cellLayout[0].y + segment[0].y) + 3 * t * (1 - t) * (1 - t) * (locations.cellLayout[0].y + segment[1].y) + 3 * t * t * (1 - t) * (locations.cellLayout[0].y + segment[2].y) + t * t * t * (locations.cellLayout[0].y + segment[3].y),
        z: (1 - t) * (1 - t) * (1 - t) * (locations.cellLayout[0].z + segment[0].z) + 3 * t * (1 - t) * (1 - t) * (locations.cellLayout[0].z + segment[1].z) + 3 * t * t * (1 - t) * (locations.cellLayout[0].z + segment[2].z) + t * t * t * (locations.cellLayout[0].z + segment[3].z)
    }

}

function GetRotationOnMT(segment, t) {

    var dir = -1;
    if (t < 0.5) {
        dir = 1;
    }
    var tangent = Vec3.normalize({
        x: 3 * (1 - t) * (1 - t) * ((locations.cellLayout[0].x + segment[1].x) - (locations.cellLayout[0].x + segment[0].x)) + 6 * t * (1 - t) * ((locations.cellLayout[0].x + segment[2].x) - (locations.cellLayout[0].x + segment[1].x)) + 3 * t * t * ((locations.cellLayout[0].x + segment[3].x) - (locations.cellLayout[0].x + segment[2].x)),
        y: 3 * (1 - t) * (1 - t) * ((locations.cellLayout[0].y + segment[1].y) - (locations.cellLayout[0].y + segment[0].y)) + 6 * t * (1 - t) * ((locations.cellLayout[0].y + segment[2].y) - (locations.cellLayout[0].y + segment[1].y)) + 3 * t * t * ((locations.cellLayout[0].y + segment[3].y) - (locations.cellLayout[0].y + segment[2].y)),
        z: 3 * (1 - t) * (1 - t) * ((locations.cellLayout[0].z + segment[1].z) - (locations.cellLayout[0].z + segment[0].z)) + 6 * t * (1 - t) * ((locations.cellLayout[0].z + segment[2].z) - (locations.cellLayout[0].z + segment[1].z)) + 3 * t * t * ((locations.cellLayout[0].z + segment[3].z) - (locations.cellLayout[0].z + segment[2].z))
    });
    var pitch = Quat.angleAxis(Math.asin(dir * -tangent.y) * 180.0 / Math.PI, {
        x: 1,
        y: 0,
        z: 0
    });
    var yaw = Quat.angleAxis(Math.atan2(dir * tangent.x, dir * tangent.z) * 180.0 / Math.PI, {
        x: 0,
        y: 1,
        z: 0
    });

    return {
        rot: Quat.multiply(yaw, pitch),
        tangent: tangent
    };

}


var curves = [
    [
        [{
            x: -495.1377539,
            y: 447.6999851,
            z: 67.71881918
        }, {
            x: -459.8060244,
            y: 430.0299175,
            z: 58.61231833
        }, {
            x: -430.0561194,
            y: 393.4288088,
            z: 58.18999042
        }, {
            x: -396.3439356,
            y: 368.0364237,
            z: 55.07196935
        }],
        [{
            x: -396.3439356,
            y: 368.0364237,
            z: 55.07196935
        }, {
            x: -371.0597978,
            y: 348.9921349,
            z: 52.73345354
        }, {
            x: -343.5468781,
            y: 336.252753,
            z: 48.87861033
        }, {
            x: -313.7608397,
            y: 326.6089999,
            z: 39.67528364
        }],
        [{
            x: -313.7608397,
            y: 326.6089999,
            z: 39.67528364
        }, {
            x: -283.2951845,
            y: 316.7452089,
            z: 30.26196812
        }, {
            x: -250.4514975,
            y: 310.1199219,
            z: 15.25331604
        }, {
            x: -221.5619658,
            y: 302.4919751,
            z: 6.625614572
        }],
        [{
            x: -221.5619658,
            y: 302.4919751,
            z: 6.625614572
        }, {
            x: -191.9958819,
            y: 294.6853925,
            z: -2.204135545
        }, {
            x: -166.5713233,
            y: 285.8286384,
            z: -4.350569745
        }, {
            x: -145.8284432,
            y: 264.5135505,
            z: -12.40980801
        }],
        [{
            x: -145.8284432,
            y: 264.5135505,
            z: -12.40980801
        }, {
            x: -133.0044557,
            y: 251.3358039,
            z: -17.39231622
        }, {
            x: -121.9698772,
            y: 233.3962915,
            z: -24.63478854
        }, {
            x: -111.9765579,
            y: 220.2156326,
            z: -34.7149099
        }],
        [{
            x: -111.9765579,
            y: 220.2156326,
            z: -34.7149099
        }, {
            x: -97.01720198,
            y: 200.4850343,
            z: -49.80420286
        }, {
            x: -84.39111651,
            y: 191.4181523,
            z: -71.25214612
        }, {
            x: -63.5035664,
            y: 185.1529929,
            z: -93.19394907
        }],
        [{
            x: -63.5035664,
            y: 185.1529929,
            z: -93.19394907
        }, {
            x: -44.42551249,
            y: 179.4305865,
            z: -113.2349255
        }, {
            x: -18.45538061,
            y: 176.0455007,
            z: -133.6879014
        }, {
            x: 2.988566987,
            y: 160.6771648,
            z: -149.4073571
        }],
        [{
            x: 2.988566987,
            y: 160.6771648,
            z: -149.4073571
        }, {
            x: 16.05317726,
            y: 151.3140885,
            z: -158.9843512
        }, {
            x: 27.4377628,
            y: 137.503081,
            z: -166.8043619
        }, {
            x: 39.12809334,
            y: 129.893086,
            z: -175.4108522
        }]
    ],
    [
        [{
            x: -502.782479,
            y: 419.211002,
            z: 1.244313114
        }, {
            x: -500.5011342,
            y: 426.9581177,
            z: 22.97154761
        }, {
            x: -493.6620063,
            y: 427.5269764,
            z: 46.59112475
        }, {
            x: -484.2420008,
            y: 422.3450495,
            z: 68.19268056
        }],
        [{
            x: -484.2420008,
            y: 422.3450495,
            z: 68.19268056
        }, {
            x: -473.3807231,
            y: 416.3702814,
            z: 93.09930031
        }, {
            x: -459.0883963,
            y: 402.7503513,
            z: 115.3231388
        }, {
            x: -439.1375769,
            y: 388.320256,
            z: 124.0995719
        }],
        [{
            x: -439.1375769,
            y: 388.320256,
            z: 124.0995719
        }, {
            x: -421.2093247,
            y: 375.35305,
            z: 131.986271
        }, {
            x: -398.7117145,
            y: 361.7316177,
            z: 129.0138927
        }, {
            x: -386.3977195,
            y: 341.5972231,
            z: 134.6455722
        }],
        [{
            x: -386.3977195,
            y: 341.5972231,
            z: 134.6455722
        }, {
            x: -374.5129163,
            y: 322.1645922,
            z: 140.0809652
        }, {
            x: -372.1142204,
            y: 296.6650923,
            z: 153.5310969
        }, {
            x: -368.5105956,
            y: 269.527359,
            z: 165.0896717
        }],
        [{
            x: -368.5105956,
            y: 269.527359,
            z: 165.0896717
        }, {
            x: -364.7513477,
            y: 241.2176782,
            z: 177.1474055
        }, {
            x: -359.6808536,
            y: 211.1252139,
            z: 187.1466802
        }, {
            x: -346.5997244,
            y: 191.7226816,
            z: 192.337033
        }],
        [{
            x: -346.5997244,
            y: 191.7226816,
            z: 192.337033
        }, {
            x: -329.5847406,
            y: 166.4852754,
            z: 199.0882673
        }, {
            x: -299.0166281,
            y: 159.3340786,
            z: 197.7033256
        }, {
            x: -283.2065466,
            y: 132.268229,
            z: 200.2987292
        }]
    ],
    [
        [{
            x: -498.1661216,
            y: 448.2948983,
            z: 68.07753142
        }, {
            x: -480.969939,
            y: 434.597862,
            z: 91.93436888
        }, {
            x: -462.0873198,
            y: 416.3863837,
            z: 111.6850532
        }, {
            x: -430.4447784,
            y: 401.7816162,
            z: 127.2140427
        }],
        [{
            x: -430.4447784,
            y: 401.7816162,
            z: 127.2140427
        }, {
            x: -413.4109253,
            y: 393.9195592,
            z: 135.573628
        }, {
            x: -392.679377,
            y: 387.1026901,
            z: 142.7098093
        }, {
            x: -377.1998096,
            y: 378.8878881,
            z: 151.4955421
        }],
        [{
            x: -377.1998096,
            y: 378.8878881,
            z: 151.4955421
        }, {
            x: -361.1299593,
            y: 370.3598308,
            z: 160.6163017
        }, {
            x: -350.7202749,
            y: 360.3251929,
            z: 171.5148162
        }, {
            x: -345.9682977,
            y: 348.6211265,
            z: 185.1895909
        }],
        [{
            x: -345.9682977,
            y: 348.6211265,
            z: 185.1895909
        }, {
            x: -338.7420105,
            y: 330.8228622,
            z: 205.9846928
        }, {
            x: -344.5991714,
            y: 309.1640443,
            z: 233.1998963
        }, {
            x: -342.3661924,
            y: 298.3047114,
            z: 259.8801776
        }],
        [{
            x: -342.3661924,
            y: 298.3047114,
            z: 259.8801776
        }, {
            x: -340.360668,
            y: 288.5515262,
            z: 283.842766
        }, {
            x: -331.8292095,
            y: 287.5097759,
            z: 307.373858
        }, {
            x: -322.8468416,
            y: 285.6250259,
            z: 330.5470392
        }],
        [{
            x: -322.8468416,
            y: 285.6250259,
            z: 330.5470392
        }, {
            x: -310.143855,
            y: 282.9595868,
            z: 363.3188662
        }, {
            x: -296.5390493,
            y: 278.6081482,
            z: 395.3748716
        }, {
            x: -282.3863571,
            y: 281.1825088,
            z: 427.1718672
        }],
        [{
            x: -282.3863571,
            y: 281.1825088,
            z: 427.1718672
        }, {
            x: -272.8951618,
            y: 282.9089477,
            z: 448.4958312
        }, {
            x: -263.1575587,
            y: 287.7502113,
            z: 469.7033076
        }, {
            x: -256.0187797,
            y: 284.2301018,
            z: 492.8369086
        }],
        [{
            x: -256.0187797,
            y: 284.2301018,
            z: 492.8369086
        }, {
            x: -247.4740915,
            y: 280.0167429,
            z: 520.5264352
        }, {
            x: -242.6526471,
            y: 263.8243394,
            z: 550.9754528
        }, {
            x: -229.1934731,
            y: 255.3135035,
            z: 574.1391333
        }]
    ],
    [
        [{
            x: -503.7720785,
            y: 430.5201084,
            z: 10.4346144
        }, {
            x: -480.8938702,
            y: 405.3240418,
            z: 10.73584236
        }, {
            x: -454.1301131,
            y: 371.1599558,
            z: 12.4282249
        }, {
            x: -427.8161266,
            y: 353.2378994,
            z: 8.87717126
        }],
        [{
            x: -427.8161266,
            y: 353.2378994,
            z: 8.87717126
        }, {
            x: -414.070971,
            y: 343.8762828,
            z: 7.02227236
        }, {
            x: -400.4485359,
            y: 338.946325,
            z: 3.736695102
        }, {
            x: -384.9062354,
            y: 331.5267806,
            z: -0.8129614516
        }],
        [{
            x: -384.9062354,
            y: 331.5267806,
            z: -0.8129614516
        }, {
            x: -370.2436585,
            y: 324.5271964,
            z: -5.10509886
        }, {
            x: -353.8724013,
            y: 315.3118801,
            z: -10.52226684
        }, {
            x: -342.8231622,
            y: 302.5212162,
            z: -12.50855813
        }],
        [{
            x: -342.8231622,
            y: 302.5212162,
            z: -12.50855813
        }, {
            x: -331.773923,
            y: 289.7305524,
            z: -14.49484943
        }, {
            x: -326.046702,
            y: 273.364541,
            z: -13.05026403
        }, {
            x: -323.0565679,
            y: 255.1023769,
            z: -15.96492668
        }],
        [{
            x: -323.0565679,
            y: 255.1023769,
            z: -15.96492668
        }, {
            x: -319.3807262,
            y: 232.6522718,
            z: -19.5479896
        }, {
            x: -319.8412689,
            y: 207.3366325,
            z: -29.71890491
        }, {
            x: -315.7784556,
            y: 186.3698836,
            z: -39.64454531
        }],
        [{
            x: -315.7784556,
            y: 186.3698836,
            z: -39.64454531
        }, {
            x: -310.4195804,
            y: 158.714615,
            z: -52.73652502
        }, {
            x: -297.191075,
            y: 138.6254457,
            z: -65.4017811
        }, {
            x: -273.1094624,
            y: 129.8074593,
            z: -80.07758485
        }],
        [{
            x: -273.1094624,
            y: 129.8074593,
            z: -80.07758485
        }, {
            x: -255.68555,
            y: 123.427329,
            z: -90.69605648
        }, {
            x: -232.5799897,
            y: 122.9477112,
            z: -102.3670584
        }, {
            x: -222.4051511,
            y: 108.2136042,
            z: -110.7644361
        }],
        [{
            x: -222.4051511,
            y: 108.2136042,
            z: -110.7644361
        }, {
            x: -210.4188841,
            y: 90.85638155,
            z: -120.6568004
        }, {
            x: -216.3772923,
            y: 53.71741878,
            z: -126.006176
        }, {
            x: -194.8173292,
            y: 37.98002022,
            z: -145.3281613
        }]
    ],
    [
        [{
            x: -504.710286,
            y: 445.6176047,
            z: 46.22281905
        }, {
            x: -502.1665999,
            y: 439.2349181,
            z: 49.57860765
        }, {
            x: -499.6573504,
            y: 432.3491346,
            z: 52.19602956
        }, {
            x: -496.4652032,
            y: 425.3041566,
            z: 53.76851796
        }],
        [{
            x: -496.4652032,
            y: 425.3041566,
            z: 53.76851796
        }, {
            x: -485.7616318,
            y: 401.6816792,
            z: 59.04122028
        }, {
            x: -467.3800759,
            y: 376.2693395,
            z: 52.56548089
        }, {
            x: -450.0394799,
            y: 354.0789884,
            z: 47.77859835
        }],
        [{
            x: -450.0394799,
            y: 354.0789884,
            z: 47.77859835
        }, {
            x: -433.9082082,
            y: 333.4361812,
            z: 43.32555056
        }, {
            x: -418.6777673,
            y: 315.5816334,
            z: 40.3340135
        }, {
            x: -418.9993969,
            y: 290.581774,
            z: 34.5885728
        }],
        [{
            x: -418.9993969,
            y: 290.581774,
            z: 34.5885728
        }, {
            x: -419.1216349,
            y: 281.0803656,
            z: 32.40496937
        }, {
            x: -421.4902832,
            y: 270.5468567,
            z: 29.82357985
        }, {
            x: -425.4939095,
            y: 260.2170128,
            z: 27.38929236
        }],
        [{
            x: -425.4939095,
            y: 260.2170128,
            z: 27.38929236
        }, {
            x: -434.1664587,
            y: 237.8407786,
            z: 22.11620328
        }, {
            x: -450.5108279,
            y: 216.4202039,
            z: 17.53336216
        }, {
            x: -459.1616169,
            y: 198.0896845,
            z: 8.912831477
        }],
        [{
            x: -459.1616169,
            y: 198.0896845,
            z: 8.912831477
        }, {
            x: -476.36542,
            y: 161.635825,
            z: -8.230796956
        }, {
            x: -463.1417438,
            y: 137.4028814,
            z: -41.34315664
        }, {
            x: -462.9992593,
            y: 112.0667346,
            z: -71.61026758
        }],
        [{
            x: -462.9992593,
            y: 112.0667346,
            z: -71.61026758
        }, {
            x: -462.8720652,
            y: 89.44946994,
            z: -98.6293429
        }, {
            x: -473.1691575,
            y: 65.95307244,
            z: -123.3810646
        }, {
            x: -479.5090633,
            y: 45.61148055,
            z: -150.8906155
        }],
        [{
            x: -479.5090633,
            y: 45.61148055,
            z: -150.8906155
        }, {
            x: -485.8777589,
            y: 25.17751638,
            z: -178.5250888
        }, {
            x: -488.2532469,
            y: 7.927075176,
            z: -208.942495
        }, {
            x: -480.7275499,
            y: -10.5285841,
            z: -239.1853122
        }],
        [{
            x: -480.7275499,
            y: -10.5285841,
            z: -239.1853122
        }, {
            x: -472.0278306,
            y: -31.86335979,
            z: -274.1460639
        }, {
            x: -450.0967598,
            y: -54.80871685,
            z: -308.8735052
        }, {
            x: -436.8841315,
            y: -118.8381023,
            z: -322.1849738
        }]
    ],
    [
        [{
            x: -505.1601369,
            y: 439.0243541,
            z: 32.18110962
        }, {
            x: -469.6031288,
            y: 433.2049916,
            z: 20.97102111
        }, {
            x: -449.5675953,
            y: 432.6514095,
            z: -7.569127716
        }, {
            x: -427.8385702,
            y: 426.3453994,
            z: -36.60044394
        }],
        [{
            x: -427.8385702,
            y: 426.3453994,
            z: -36.60044394
        }, {
            x: -415.9657474,
            y: 422.8997713,
            z: -52.46326666
        }, {
            x: -403.5873204,
            y: 417.7367146,
            z: -68.47273093
        }, {
            x: -385.4388182,
            y: 406.2813962,
            z: -78.07998793
        }],
        [{
            x: -385.4388182,
            y: 406.2813962,
            z: -78.07998793
        }, {
            x: -370.0265871,
            y: 396.5532096,
            z: -86.23874746
        }, {
            x: -350.4530376,
            y: 382.2871095,
            z: -89.78030176
        }, {
            x: -334.6682406,
            y: 371.5869586,
            z: -97.6946773
        }],
        [{
            x: -334.6682406,
            y: 371.5869586,
            z: -97.6946773
        }, {
            x: -308.2114971,
            y: 353.6525406,
            z: -110.9598846
        }, {
            x: -292.3984136,
            y: 345.7358668,
            z: -136.5095639
        }, {
            x: -278.0902294,
            y: 345.9619383,
            z: -163.7749124
        }],
        [{
            x: -278.0902294,
            y: 345.9619383,
            z: -163.7749124
        }, {
            x: -268.8968003,
            y: 346.1071959,
            z: -181.2937006
        }, {
            x: -260.3246598,
            y: 349.6141374,
            z: -199.5207925
        }, {
            x: -250.9168904,
            y: 349.0598469,
            z: -216.2695246
        }],
        [{
            x: -250.9168904,
            y: 349.0598469,
            z: -216.2695246
        }, {
            x: -241.1199533,
            y: 348.4826273,
            z: -233.7110952
        }, {
            x: -230.416823,
            y: 343.5012273,
            z: -249.5494666
        }, {
            x: -220.9564848,
            y: 343.2670366,
            z: -267.0463404
        }],
        [{
            x: -220.9564848,
            y: 343.2670366,
            z: -267.0463404
        }, {
            x: -211.8719434,
            y: 343.0421487,
            z: -283.8481788
        }, {
            x: -203.9334191,
            y: 347.1948104,
            z: -302.179374
        }, {
            x: -195.4671433,
            y: 347.8457865,
            z: -319.6237674
        }],
        [{
            x: -195.4671433,
            y: 347.8457865,
            z: -319.6237674
        }, {
            x: -182.0036646,
            y: 348.8809999,
            z: -347.3646803
        }, {
            x: -167.2055608,
            y: 341.0608393,
            z: -372.8629701
        }, {
            x: -159.5892756,
            y: 333.2258661,
            z: -400.6902802
        }],
        [{
            x: -159.5892756,
            y: 333.2258661,
            z: -400.6902802
        }, {
            x: -155.4384888,
            y: 328.955897,
            z: -415.8558399
        }, {
            x: -153.4207906,
            y: 324.6815284,
            z: -431.7131474
        }, {
            x: -148.1586907,
            y: 317.3516413,
            z: -445.9041331
        }],
        [{
            x: -148.1586907,
            y: 317.3516413,
            z: -445.9041331
        }, {
            x: -139.2720166,
            y: 304.9728724,
            z: -469.8699773
        }, {
            x: -121.1320788,
            y: 283.8795478,
            z: -489.0833537
        }, {
            x: -117.9051624,
            y: 270.6894516,
            z: -514.2343625
        }],
        [{
            x: -117.9051624,
            y: 270.6894516,
            z: -514.2343625
        }, {
            x: -115.3892356,
            y: 260.4055418,
            z: -533.8438255
        }, {
            x: -121.9386932,
            y: 254.9258767,
            z: -557.0626792
        }, {
            x: -135.404275,
            y: 251.7586897,
            z: -573.5450621
        }]
    ],
    [
        [{
            x: -549.9227855,
            y: -602.8259581,
            z: 568.4516777
        }, {
            x: -541.6074114,
            y: -581.0963066,
            z: 543.6979499
        }, {
            x: -519.2414097,
            y: -561.6134845,
            z: 520.5329354
        }, {
            x: -492.1734634,
            y: -541.9041298,
            z: 510.8432443
        }],
        [{
            x: -492.1734634,
            y: -541.9041298,
            z: 510.8432443
        }, {
            x: -478.9678413,
            y: -532.2885393,
            z: 506.1159408
        }, {
            x: -464.6430787,
            y: -522.6190302,
            z: 504.595987
        }, {
            x: -449.658121,
            y: -515.8148714,
            z: 502.209755
        }],
        [{
            x: -449.658121,
            y: -515.8148714,
            z: 502.209755
        }, {
            x: -426.6323738,
            y: -505.359664,
            z: 498.5430929
        }, {
            x: -402.0478325,
            y: -501.669867,
            z: 492.8310517
        }, {
            x: -376.3838505,
            y: -502.7766533,
            z: 485.8181035
        }],
        [{
            x: -376.3838505,
            y: -502.7766533,
            z: 485.8181035
        }, {
            x: -348.3739159,
            y: -503.9846113,
            z: 478.1640995
        }, {
            x: -319.0781769,
            y: -510.9061466,
            z: 468.9604856
        }, {
            x: -291.57025,
            y: -519.0230285,
            z: 468.037493
        }],
        [{
            x: -291.57025,
            y: -519.0230285,
            z: 468.037493
        }, {
            x: -258.6514233,
            y: -528.7365279,
            z: 466.9329446
        }, {
            x: -228.2929202,
            y: -540.1618817,
            z: 477.6870637
        }, {
            x: -197.1934409,
            y: -551.5702378,
            z: 484.7955041
        }],
        [{
            x: -197.1934409,
            y: -551.5702378,
            z: 484.7955041
        }, {
            x: -167.0469079,
            y: -562.6290204,
            z: 491.6861286
        }, {
            x: -136.2041128,
            y: -573.671831,
            z: 495.1510722
        }, {
            x: -104.7492178,
            y: -581.9716898,
            z: 498.5899459
        }],
        [{
            x: -104.7492178,
            y: -581.9716898,
            z: 498.5899459
        }, {
            x: -66.21321732,
            y: -592.1400074,
            z: 502.8029767
        }, {
            x: -26.75850559,
            y: -598.1913809,
            z: 506.9768788
        }, {
            x: 11.56838577,
            y: -587.9849281,
            z: 515.3568901
        }]
    ],
    [
        [{
            x: -560.1377922,
            y: -566.2336,
            z: 525.511539
        }, {
            x: -548.8201853,
            y: -535.7592779,
            z: 507.2404028
        }, {
            x: -531.8659636,
            y: -506.2643268,
            z: 491.0276264
        }, {
            x: -512.9024853,
            y: -478.8172598,
            z: 474.9621421
        }],
        [{
            x: -512.9024853,
            y: -478.8172598,
            z: 474.9621421
        }, {
            x: -499.4840874,
            y: -459.3959457,
            z: 463.5943404
        }, {
            x: -485.0596831,
            y: -440.9999783,
            z: 452.3002857
        }, {
            x: -472.83469,
            y: -414.379096,
            z: 447.0092045
        }],
        [{
            x: -472.83469,
            y: -414.379096,
            z: 447.0092045
        }, {
            x: -461.2989921,
            y: -389.2592083,
            z: 442.0164561
        }, {
            x: -451.721674,
            y: -356.8157664,
            z: 442.368821
        }, {
            x: -440.3065566,
            y: -331.1423045,
            z: 437.6421797
        }],
        [{
            x: -440.3065566,
            y: -331.1423045,
            z: 437.6421797
        }, {
            x: -428.8914392,
            y: -305.4688425,
            z: 432.9155384
        }, {
            x: -415.6385225,
            y: -286.5653605,
            z: 423.1098909
        }, {
            x: -405.3535373,
            y: -271.885327,
            z: 407.3309734
        }],
        [{
            x: -405.3535373,
            y: -271.885327,
            z: 407.3309734
        }, {
            x: -394.3795905,
            y: -256.2219202,
            z: 390.4950714
        }, {
            x: -386.7845197,
            y: -245.3667472,
            z: 366.8588311
        }, {
            x: -381.2843706,
            y: -225.352747,
            z: 351.0743408
        }],
        [{
            x: -381.2843706,
            y: -225.352747,
            z: 351.0743408
        }, {
            x: -375.0393656,
            y: -202.6283574,
            z: 333.1522412
        }, {
            x: -371.4951111,
            y: -168.0965069,
            z: 325.3525327
        }, {
            x: -364.4120362,
            y: -172.2442304,
            z: 283.5623391
        }]
    ],
    [
        [{
            x: -557.0260841,
            y: -569.0104099,
            z: 548.4604679
        }, {
            x: -546.3591985,
            y: -562.8543008,
            z: 526.6889814
        }, {
            x: -526.7979297,
            y: -557.9022974,
            z: 507.2432248
        }, {
            x: -514.1321194,
            y: -545.0985813,
            z: 489.6451295
        }],
        [{
            x: -514.1321194,
            y: -545.0985813,
            z: 489.6451295
        }, {
            x: -501.7395637,
            y: -532.571095,
            z: 472.4266987
        }, {
            x: -495.9481481,
            y: -512.52703,
            z: 456.9770655
        }, {
            x: -483.7474762,
            y: -501.9684124,
            z: 440.4702199
        }],
        [{
            x: -483.7474762,
            y: -501.9684124,
            z: 440.4702199
        }, {
            x: -475.0382631,
            y: -494.431348,
            z: 428.6871282
        }, {
            x: -463.0631868,
            y: -491.7276326,
            z: 416.3653296
        }, {
            x: -448.1502659,
            y: -495.7810264,
            z: 401.7616844
        }],
        [{
            x: -448.1502659,
            y: -495.7810264,
            z: 401.7616844
        }, {
            x: -423.8737894,
            y: -502.3794735,
            z: 377.9886724
        }, {
            x: -391.8120245,
            y: -526.8842603,
            z: 348.1687669
        }, {
            x: -366.4085667,
            y: -534.4962062,
            z: 322.394162
        }],
        [{
            x: -366.4085667,
            y: -534.4962062,
            z: 322.394162
        }, {
            x: -337.5020289,
            y: -543.1578225,
            z: 293.0652968
        }, {
            x: -317.2167424,
            y: -529.9463972,
            z: 268.9743324
        }, {
            x: -286.0213466,
            y: -526.1624597,
            z: 266.5628322
        }],
        [{
            x: -286.0213466,
            y: -526.1624597,
            z: 266.5628322
        }, {
            x: -255.8367144,
            y: -522.5011258,
            z: 264.2294671
        }, {
            x: -215.4375173,
            y: -527.6662558,
            z: 282.193451
        }, {
            x: -180.3879868,
            y: -528.6882312,
            z: 288.8700781
        }],
        [{
            x: -180.3879868,
            y: -528.6882312,
            z: 288.8700781
        }, {
            x: -146.341562,
            y: -529.680958,
            z: 295.3556225
        }, {
            x: -117.3429745,
            y: -526.7642878,
            z: 291.1906455
        }, {
            x: -87.70964136,
            y: -526.3337374,
            z: 287.219477
        }],
        [{
            x: -87.70964136,
            y: -526.3337374,
            z: 287.219477
        }, {
            x: -50.32284231,
            y: -525.7905349,
            z: 282.2092652
        }, {
            x: -11.92568537,
            y: -529.20462,
            z: 277.5075487
        }, {
            x: 29.34180548,
            y: -567.174828,
            z: 250.2487439
        }]
    ],
    [
        [{
            x: -580.37822,
            y: -543.7030939,
            z: 501.0902597
        }, {
            x: -550.3942676,
            y: -518.9295084,
            z: 477.2897125
        }, {
            x: -528.916053,
            y: -499.838463,
            z: 437.8832747
        }, {
            x: -511.9752494,
            y: -480.7914578,
            z: 397.4363904
        }],
        [{
            x: -511.9752494,
            y: -480.7914578,
            z: 397.4363904
        }, {
            x: -505.2261812,
            y: -473.2032974,
            z: 381.3227052
        }, {
            x: -499.197271,
            y: -465.6221268,
            z: 365.0438849
        }, {
            x: -493.8764433,
            y: -452.8341656,
            z: 353.3114255
        }],
        [{
            x: -493.8764433,
            y: -452.8341656,
            z: 353.3114255
        }, {
            x: -488.9550622,
            y: -441.0062255,
            z: 342.4597483
        }, {
            x: -484.6394396,
            y: -424.723921,
            z: 335.4974433
        }, {
            x: -479.8941928,
            y: -405.5052856,
            z: 330.9886374
        }],
        [{
            x: -479.8941928,
            y: -405.5052856,
            z: 330.9886374
        }, {
            x: -472.7018992,
            y: -376.3759117,
            z: 324.1547134
        }, {
            x: -464.5226312,
            y: -340.5009125,
            z: 322.9572061
        }, {
            x: -464.8309975,
            y: -311.2892635,
            z: 315.2544084
        }],
        [{
            x: -464.8309975,
            y: -311.2892635,
            z: 315.2544084
        }, {
            x: -465.0721874,
            y: -288.4412587,
            z: 309.2296352
        }, {
            x: -470.5058124,
            y: -269.6696563,
            z: 299.225155
        }, {
            x: -472.9449076,
            y: -256.7905369,
            z: 283.9960679
        }],
        [{
            x: -472.9449076,
            y: -256.7905369,
            z: 283.9960679
        }, {
            x: -475.4419466,
            y: -243.6054576,
            z: 268.4051942
        }, {
            x: -474.8004879,
            y: -236.5961536,
            z: 247.3385307
        }, {
            x: -472.386628,
            y: -231.4909257,
            z: 224.8850611
        }],
        [{
            x: -472.386628,
            y: -231.4909257,
            z: 224.8850611
        }, {
            x: -470.2612941,
            y: -226.9959201,
            z: 205.11543
        }, {
            x: -466.7619426,
            y: -223.9770102,
            z: 184.2707062
        }, {
            x: -465.7689288,
            y: -217.0037188,
            z: 166.2791855
        }],
        [{
            x: -465.7689288,
            y: -217.0037188,
            z: 166.2791855
        }, {
            x: -464.4089205,
            y: -207.4532633,
            z: 141.6384226
        }, {
            x: -467.7501491,
            y: -190.4854177,
            z: 122.349526
        }, {
            x: -478.8366348,
            y: -176.5122848,
            z: 105.4221503
        }]
    ],
    [
        [{
            x: -588.3330167,
            y: -567.0754484,
            z: 514.6207423
        }, {
            x: -584.6926986,
            y: -559.7377537,
            z: 512.7537068
        }, {
            x: -580.0041859,
            y: -556.1986347,
            z: 508.0228802
        }, {
            x: -574.896214,
            y: -552.3095187,
            z: 504.3239974
        }],
        [{
            x: -574.896214,
            y: -552.3095187,
            z: 504.3239974
        }, {
            x: -538.7773986,
            y: -524.8093158,
            z: 478.1689464
        }, {
            x: -481.6856241,
            y: -479.8092604,
            z: 503.6110892
        }, {
            x: -429.29065,
            y: -455.8440568,
            z: 501.8311261
        }],
        [{
            x: -429.29065,
            y: -455.8440568,
            z: 501.8311261
        }, {
            x: -410.2624074,
            y: -447.1406321,
            z: 501.1846983
        }, {
            x: -391.8536347,
            y: -441.2115339,
            z: 496.9478954
        }, {
            x: -373.2425172,
            y: -433.0336362,
            z: 495.6307518
        }],
        [{
            x: -373.2425172,
            y: -433.0336362,
            z: 495.6307518
        }, {
            x: -353.8470398,
            y: -424.5110835,
            z: 494.2580977
        }, {
            x: -334.2318026,
            y: -413.5461872,
            z: 496.056385
        }, {
            x: -314.8424611,
            y: -398.2581398,
            z: 502.4137406
        }],
        [{
            x: -314.8424611,
            y: -398.2581398,
            z: 502.4137406
        }, {
            x: -295.4531197,
            y: -382.9700924,
            z: 508.7710962
        }, {
            x: -276.2896741,
            y: -363.3588939,
            z: 519.68752
        }, {
            x: -256.5851034,
            y: -345.5847604,
            z: 531.6495914
        }],
        [{
            x: -256.5851034,
            y: -345.5847604,
            z: 531.6495914
        }, {
            x: -239.1278584,
            y: -329.8377843,
            z: 542.2473769
        }, {
            x: -221.2458813,
            y: -315.5327309,
            z: 553.6658969
        }, {
            x: -202.7088945,
            y: -305.7958697,
            z: 562.4952152
        }],
        [{
            x: -202.7088945,
            y: -305.7958697,
            z: 562.4952152
        }, {
            x: -175.5952761,
            y: -291.5539898,
            z: 575.4096533
        }, {
            x: -147.0803146,
            y: -287.0854057,
            z: 582.7846933
        }, {
            x: -120.6934978,
            y: -276.7267314,
            z: 586.3126731
        }],
        [{
            x: -120.6934978,
            y: -276.7267314,
            z: 586.3126731
        }, {
            x: -91.38502639,
            y: -265.2211029,
            z: 590.2312851
        }, {
            x: -64.7020637,
            y: -246.4488238,
            z: 589.4037487
        }, {
            x: -36.83838731,
            y: -240.3643005,
            z: 578.8871459
        }],
        [{
            x: -36.83838731,
            y: -240.3643005,
            z: 578.8871459
        }, {
            x: -15.66541271,
            y: -235.7408094,
            z: 570.8958183
        }, {
            x: 6.189321891,
            y: -238.4433995,
            z: 557.3098932
        }, {
            x: 29.70555885,
            y: -248.6508732,
            z: 539.4838767
        }]
    ],
    [
        [{
            x: 649.743553,
            y: 212.3179198,
            z: 998.1955368
        }, {
            x: 654.9930671,
            y: 211.2448922,
            z: 983.4484784
        }, {
            x: 658.2438449,
            y: 207.8667642,
            z: 968.0743737
        }, {
            x: 659.6191942,
            y: 207.0208092,
            z: 954.1549166
        }],
        [{
            x: 659.6191942,
            y: 207.0208092,
            z: 954.1549166
        }, {
            x: 660.662305,
            y: 206.3792087,
            z: 943.5979352
        }, {
            x: 660.6266296,
            y: 207.1941678,
            z: 933.877698
        }, {
            x: 660.547424,
            y: 206.1235234,
            z: 921.9996824
        }],
        [{
            x: 660.547424,
            y: 206.1235234,
            z: 921.9996824
        }, {
            x: 660.4109412,
            y: 204.278647,
            z: 901.5321319
        }, {
            x: 660.1452073,
            y: 196.8349783,
            z: 874.6576393
        }, {
            x: 662.0421068,
            y: 182.0660679,
            z: 857.0338657
        }],
        [{
            x: 662.0421068,
            y: 182.0660679,
            z: 857.0338657
        }, {
            x: 662.6897815,
            y: 177.023392,
            z: 851.0164293
        }, {
            x: 663.5895766,
            y: 171.1267371,
            z: 846.0774446
        }, {
            x: 664.5379451,
            y: 164.8912447,
            z: 841.3504366
        }],
        [{
            x: 664.5379451,
            y: 164.8912447,
            z: 841.3504366
        }, {
            x: 666.7462841,
            y: 150.3714856,
            z: 830.3432839
        }, {
            x: 669.2179982,
            y: 134.0144743,
            z: 820.4855162
        }, {
            x: 668.9621152,
            y: 118.1275959,
            z: 811.5539607
        }],
        [{
            x: 668.9621152,
            y: 118.1275959,
            z: 811.5539607
        }, {
            x: 668.3124182,
            y: 77.79018517,
            z: 788.8763887
        }, {
            x: 650.0786628,
            y: 40.48359067,
            z: 772.169848
        }, {
            x: 629.3571948,
            y: 9.987837182,
            z: 752.014915
        }],
        [{
            x: 629.3571948,
            y: 9.987837182,
            z: 752.014915
        }, {
            x: 609.6415424,
            y: -19.027659,
            z: 732.8382981
        }, {
            x: 587.6738222,
            y: -41.87746059,
            z: 710.5399325
        }, {
            x: 561.549695,
            y: -63.92735044,
            z: 686.5645624
        }],
        [{
            x: 561.549695,
            y: -63.92735044,
            z: 686.5645624
        }, {
            x: 536.0099912,
            y: -85.48396182,
            z: 663.1255457
        }, {
            x: 506.4977663,
            y: -106.2760508,
            z: 638.0837177
        }, {
            x: 482.5807376,
            y: -122.5523062,
            z: 606.4581538
        }],
        [{
            x: 482.5807376,
            y: -122.5523062,
            z: 606.4581538
        }, {
            x: 458.0735048,
            y: -139.2302133,
            z: 574.0521611
        }, {
            x: 439.4410224,
            y: -151.1666609,
            z: 534.7334875
        }, {
            x: 420.0445902,
            y: -174.9389516,
            z: 509.8485651
        }],
        [{
            x: 420.0445902,
            y: -174.9389516,
            z: 509.8485651
        }, {
            x: 412.4686945,
            y: -184.2239784,
            z: 500.1289645
        }, {
            x: 404.776255,
            y: -195.3146132,
            z: 492.6112936
        }, {
            x: 397.3102672,
            y: -206.3305013,
            z: 484.7358733
        }],
        [{
            x: 397.3102672,
            y: -206.3305013,
            z: 484.7358733
        }, {
            x: 386.4532758,
            y: -222.3497347,
            z: 473.2834879
        }, {
            x: 376.0751568,
            y: -238.2109029,
            z: 461.0745772
        }, {
            x: 364.9094503,
            y: -254.2009308,
            z: 450.4114705
        }],
        [{
            x: 364.9094503,
            y: -254.2009308,
            z: 450.4114705
        }, {
            x: 358.148946,
            y: -263.8824188,
            z: 443.9552754
        }, {
            x: 351.0997165,
            y: -273.6111461,
            z: 438.0657634
        }, {
            x: 347.8659291,
            y: -286.2945603,
            z: 434.2823127
        }],
        [{
            x: 347.8659291,
            y: -286.2945603,
            z: 434.2823127
        }, {
            x: 342.8598739,
            y: -305.9290824,
            z: 428.4253529
        }, {
            x: 346.9973335,
            y: -332.6443631,
            z: 427.6154633
        }, {
            x: 346.185195,
            y: -347.5551623,
            z: 413.7670996
        }],
        [{
            x: 346.185195,
            y: -347.5551623,
            z: 413.7670996
        }, {
            x: 345.8941874,
            y: -352.8980382,
            z: 408.8049184
        }, {
            x: 344.9676759,
            y: -356.7252769,
            z: 402.1686615
        }, {
            x: 344.2406389,
            y: -360.3566871,
            z: 395.5162684
        }],
        [{
            x: 344.2406389,
            y: -360.3566871,
            z: 395.5162684
        }, {
            x: 342.8879712,
            y: -367.1130021,
            z: 383.1393512
        }, {
            x: 342.225792,
            y: -373.1914493,
            z: 370.706578
        }, {
            x: 341.2703853,
            y: -379.5670491,
            z: 358.2761681
        }]
    ],
    [
        [{
            x: 649.8190401,
            y: 214.6343176,
            z: 1010.991031
        }, {
            x: 639.9654235,
            y: 201.0401859,
            z: 975.9808369
        }, {
            x: 626.5892163,
            y: 186.3815633,
            z: 942.4943864
        }, {
            x: 613.2102749,
            y: 171.7381825,
            z: 909.0685578
        }],
        [{
            x: 613.2102749,
            y: 171.7381825,
            z: 909.0685578
        }, {
            x: 607.8158523,
            y: 165.8339348,
            z: 895.5911801
        }, {
            x: 602.4209852,
            y: 159.9321649,
            z: 882.1236579
        }, {
            x: 598.0099845,
            y: 154.3160206,
            z: 868.1873116
        }],
        [{
            x: 598.0099845,
            y: 154.3160206,
            z: 868.1873116
        }, {
            x: 593.768894,
            y: 148.9162082,
            z: 854.7877885
        }, {
            x: 590.4373333,
            y: 143.7804406,
            z: 840.9548636
        }, {
            x: 584.0069445,
            y: 138.1523009,
            z: 827.4402524
        }],
        [{
            x: 584.0069445,
            y: 138.1523009,
            z: 827.4402524
        }, {
            x: 572.3374822,
            y: 127.9387093,
            z: 802.9147917
        }, {
            x: 550.4627447,
            y: 116.1036034,
            z: 779.4376239
        }, {
            x: 529.7888033,
            y: 104.157051,
            z: 758.6958823
        }],
        [{
            x: 529.7888033,
            y: 104.157051,
            z: 758.6958823
        }, {
            x: 494.286733,
            y: 83.64198142,
            z: 723.0773829
        }, {
            x: 462.3256961,
            y: 62.79826636,
            z: 695.525396
        }, {
            x: 437.6504436,
            y: 37.51522704,
            z: 664.8824498
        }],
        [{
            x: 437.6504436,
            y: 37.51522704,
            z: 664.8824498
        }, {
            x: 407.4544996,
            y: 6.575513746,
            z: 627.3836365
        }, {
            x: 388.169198,
            y: -31.01219751,
            z: 585.2560353
        }, {
            x: 356.2809519,
            y: -61.87811273,
            z: 549.5578191
        }],
        [{
            x: 356.2809519,
            y: -61.87811273,
            z: 549.5578191
        }, {
            x: 331.8954411,
            y: -85.48182902,
            z: 522.258756
        }, {
            x: 300.1398116,
            y: -105.1546834,
            z: 498.7195549
        }, {
            x: 273.7716321,
            y: -110.7553815,
            z: 465.1121129
        }],
        [{
            x: 273.7716321,
            y: -110.7553815,
            z: 465.1121129
        }, {
            x: 245.755688,
            y: -116.7060707,
            z: 429.4045202
        }, {
            x: 223.8215628,
            y: -106.7708944,
            z: 382.3310276
        }, {
            x: 208.1511062,
            y: -124.2353231,
            z: 342.0223855
        }]
    ],
    [
        [{
            x: 632.5641586,
            y: 200.5764061,
            z: 1021.558579
        }, {
            x: 634.8921395,
            y: 200.7858731,
            z: 1015.748944
        }, {
            x: 636.7014539,
            y: 199.8597104,
            z: 1009.334372
        }, {
            x: 637.4916616,
            y: 197.7987526,
            z: 1003.64374
        }],
        [{
            x: 637.4916616,
            y: 197.7987526,
            z: 1003.64374
        }, {
            x: 640.3800961,
            y: 190.2653635,
            z: 982.8428546
        }, {
            x: 629.6521316,
            y: 167.5698497,
            z: 971.7145956
        }, {
            x: 620.0433141,
            y: 149.0145031,
            z: 957.156368
        }],
        [{
            x: 620.0433141,
            y: 149.0145031,
            z: 957.156368
        }, {
            x: 609.777732,
            y: 129.1908949,
            z: 941.6030827
        }, {
            x: 600.789513,
            y: 114.0927581,
            z: 922.1349269
        }, {
            x: 587.4641696,
            y: 96.2007205,
            z: 907.1320174
        }],
        [{
            x: 587.4641696,
            y: 96.2007205,
            z: 907.1320174
        }, {
            x: 579.8341002,
            y: 85.95576954,
            z: 898.5413764
        }, {
            x: 570.7820227,
            y: 74.79478548,
            z: 891.4147507
        }, {
            x: 562.8144994,
            y: 63.32371634,
            z: 885.7487667
        }],
        [{
            x: 562.8144994,
            y: 63.32371634,
            z: 885.7487667
        }, {
            x: 551.720202,
            y: 47.35094191,
            z: 877.8592245
        }, {
            x: 542.7287345,
            y: 30.77694696,
            z: 872.8017044
        }, {
            x: 533.3385674,
            y: 14.22313773,
            z: 868.8369432
        }],
        [{
            x: 533.3385674,
            y: 14.22313773,
            z: 868.8369432
        }, {
            x: 524.1661728,
            y: -1.946762823,
            z: 864.964131
        }, {
            x: 514.6133571,
            y: -18.09740307,
            z: 862.1339799
        }, {
            x: 509.0302724,
            y: -34.9103866,
            z: 859.9185356
        }],
        [{
            x: 509.0302724,
            y: -34.9103866,
            z: 859.9185356
        }, {
            x: 503.4892612,
            y: -51.59666913,
            z: 857.7197867
        }, {
            x: 501.8583756,
            y: -68.93534985,
            z: 856.1265149
        }, {
            x: 501.7144834,
            y: -86.18970874,
            z: 859.2901137
        }],
        [{
            x: 501.7144834,
            y: -86.18970874,
            z: 859.2901137
        }, {
            x: 501.4656352,
            y: -116.0295432,
            z: 864.7612682
        }, {
            x: 505.664167,
            y: -145.6171833,
            z: 884.4595267
        }, {
            x: 499.9249241,
            y: -173.571743,
            z: 901.897508
        }],
        [{
            x: 499.9249241,
            y: -173.571743,
            z: 901.897508
        }, {
            x: 493.6144433,
            y: -204.3086736,
            z: 921.0711251
        }, {
            x: 475.2894867,
            y: -233.0712581,
            z: 937.5121339
        }, {
            x: 451.4392712,
            y: -260.2525333,
            z: 951.2531709
        }],
        [{
            x: 451.4392712,
            y: -260.2525333,
            z: 951.2531709
        }, {
            x: 428.7960184,
            y: -286.0582744,
            z: 964.2988298
        }, {
            x: 401.1725787,
            y: -310.4387038,
            z: 974.9108716
        }, {
            x: 364.7374573,
            y: -329.124654,
            z: 980.7692883
        }],
        [{
            x: 364.7374573,
            y: -329.124654,
            z: 980.7692883
        }, {
            x: 324.9151135,
            y: -349.5477597,
            z: 987.1723378
        }, {
            x: 274.5665605,
            y: -363.1683866,
            z: 987.8968286
        }, {
            x: 252.5076793,
            y: -392.1866318,
            z: 995.7961109
        }]
    ],
    [
        [{
            x: 663.7304057,
            y: 219.4680244,
            z: 1002.273759
        }, {
            x: 666.8083692,
            y: 221.4853407,
            z: 937.6858859
        }, {
            x: 660.7128815,
            y: 219.1329505,
            z: 872.7563778
        }, {
            x: 655.1144593,
            y: 210.1395799,
            z: 808.9985228
        }],
        [{
            x: 655.1144593,
            y: 210.1395799,
            z: 808.9985228
        }, {
            x: 649.2082911,
            y: 200.6518424,
            z: 741.7358896
        }, {
            x: 643.8553379,
            y: 183.7729462,
            z: 675.7772618
        }, {
            x: 642.4072081,
            y: 176.7845855,
            z: 608.4484047
        }],
        [{
            x: 642.4072081,
            y: 176.7845855,
            z: 608.4484047
        }, {
            x: 641.1660629,
            y: 170.7950874,
            z: 550.7430172
        }, {
            x: 642.7932643,
            y: 172.0708306,
            z: 492.031107
        }, {
            x: 616.7655112,
            y: 145.1988786,
            z: 414.434995
        }],
        [{
            x: 616.7655112,
            y: 145.1988786,
            z: 414.434995
        }, {
            x: 588.8235429,
            y: 116.3506246,
            z: 331.1320647
        }, {
            x: 529.0092615,
            y: 55.06217426,
            z: 226.0651068
        }, {
            x: 514.0282355,
            y: 48.43770948,
            z: 165.443371
        }]
    ],
    [
        [{
            x: 650.1790286,
            y: 213.405679,
            z: 1011.280543
        }, {
            x: 647.0526726,
            y: 201.7398368,
            z: 956.6570369
        }, {
            x: 622.760157,
            y: 186.7391768,
            z: 903.9641442
        }, {
            x: 590.3186889,
            y: 172.0862986,
            z: 860.0330988
        }],
        [{
            x: 590.3186889,
            y: 172.0862986,
            z: 860.0330988
        }, {
            x: 572.0205979,
            y: 163.8215771,
            z: 835.2544955
        }, {
            x: 551.130045,
            y: 155.6674968,
            z: 813.2633368
        }, {
            x: 528.123337,
            y: 152.6395469,
            z: 790.1223455
        }],
        [{
            x: 528.123337,
            y: 152.6395469,
            z: 790.1223455
        }, {
            x: 506.5857205,
            y: 149.8049464,
            z: 768.4590202
        }, {
            x: 483.1935741,
            y: 151.4627205,
            z: 745.7880189
        }, {
            x: 462.7994047,
            y: 154.3830035,
            z: 722.632521
        }],
        [{
            x: 462.7994047,
            y: 154.3830035,
            z: 722.632521
        }, {
            x: 442.0143378,
            y: 157.3592598,
            z: 699.0331989
        }, {
            x: 424.3432746,
            y: 161.6468861,
            z: 674.9306295
        }, {
            x: 404.2923523,
            y: 160.2107179,
            z: 653.1009642
        }],
        [{
            x: 404.2923523,
            y: 160.2107179,
            z: 653.1009642
        }, {
            x: 381.5643354,
            y: 158.5828,
            z: 628.3567158
        }, {
            x: 355.7785429,
            y: 149.6006317,
            z: 606.5328213
        }, {
            x: 326.1629405,
            y: 141.3957684,
            z: 591.8434476
        }],
        [{
            x: 326.1629405,
            y: 141.3957684,
            z: 591.8434476
        }, {
            x: 309.7582274,
            y: 136.8509198,
            z: 583.7066905
        }, {
            x: 292.1784183,
            y: 132.5445708,
            z: 577.7590099
        }, {
            x: 271.5403344,
            y: 125.7101365,
            z: 572.6844976
        }],
        [{
            x: 271.5403344,
            y: 125.7101365,
            z: 572.6844976
        }, {
            x: 250.9968488,
            y: 118.907029,
            z: 567.6332452
        }, {
            x: 227.4230605,
            y: 109.5989589,
            z: 563.4471746
        }, {
            x: 208.422586,
            y: 100.1480243,
            z: 562.5706834
        }],
        [{
            x: 208.422586,
            y: 100.1480243,
            z: 562.5706834
        }, {
            x: 189.1896811,
            y: 90.58147759,
            z: 561.6834701
        }, {
            x: 174.6426639,
            y: 80.86854972,
            z: 564.1873026
        }, {
            x: 159.9644068,
            y: 69.27275283,
            z: 563.8797397
        }],
        [{
            x: 159.9644068,
            y: 69.27275283,
            z: 563.8797397
        }, {
            x: 140.5927676,
            y: 53.96919251,
            z: 563.4738334
        }, {
            x: 120.9925424,
            y: 35.38616063,
            z: 558.1712023
        }, {
            x: 86.33421218,
            y: 24.93127624,
            z: 557.0696285
        }],
        [{
            x: 86.33421218,
            y: 24.93127624,
            z: 557.0696285
        }, {
            x: 68.13076534,
            y: 19.44010193,
            z: 556.4910536
        }, {
            x: 45.77335851,
            y: 16.19117522,
            z: 557.071391
        }, {
            x: 26.80066872,
            y: 15.81542641,
            z: 555.2421132
        }],
        [{
            x: 26.80066872,
            y: 15.81542641,
            z: 555.2421132
        }, {
            x: -0.8565416642,
            y: 15.26768312,
            z: 552.5755054
        }, {
            x: -21.32122835,
            y: 20.8254429,
            z: 544.7884651
        }, {
            x: -40.32594368,
            y: 22.97305666,
            z: 548.8029482
        }],
        [{
            x: -40.32594368,
            y: 22.97305666,
            z: 548.8029482
        }, {
            x: -54.97459342,
            y: 24.62841646,
            z: 551.8972727
        }, {
            x: -68.75584842,
            y: 24.25774833,
            z: 562.0030905
        }, {
            x: -76.53252108,
            y: 25.44717994,
            z: 574.5788466
        }]
    ],
    [
        [{
            x: -457.0403192,
            y: 350.4984423,
            z: 932.3275974
        }, {
            x: -431.0914832,
            y: 313.2564843,
            z: 903.0996774
        }, {
            x: -392.7263555,
            y: 283.4841157,
            z: 846.9558105
        }, {
            x: -363.3006146,
            y: 249.0306781,
            z: 804.623502
        }],
        [{
            x: -363.3006146,
            y: 249.0306781,
            z: 804.623502
        }, {
            x: -341.2313088,
            y: 223.1905998,
            z: 772.8742707
        }, {
            x: -324.1904081,
            y: 194.7174203,
            z: 748.8940409
        }, {
            x: -306.4591557,
            y: 162.9754724,
            z: 731.7083556
        }],
        [{
            x: -306.4591557,
            y: 162.9754724,
            z: 731.7083556
        }, {
            x: -288.323336,
            y: 130.5092804,
            z: 714.130551
        }, {
            x: -269.4653023,
            y: 94.62345378,
            z: 703.6608852
        }, {
            x: -254.1637331,
            y: 63.85796556,
            z: 688.8370662
        }],
        [{
            x: -254.1637331,
            y: 63.85796556,
            z: 688.8370662
        }, {
            x: -238.5038227,
            y: 32.37199283,
            z: 673.666094
        }, {
            x: -226.5689019,
            y: 6.248988914,
            z: 653.9346444
        }, {
            x: -198.7651199,
            y: -15.88691008,
            z: 625.6980618
        }],
        [{
            x: -198.7651199,
            y: -15.88691008,
            z: 625.6980618
        }, {
            x: -181.5758313,
            y: -29.57211144,
            z: 608.2412004
        }, {
            x: -158.3212213,
            y: -41.73338034,
            z: 587.5335472
        }, {
            x: -137.9228187,
            y: -53.39124612,
            z: 575.1177753
        }],
        [{
            x: -137.9228187,
            y: -53.39124612,
            z: 575.1177753
        }, {
            x: -107.3877227,
            y: -70.84232095,
            z: 556.5321637
        }, {
            x: -83.25286265,
            y: -87.16536184,
            z: 556.5271349
        }, {
            x: -60.95988086,
            y: -112.0385313,
            z: 556.7707108
        }],
        [{
            x: -60.95988086,
            y: -112.0385313,
            z: 556.7707108
        }, {
            x: -40.59814834,
            y: -134.7569288,
            z: 556.9931857
        }, {
            x: -21.77299182,
            y: -164.6082205,
            z: 557.4230575
        }, {
            x: 5.351713362,
            y: -188.8478148,
            z: 541.8108971
        }],
        [{
            x: 5.351713362,
            y: -188.8478148,
            z: 541.8108971
        }, {
            x: 21.87729478,
            y: -203.6156578,
            z: 532.2992719
        }, {
            x: 41.4834947,
            y: -216.3005564,
            z: 516.8331806
        }, {
            x: 55.54016222,
            y: -229.5341436,
            z: 509.271554
        }]
    ],
    [
        [{
            x: -385.1563086,
            y: 347.5875636,
            z: 951.5832234
        }, {
            x: -407.1287961,
            y: 348.786922,
            z: 943.5042277
        }, {
            x: -422.9868028,
            y: 345.780842,
            z: 923.4610711
        }, {
            x: -431.6605347,
            y: 339.9021131,
            z: 896.9539333
        }],
        [{
            x: -431.6605347,
            y: 339.9021131,
            z: 896.9539333
        }, {
            x: -441.6613581,
            y: 333.1239315,
            z: 866.3911716
        }, {
            x: -442.1113221,
            y: 322.5268167,
            z: 827.2351172
        }, {
            x: -432.380964,
            y: 303.9141808,
            z: 794.876607
        }],
        [{
            x: -432.380964,
            y: 303.9141808,
            z: 794.876607
        }, {
            x: -423.6370467,
            y: 287.1884503,
            z: 765.7985265
        }, {
            x: -406.6722942,
            y: 263.9900093,
            z: 742.2096134
        }, {
            x: -389.7712205,
            y: 252.7277951,
            z: 708.6049691
        }],
        [{
            x: -389.7712205,
            y: 252.7277951,
            z: 708.6049691
        }, {
            x: -373.4592164,
            y: 241.8581139,
            z: 676.1715803
        }, {
            x: -357.2065297,
            y: 242.1071099,
            z: 634.4084693
        }, {
            x: -337.9943979,
            y: 240.8375501,
            z: 591.6002756
        }],
        [{
            x: -337.9943979,
            y: 240.8375501,
            z: 591.6002756
        }, {
            x: -317.9525871,
            y: 239.5131641,
            z: 546.9434028
        }, {
            x: -294.6902039,
            y: 236.5362322,
            z: 501.1492343
        }, {
            x: -278.1596929,
            y: 224.4055775,
            z: 468.432283
        }],
        [{
            x: -278.1596929,
            y: 224.4055775,
            z: 468.432283
        }, {
            x: -256.6580035,
            y: 208.6269019,
            z: 425.8764464
        }, {
            x: -246.5459142,
            y: 177.3611169,
            z: 405.4458477
        }, {
            x: -220.554898,
            y: 162.142157,
            z: 364.3437106
        }]
    ],
    [
        [{
            x: -458.0784757,
            y: 353.635112,
            z: 934.0481107
        }, {
            x: -459.338886,
            y: 340.3545522,
            z: 892.3409403
        }, {
            x: -453.2984712,
            y: 324.740729,
            z: 847.6076106
        }, {
            x: -447.0833806,
            y: 295.3513996,
            z: 805.0063179
        }],
        [{
            x: -447.0833806,
            y: 295.3513996,
            z: 805.0063179
        }, {
            x: -443.7376652,
            y: 279.5304984,
            z: 782.0731359
        }, {
            x: -440.3413305,
            y: 259.7175963,
            z: 759.7577965
        }, {
            x: -437.0431615,
            y: 245.558405,
            z: 736.7353327
        }],
        [{
            x: -437.0431615,
            y: 245.558405,
            z: 736.7353327
        }, {
            x: -433.6192233,
            y: 230.8592807,
            z: 712.8349523
        }, {
            x: -430.3010802,
            y: 222.2532749,
            z: 688.1724897
        }, {
            x: -427.5955912,
            y: 219.9086175,
            z: 661.7973574
        }],
        [{
            x: -427.5955912,
            y: 219.9086175,
            z: 661.7973574
        }, {
            x: -423.4813794,
            y: 216.3431191,
            z: 621.6889407
        }, {
            x: -420.7839302,
            y: 227.2569889,
            z: 577.6199754
        }, {
            x: -427.7760686,
            y: 229.6532968,
            z: 543.4979273
        }],
        [{
            x: -427.7760686,
            y: 229.6532968,
            z: 543.4979273
        }, {
            x: -434.0559775,
            y: 231.8055133,
            z: 512.8516014
        }, {
            x: -448.1520194,
            y: 227.0870145,
            z: 490.2289843
        }, {
            x: -461.1483233,
            y: 221.8629994,
            z: 466.6750956
        }],
        [{
            x: -461.1483233,
            y: 221.8629994,
            z: 466.6750956
        }, {
            x: -479.5278724,
            y: 214.4751265,
            z: 433.3648669
        }, {
            x: -495.7079454,
            y: 206.0762213,
            z: 398.1920953
        }, {
            x: -518.4854736,
            y: 196.958341,
            z: 371.2927569
        }],
        [{
            x: -518.4854736,
            y: 196.958341,
            z: 371.2927569
        }, {
            x: -533.7607278,
            y: 190.8436328,
            z: 353.2533004
        }, {
            x: -552.0031364,
            y: 184.4055712,
            z: 338.9347565
        }, {
            x: -563.4952099,
            y: 181.08143,
            z: 314.2266485
        }],
        [{
            x: -563.4952099,
            y: 181.08143,
            z: 314.2266485
        }, {
            x: -577.2505293,
            y: 177.1026333,
            z: 284.6525319
        }, {
            x: -581.3348813,
            y: 177.5850406,
            z: 240.1936529
        }, {
            x: -587.4945962,
            y: 167.9088277,
            z: 206.8086936
        }]
    ],
    [
        [{
            x: -402.4851157,
            y: 349.9511742,
            z: 958.6269155
        }, {
            x: -376.301045,
            y: 327.1111496,
            z: 918.5186098
        }, {
            x: -341.9554264,
            y: 300.6967193,
            z: 864.9611238
        }, {
            x: -320.1066749,
            y: 273.584113,
            z: 835.0395751
        }],
        [{
            x: -320.1066749,
            y: 273.584113,
            z: 835.0395751
        }, {
            x: -308.6939424,
            y: 259.421797,
            z: 819.4100033
        }, {
            x: -300.6909962,
            y: 245.0689828,
            z: 810.2295276
        }, {
            x: -289.2604062,
            y: 228.5889312,
            z: 798.2070058
        }],
        [{
            x: -289.2604062,
            y: 228.5889312,
            z: 798.2070058
        }, {
            x: -278.476809,
            y: 213.0416816,
            z: 786.8649815
        }, {
            x: -264.6426088,
            y: 195.6011907,
            z: 772.9935361
        }, {
            x: -249.9609406,
            y: 184.2222619,
            z: 754.486796
        }],
        [{
            x: -249.9609406,
            y: 184.2222619,
            z: 754.486796
        }, {
            x: -235.2792723,
            y: 172.843333,
            z: 735.9800558
        }, {
            x: -219.7501361,
            y: 167.5259663,
            z: 712.838021
        }, {
            x: -199.6372603,
            y: 164.2967694,
            z: 691.868555
        }],
        [{
            x: -199.6372603,
            y: 164.2967694,
            z: 691.868555
        }, {
            x: -174.9120321,
            y: 160.3270421,
            z: 666.0903005
        }, {
            x: -143.2596918,
            y: 159.5130323,
            z: 643.5953102
        }, {
            x: -115.7278356,
            y: 154.0568103,
            z: 624.2239866
        }],
        [{
            x: -115.7278356,
            y: 154.0568103,
            z: 624.2239866
        }, {
            x: -79.41315088,
            y: 146.8600205,
            z: 598.6730938
        }, {
            x: -50.26718939,
            y: 131.586817,
            z: 578.5566843
        }, {
            x: -30.09377427,
            y: 104.7239395,
            z: 568.9018964
        }],
        [{
            x: -30.09377427,
            y: 104.7239395,
            z: 568.9018964
        }, {
            x: -15.49758317,
            y: 85.28768179,
            z: 561.9163103
        }, {
            x: -5.598558441,
            y: 59.78418025,
            z: 560.4074278
        }, {
            x: 15.2156288,
            y: 48.25066329,
            z: 544.7803887
        }],
        [{
            x: 15.2156288,
            y: 48.25066329,
            z: 544.7803887
        }, {
            x: 39.73536951,
            y: 34.66383225,
            z: 526.371265
        }, {
            x: 79.40268268,
            y: 40.4639186,
            z: 488.3695975
        }, {
            x: 109.3053989,
            y: 15.50624823,
            z: 474.9867395
        }]
    ],
    [
        [{
            x: -441.3059116,
            y: 356.6731165,
            z: 950.1042845
        }, {
            x: -437.0934402,
            y: 354.7531151,
            z: 938.8390127
        }, {
            x: -431.8954444,
            y: 352.7524904,
            z: 927.5423934
        }, {
            x: -425.7946115,
            y: 349.8827825,
            z: 916.5559057
        }],
        [{
            x: -425.7946115,
            y: 349.8827825,
            z: 916.5559057
        }, {
            x: -405.337942,
            y: 340.2603806,
            z: 879.7171735
        }, {
            x: -374.730456,
            y: 320.8666638,
            z: 846.3653269
        }, {
            x: -348.4924997,
            y: 302.7635323,
            z: 816.0425327
        }],
        [{
            x: -348.4924997,
            y: 302.7635323,
            z: 816.0425327
        }, {
            x: -324.0843651,
            y: 285.922904,
            z: 787.8344347
        }, {
            x: -303.4575543,
            y: 270.1991283,
            z: 762.247632
        }, {
            x: -275.0626988,
            y: 269.9827321,
            z: 736.730279
        }],
        [{
            x: -275.0626988,
            y: 269.9827321,
            z: 736.730279
        }, {
            x: -264.2709933,
            y: 269.9004889,
            z: 727.0321927
        }, {
            x: -252.3572369,
            y: 272.0582005,
            z: 717.3441381
        }, {
            x: -240.8573166,
            y: 275.9035132,
            z: 708.4791444
        }],
        [{
            x: -240.8573166,
            y: 275.9035132,
            z: 708.4791444
        }, {
            x: -215.9464938,
            y: 284.2331279,
            z: 689.2760301
        }, {
            x: -192.9775174,
            y: 300.4814866,
            z: 673.9349714
        }, {
            x: -169.8061873,
            y: 308.1574079,
            z: 662.1047193
        }],
        [{
            x: -169.8061873,
            y: 308.1574079,
            z: 662.1047193
        }, {
            x: -123.7254194,
            y: 323.422494,
            z: 638.5779258
        }, {
            x: -76.84435968,
            y: 304.7842931,
            z: 628.9360847
        }, {
            x: -31.70232394,
            y: 299.9751543,
            z: 621.3408858
        }],
        [{
            x: -31.70232394,
            y: 299.9751543,
            z: 621.3408858
        }, {
            x: 8.595412622,
            y: 295.6820956,
            z: 614.5607458
        }, {
            x: 47.50733628,
            y: 302.4092947,
            z: 609.411557
        }, {
            x: 85.43930018,
            y: 304.5862415,
            z: 608.4606242
        }],
        [{
            x: 85.43930018,
            y: 304.5862415,
            z: 608.4606242
        }, {
            x: 123.5435152,
            y: 306.773074,
            z: 607.5053732
        }, {
            x: 160.6588501,
            y: 304.3682345,
            z: 610.7865939
        }, {
            x: 199.5407975,
            y: 291.9222978,
            z: 608.3125294
        }],
        [{
            x: 199.5407975,
            y: 291.9222978,
            z: 608.3125294
        }, {
            x: 244.4883993,
            y: 277.5347724,
            z: 605.4525064
        }, {
            x: 291.7967963,
            y: 249.7289251,
            z: 594.9014644
        }, {
            x: 364.5735542,
            y: 235.1056085,
            z: 522.5642344
        }]
    ],
    [
        [{
            x: -425.4736575,
            y: 354.8806676,
            z: 952.9899025
        }, {
            x: -409.7545596,
            y: 316.8549857,
            z: 939.4732246
        }, {
            x: -388.7284569,
            y: 291.6435713,
            z: 951.9123168
        }, {
            x: -361.5991999,
            y: 264.72079,
            z: 957.1066173
        }],
        [{
            x: -361.5991999,
            y: 264.72079,
            z: 957.1066173
        }, {
            x: -346.7756696,
            y: 250.0100788,
            z: 959.944803
        }, {
            x: -330.1299989,
            y: 234.7884269,
            z: 960.6200043
        }, {
            x: -311.1686211,
            y: 214.8695743,
            z: 946.4761562
        }],
        [{
            x: -311.1686211,
            y: 214.8695743,
            z: 946.4761562
        }, {
            x: -295.0660725,
            y: 197.9539105,
            z: 934.4647927
        }, {
            x: -277.2934601,
            y: 177.6506746,
            z: 911.7660851
        }, {
            x: -260.3723218,
            y: 160.4128455,
            z: 898.2468662
        }],
        [{
            x: -260.3723218,
            y: 160.4128455,
            z: 898.2468662
        }, {
            x: -232.0109677,
            y: 131.520689,
            z: 875.587436
        }, {
            x: -206.0416414,
            y: 111.2401135,
            z: 878.715745
        }, {
            x: -187.0377584,
            y: 92.0670441,
            z: 893.5438094
        }],
        [{
            x: -187.0377584,
            y: 92.0670441,
            z: 893.5438094
        }, {
            x: -174.8272047,
            y: 79.74778319,
            z: 903.0712768
        }, {
            x: -165.4922928,
            y: 67.88574955,
            z: 917.4289185
        }, {
            x: -153.093358,
            y: 55.48941348,
            z: 925.4450655
        }],
        [{
            x: -153.093358,
            y: 55.48941348,
            z: 925.4450655
        }, {
            x: -140.1815211,
            y: 42.58028283,
            z: 933.7928135
        }, {
            x: -123.9469216,
            y: 29.09173088,
            z: 935.2635628
        }, {
            x: -111.3591268,
            y: 16.51054645,
            z: 944.2066352
        }],
        [{
            x: -111.3591268,
            y: 16.51054645,
            z: 944.2066352
        }, {
            x: -99.27136205,
            y: 4.42912945,
            z: 952.7944584
        }, {
            x: -90.54642945,
            y: -6.815575695,
            z: 968.2727435
        }, {
            x: -78.93406953,
            y: -18.38978422,
            z: 978.6575366
        }],
        [{
            x: -78.93406953,
            y: -18.38978422,
            z: 978.6575366
        }, {
            x: -60.46753671,
            y: -36.79564686,
            z: 995.1719336
        }, {
            x: -34.69902103,
            y: -56.03478693,
            z: 998.8054553
        }, {
            x: -7.866381191,
            y: -68.34612144,
            z: 1007.210483
        }],
        [{
            x: -7.866381191,
            y: -68.34612144,
            z: 1007.210483
        }, {
            x: 6.757094934,
            y: -75.05565533,
            z: 1011.791125
        }, {
            x: 21.69662902,
            y: -79.70754573,
            z: 1017.788963
        }, {
            x: 38.75572063,
            y: -87.33401985,
            z: 1017.493596
        }],
        [{
            x: 38.75572063,
            y: -87.33401985,
            z: 1017.493596
        }, {
            x: 67.56524366,
            y: -100.2136671,
            z: 1016.994778
        }, {
            x: 102.4198952,
            y: -121.5770382,
            z: 998.5472927
        }, {
            x: 132.4197432,
            y: -128.874199,
            z: 1000.358242
        }],
        [{
            x: 132.4197432,
            y: -128.874199,
            z: 1000.358242
        }, {
            x: 155.809696,
            y: -134.5635694,
            z: 1001.770183
        }, {
            x: 176.2484922,
            y: -131.7023182,
            z: 1015.496898
        }, {
            x: 189.4250737,
            y: -120.6982278,
            z: 1029.827856
        }]
    ],
    [
        [{
            x: 684.0874638,
            y: -548.3225758,
            z: 278.8122689
        }, {
            x: 659.1491071,
            y: -542.312975,
            z: 279.810686
        }, {
            x: 634.7397299,
            y: -529.9321737,
            z: 272.0804584
        }, {
            x: 618.6178456,
            y: -510.6489393,
            z: 264.2012214
        }],
        [{
            x: 618.6178456,
            y: -510.6489393,
            z: 264.2012214
        }, {
            x: 610.752472,
            y: -501.2412398,
            z: 260.3571829
        }, {
            x: 604.8596586,
            y: -490.1906471,
            z: 256.4776777
        }, {
            x: 599.38628,
            y: -480.4465885,
            z: 250.7319072
        }],
        [{
            x: 599.38628,
            y: -480.4465885,
            z: 250.7319072
        }, {
            x: 590.9759371,
            y: -465.4739582,
            z: 241.9030095
        }, {
            x: 583.5559262,
            y: -453.5862002,
            z: 228.667652
        }, {
            x: 576.9381293,
            y: -443.8484771,
            z: 212.3494517
        }],
        [{
            x: 576.9381293,
            y: -443.8484771,
            z: 212.3494517
        }, {
            x: 569.7153975,
            y: -433.2206257,
            z: 194.5395997
        }, {
            x: 563.4482442,
            y: -425.1538453,
            z: 173.0575378
        }, {
            x: 563.2059746,
            y: -415.7654023,
            z: 153.5123383
        }],
        [{
            x: 563.2059746,
            y: -415.7654023,
            z: 153.5123383
        }, {
            x: 562.9160498,
            y: -404.5302216,
            z: 130.1225338
        }, {
            x: 571.2543535,
            y: -391.4022894,
            z: 109.5065075
        }, {
            x: 577.1308963,
            y: -379.1173759,
            z: 87.82625193
        }],
        [{
            x: 577.1308963,
            y: -379.1173759,
            z: 87.82625193
        }, {
            x: 582.8273708,
            y: -367.2088952,
            z: 66.81032006
        }, {
            x: 586.2106388,
            y: -356.0925613,
            z: 44.7943796
        }, {
            x: 588.5147464,
            y: -343.4000282,
            z: 23.70678191
        }],
        [{
            x: 588.5147464,
            y: -343.4000282,
            z: 23.70678191
        }, {
            x: 591.3375532,
            y: -327.8501602,
            z: -2.128042138
        }, {
            x: 592.5406293,
            y: -309.9345472,
            z: -26.56950033
        }, {
            x: 590.6515537,
            y: -283.5573725,
            z: -42.0327494
        }]
    ],
    [
        [{
            x: 643.5678197,
            y: -550.503582,
            z: 293.9966112
        }, {
            x: 619.3850813,
            y: -536.7660258,
            z: 298.5334802
        }, {
            x: 596.3728396,
            y: -520.0356972,
            z: 299.7865146
        }, {
            x: 574.0159732,
            y: -503.2294717,
            z: 298.9713603
        }],
        [{
            x: 574.0159732,
            y: -503.2294717,
            z: 298.9713603
        }, {
            x: 558.1964424,
            y: -491.3375277,
            z: 298.3945639
        }, {
            x: 542.7050487,
            y: -479.4075832,
            z: 296.7822547
        }, {
            x: 528.3391767,
            y: -462.705132,
            z: 301.328536
        }],
        [{
            x: 528.3391767,
            y: -462.705132,
            z: 301.328536
        }, {
            x: 514.7833114,
            y: -446.9444335,
            z: 305.6184793
        }, {
            x: 502.2296229,
            y: -426.9342428,
            z: 315.3920991
        }, {
            x: 488.6566696,
            y: -410.884812,
            z: 320.0563125
        }],
        [{
            x: 488.6566696,
            y: -410.884812,
            z: 320.0563125
        }, {
            x: 475.0837163,
            y: -394.8353811,
            z: 324.7205259
        }, {
            x: 460.4914982,
            y: -382.7467103,
            z: 324.2753328
        }, {
            x: 443.8186796,
            y: -376.082156,
            z: 322.4731277
        }],
        [{
            x: 443.8186796,
            y: -376.082156,
            z: 322.4731277
        }, {
            x: 426.0289966,
            y: -368.9711623,
            z: 320.5501981
        }, {
            x: 405.8706299,
            y: -368.0353166,
            z: 317.0823626
        }, {
            x: 387.695614,
            y: -361.2224199,
            z: 320.4804032
        }],
        [{
            x: 387.695614,
            y: -361.2224199,
            z: 320.4804032
        }, {
            x: 367.0592525,
            y: -353.4868889,
            z: 324.3386223
        }, {
            x: 348.9798056,
            y: -338.1747274,
            z: 337.0482547
        }, {
            x: 322.4841788,
            y: -350.564038,
            z: 323.6222798
        }]
    ],
    [
        [{
            x: 659.3037872,
            y: -542.6318039,
            z: 295.0421965
        }, {
            x: 641.7941209,
            y: -541.7681057,
            z: 287.9739609
        }, {
            x: 625.4031002,
            y: -536.3293376,
            z: 275.7182328
        }, {
            x: 608.0010114,
            y: -529.9813496,
            z: 271.3655125
        }],
        [{
            x: 608.0010114,
            y: -529.9813496,
            z: 271.3655125
        }, {
            x: 590.9743585,
            y: -523.7703142,
            z: 267.1066986
        }, {
            x: 572.9797929,
            y: -516.6888672,
            z: 270.4135689
        }, {
            x: 557.1575997,
            y: -511.2569585,
            z: 265.4720629
        }],
        [{
            x: 557.1575997,
            y: -511.2569585,
            z: 265.4720629
        }, {
            x: 545.8632338,
            y: -507.3794957,
            z: 261.9446646
        }, {
            x: 535.6758094,
            y: -504.3425621,
            z: 254.214272
        }, {
            x: 526.1901433,
            y: -503.7887414,
            z: 241.2572995
        }],
        [{
            x: 526.1901433,
            y: -503.7887414,
            z: 241.2572995
        }, {
            x: 510.7485976,
            y: -502.8871866,
            z: 220.1648765
        }, {
            x: 497.1667117,
            y: -508.5658813,
            z: 185.2220166
        }, {
            x: 480.6796639,
            y: -508.2528141,
            z: 162.6597934
        }],
        [{
            x: 480.6796639,
            y: -508.2528141,
            z: 162.6597934
        }, {
            x: 461.9190892,
            y: -507.8965757,
            z: 136.98629
        }, {
            x: 439.3968779,
            y: -499.782136,
            z: 127.343375
        }, {
            x: 433.4747104,
            y: -483.5070166,
            z: 110.9950964
        }],
        [{
            x: 433.4747104,
            y: -483.5070166,
            z: 110.9950964
        }, {
            x: 427.7444273,
            y: -467.7592281,
            z: 95.17651927
        }, {
            x: 437.5558973,
            y: -444.3710218,
            z: 73.08006073
        }, {
            x: 438.973342,
            y: -425.4277825,
            z: 53.93021538
        }],
        [{
            x: 438.973342,
            y: -425.4277825,
            z: 53.93021538
        }, {
            x: 440.3502199,
            y: -407.0266922,
            z: 35.32843207
        }, {
            x: 433.8066654,
            y: -392.8197829,
            z: 19.50701336
        }, {
            x: 428.239725,
            y: -379.4011324,
            z: 2.200349923
        }],
        [{
            x: 428.239725,
            y: -379.4011324,
            z: 2.200349923
        }, {
            x: 421.2162127,
            y: -362.4715353,
            z: -19.63454613
        }, {
            x: 415.7472285,
            y: -346.7966512,
            z: -43.83358429
        }, {
            x: 407.8966138,
            y: -353.4111832,
            z: -89.81990853
        }]
    ],
    [
        [{
            x: 621.2627433,
            y: -558.0194787,
            z: 311.6146248
        }, {
            x: 593.7782434,
            y: -539.6348383,
            z: 301.9214642
        }, {
            x: 558.970266,
            y: -533.283906,
            z: 291.6063319
        }, {
            x: 523.9281163,
            y: -529.5265637,
            z: 283.6564923
        }],
        [{
            x: 523.9281163,
            y: -529.5265637,
            z: 283.6564923
        }, {
            x: 509.9676301,
            y: -528.0296714,
            z: 280.4893457
        }, {
            x: 495.9699772,
            y: -526.9444222,
            z: 277.6976079
        }, {
            x: 483.1081214,
            y: -522.2536072,
            z: 278.5501892
        }],
        [{
            x: 483.1081214,
            y: -522.2536072,
            z: 278.5501892
        }, {
            x: 471.2118342,
            y: -517.9149423,
            z: 279.3387653
        }, {
            x: 460.2872116,
            y: -510.4917469,
            z: 283.2450249
        }, {
            x: 449.8522537,
            y: -500.6652628,
            z: 288.7168516
        }],
        [{
            x: 449.8522537,
            y: -500.6652628,
            z: 288.7168516
        }, {
            x: 434.0361575,
            y: -485.7714203,
            z: 297.0104105
        }, {
            x: 419.3449668,
            y: -465.356509,
            z: 308.9005428
        }, {
            x: 403.6418215,
            y: -454.3939022,
            z: 321.3287015
        }],
        [{
            x: 403.6418215,
            y: -454.3939022,
            z: 321.3287015
        }, {
            x: 391.3595454,
            y: -445.8194566,
            z: 331.0494342
        }, {
            x: 378.4581912,
            y: -443.0275972,
            z: 341.0993126
        }, {
            x: 364.011377,
            y: -443.2664987,
            z: 345.7675375
        }],
        [{
            x: 364.011377,
            y: -443.2664987,
            z: 345.7675375
        }, {
            x: 349.2213602,
            y: -443.5110756,
            z: 350.546662
        }, {
            x: 332.8115824,
            y: -446.932123,
            z: 349.6853998
        }, {
            x: 316.0232995,
            y: -450.8372243,
            z: 346.6828887
        }],
        [{
            x: 316.0232995,
            y: -450.8372243,
            z: 346.6828887
        }, {
            x: 301.2417016,
            y: -454.2755532,
            z: 344.0392645
        }, {
            x: 286.1666753,
            y: -458.0891349,
            z: 339.7356812
        }, {
            x: 271.7486942,
            y: -460.3300754,
            z: 339.2084314
        }],
        [{
            x: 271.7486942,
            y: -460.3300754,
            z: 339.2084314
        }, {
            x: 252.0021668,
            y: -463.3992149,
            z: 338.4863225
        }, {
            x: 233.4880849,
            y: -463.5184889,
            z: 344.8476319
        }, {
            x: 218.3500069,
            y: -468.0537965,
            z: 354.6242106
        }]
    ],
    [
        [{
            x: 639.4251955,
            y: -568.3604473,
            z: 307.637624
        }, {
            x: 635.1724108,
            y: -563.7599643,
            z: 308.6642318
        }, {
            x: 630.3369475,
            y: -561.3781427,
            z: 306.8118432
        }, {
            x: 626.0087454,
            y: -558.2828113,
            z: 305.0696097
        }],
        [{
            x: 626.0087454,
            y: -558.2828113,
            z: 305.0696097
        }, {
            x: 595.4037353,
            y: -536.3955131,
            z: 292.7501587
        }, {
            x: 590.1617908,
            y: -478.8327215,
            z: 285.9384676
        }, {
            x: 575.2545343,
            y: -442.4942707,
            z: 267.1038508
        }],
        [{
            x: 575.2545343,
            y: -442.4942707,
            z: 267.1038508
        }, {
            x: 569.8406774,
            y: -429.2972635,
            z: 260.2636975
        }, {
            x: 563.1520443,
            y: -418.8995746,
            z: 251.8378177
        }, {
            x: 557.5312958,
            y: -406.3767919,
            z: 244.8688948
        }],
        [{
            x: 557.5312958,
            y: -406.3767919,
            z: 244.8688948
        }, {
            x: 551.6736625,
            y: -393.3262403,
            z: 237.6062687
        }, {
            x: 546.9758219,
            y: -377.9676973,
            z: 231.9259933
        }, {
            x: 543.6998003,
            y: -359.1765798,
            z: 229.2066501
        }],
        [{
            x: 543.6998003,
            y: -359.1765798,
            z: 229.2066501
        }, {
            x: 540.4237787,
            y: -340.3854622,
            z: 226.4873068
        }, {
            x: 538.569576,
            y: -318.1617701,
            z: 226.7288958
        }, {
            x: 538.020617,
            y: -296.1845267,
            z: 225.9935607
        }],
        [{
            x: 538.020617,
            y: -296.1845267,
            z: 225.9935607
        }, {
            x: 537.5342673,
            y: -276.7138095,
            z: 225.3420912
        }, {
            x: 538.0724107,
            y: -257.4365313,
            z: 223.9238285
        }, {
            x: 538.5265418,
            y: -240.8397756,
            z: 219.5394271
        }],
        [{
            x: 538.5265418,
            y: -240.8397756,
            z: 219.5394271
        }, {
            x: 539.1907887,
            y: -216.5640881,
            z: 213.1264654
        }, {
            x: 539.6752977,
            y: -198.0231722,
            z: 200.3676782
        }, {
            x: 535.6886619,
            y: -179.0497163,
            z: 190.8755632
        }],
        [{
            x: 535.6886619,
            y: -179.0497163,
            z: 190.8755632
        }, {
            x: 531.2606097,
            y: -157.9754429,
            z: 180.3324431
        }, {
            x: 521.3164711,
            y: -136.3675415,
            z: 173.8194422
        }, {
            x: 509.5653502,
            y: -123.3499866,
            z: 158.983373
        }],
        [{
            x: 509.5653502,
            y: -123.3499866,
            z: 158.983373
        }, {
            x: 500.6359405,
            y: -113.4582424,
            z: 147.7097811
        }, {
            x: 490.6631549,
            y: -108.5266801,
            z: 131.6303373
        }, {
            x: 480.5057092,
            y: -107.7070599,
            z: 110.348552
        }]
    ],
    [
        [{
            x: 664.2837733,
            y: 416.7892743,
            z: 256.6105929
        }, {
            x: 647.1507061,
            y: 398.6027307,
            z: 262.5047778
        }, {
            x: 628.807291,
            y: 377.3188507,
            z: 258.1031956
        }, {
            x: 609.1558478,
            y: 364.0626759,
            z: 246.6168878
        }],
        [{
            x: 609.1558478,
            y: 364.0626759,
            z: 246.6168878
        }, {
            x: 599.5685104,
            y: 357.5953941,
            z: 241.01307
        }, {
            x: 589.6698407,
            y: 353.0388389,
            z: 233.7229703
        }, {
            x: 581.7381551,
            y: 347.5015262,
            z: 225.7014393
        }],
        [{
            x: 581.7381551,
            y: 347.5015262,
            z: 225.7014393
        }, {
            x: 569.5504002,
            y: 338.9929429,
            z: 213.3756289
        }, {
            x: 562.0069132,
            y: 328.1686822,
            z: 199.3228276
        }, {
            x: 557.7591221,
            y: 315.8138303,
            z: 184.0187088
        }],
        [{
            x: 557.7591221,
            y: 315.8138303,
            z: 184.0187088
        }, {
            x: 553.1230391,
            y: 302.3296176,
            z: 167.3156357
        }, {
            x: 552.4127117,
            y: 287.0222006,
            z: 149.1220225
        }, {
            x: 552.810437,
            y: 277.9100327,
            z: 129.4490311
        }],
        [{
            x: 552.810437,
            y: 277.9100327,
            z: 129.4490311
        }, {
            x: 553.2863961,
            y: 267.0054713,
            z: 105.9062976
        }, {
            x: 555.3491961,
            y: 264.9731186,
            z: 80.24494827
        }, {
            x: 557.3043449,
            y: 260.1931244,
            z: 55.16875665
        }],
        [{
            x: 557.3043449,
            y: 260.1931244,
            z: 55.16875665
        }, {
            x: 559.1995843,
            y: 255.5595983,
            z: 30.86094659
        }, {
            x: 560.9936687,
            y: 248.3442367,
            z: 7.10298298
        }, {
            x: 560.6673794,
            y: 241.1139817,
            z: -16.53412839
        }],
        [{
            x: 560.6673794,
            y: 241.1139817,
            z: -16.53412839
        }, {
            x: 560.267636,
            y: 232.2560564,
            z: -45.49241099
        }, {
            x: 556.6853868,
            y: 223.3757772,
            z: -74.26930436
        }, {
            x: 541.4269271,
            y: 218.8116854,
            z: -100.4376917
        }]
    ],
    [
        [{
            x: 640.3169195,
            y: 392.2793296,
            z: 283.106161
        }, {
            x: 616.1162597,
            y: 378.3073536,
            z: 286.7421501
        }, {
            x: 591.465975,
            y: 364.0757919,
            z: 285.8129698
        }, {
            x: 567.9012982,
            y: 349.2325321,
            z: 283.106161
        }],
        [{
            x: 567.9012982,
            y: 349.2325321,
            z: 283.106161
        }, {
            x: 551.2271309,
            y: 338.7295661,
            z: 281.1908459
        }, {
            x: 535.0965122,
            y: 327.9203316,
            z: 278.3854974
        }, {
            x: 513.3291496,
            y: 322.4591594,
            z: 276.8439543
        }],
        [{
            x: 513.3291496,
            y: 322.4591594,
            z: 276.8439543
        }, {
            x: 492.7891202,
            y: 317.3059105,
            z: 275.3893298
        }, {
            x: 467.2300723,
            y: 316.914635,
            z: 275.0600116
        }, {
            x: 446.3071771,
            y: 312.0175143,
            z: 273.712851
        }],
        [{
            x: 446.3071771,
            y: 312.0175143,
            z: 273.712851
        }, {
            x: 425.3842819,
            y: 307.1203937,
            z: 272.3656904
        }, {
            x: 409.0975393,
            y: 297.7174279,
            z: 270.0006875
        }, {
            x: 396.5710885,
            y: 284.7406213,
            z: 270.5817476
        }],
        [{
            x: 396.5710885,
            y: 284.7406213,
            z: 270.5817476
        }, {
            x: 383.2055267,
            y: 270.8945357,
            z: 271.2017313
        }, {
            x: 374.1209124,
            y: 252.9797697,
            z: 275.1756946
        }, {
            x: 358.671173,
            y: 241.7296538,
            z: 279.9750577
        }],
        [{
            x: 358.671173,
            y: 241.7296538,
            z: 279.9750577
        }, {
            x: 341.1291577,
            y: 228.9559946,
            z: 285.424373
        }, {
            x: 315.3812918,
            y: 224.7743303,
            z: 291.9377848
        }, {
            x: 317.5935625,
            y: 193.4006088,
            z: 298.7616778
        }]
    ],
    [
        [{
            x: 641.3765023,
            y: 407.1562185,
            z: 273.712851
        }, {
            x: 635.0696376,
            y: 389.3434658,
            z: 274.1857572
        }, {
            x: 627.8182635,
            y: 370.5021861,
            z: 267.7926058
        }, {
            x: 616.2067243,
            y: 355.4503724,
            z: 266.9614126
        }],
        [{
            x: 616.2067243,
            y: 355.4503724,
            z: 266.9614126
        }, {
            x: 604.8456946,
            y: 340.7232894,
            z: 266.1481518
        }, {
            x: 589.3106043,
            y: 329.6239268,
            z: 270.6594484
        }, {
            x: 579.4116413,
            y: 315.3178893,
            z: 269.3891192
        }],
        [{
            x: 579.4116413,
            y: 315.3178893,
            z: 269.3891192
        }, {
            x: 572.3454585,
            y: 305.1058019,
            z: 268.4823194
        }, {
            x: 568.1511877,
            y: 293.25974,
            z: 264.6294681
        }, {
            x: 568.3215038,
            y: 278.5361631,
            z: 258.1986835
        }],
        [{
            x: 568.3215038,
            y: 278.5361631,
            z: 258.1986835
        }, {
            x: 568.5987584,
            y: 254.5679164,
            z: 247.7301246
        }, {
            x: 580.4421685,
            y: 222.974255,
            z: 230.4300423
        }, {
            x: 581.2605594,
            y: 197.2225724,
            z: 219.6061913
        }],
        [{
            x: 581.2605594,
            y: 197.2225724,
            z: 219.6061913
        }, {
            x: 582.1918045,
            y: 167.9197902,
            z: 207.2897555
        }, {
            x: 568.8477298,
            y: 146.1812683,
            z: 203.3588173
        }, {
            x: 559.8587137,
            y: 135.7503946,
            z: 183.9265695
        }],
        [{
            x: 559.8587137,
            y: 135.7503946,
            z: 183.9265695
        }, {
            x: 551.1609511,
            y: 125.6574921,
            z: 165.1239469
        }, {
            x: 546.5406023,
            y: 126.1513496,
            z: 131.8082584
        }, {
            x: 540.1437051,
            y: 120.6123096,
            z: 106.1963318
        }],
        [{
            x: 540.1437051,
            y: 120.6123096,
            z: 106.1963318
        }, {
            x: 533.929885,
            y: 115.231795,
            z: 81.31740977
        }, {
            x: 526.0397499,
            y: 104.1587606,
            z: 63.70760158
        }, {
            x: 519.8299171,
            y: 92.87428493,
            z: 45.14230621
        }],
        [{
            x: 519.8299171,
            y: 92.87428493,
            z: 45.14230621
        }, {
            x: 511.9953016,
            y: 78.63726269,
            z: 21.71946124
        }, {
            x: 506.8353117,
            y: 64.06367845,
            z: -3.224282921
        }, {
            x: 526.5984039,
            y: 30.71126011,
            z: -30.00417426
        }]
    ],
    [
        [{
            x: 628.3338201,
            y: 382.6169727,
            z: 308.1549879
        }, {
            x: 604.615573,
            y: 359.0397809,
            z: 299.8537296
        }, {
            x: 586.9747887,
            y: 326.6933508,
            z: 300.7717557
        }, {
            x: 570.2931529,
            y: 294.9299717,
            z: 305.0238845
        }],
        [{
            x: 570.2931529,
            y: 294.9299717,
            z: 305.0238845
        }, {
            x: 563.647335,
            y: 282.2757191,
            z: 306.7178955
        }, {
            x: 557.1537489,
            y: 269.7140058,
            z: 308.9410805
        }, {
            x: 546.9433431,
            y: 260.8592605,
            z: 311.2860912
        }],
        [{
            x: 546.9433431,
            y: 260.8592605,
            z: 311.2860912
        }, {
            x: 537.4994555,
            y: 252.669261,
            z: 313.4550566
        }, {
            x: 524.8758609,
            y: 247.6505405,
            z: 315.7282428
        }, {
            x: 509.9912455,
            y: 244.4063696,
            z: 317.5482979
        }],
        [{
            x: 509.9912455,
            y: 244.4063696,
            z: 317.5482979
        }, {
            x: 487.4308754,
            y: 239.4892324,
            z: 320.3069259
        }, {
            x: 459.6762682,
            y: 238.648763,
            z: 322.024577
        }, {
            x: 438.5759739,
            y: 235.2973805,
            z: 330.0727113
        }],
        [{
            x: 438.5759739,
            y: 235.2973805,
            z: 330.0727113
        }, {
            x: 422.0722967,
            y: 232.6760836,
            z: 336.3675909
        }, {
            x: 409.6394932,
            y: 228.5186986,
            z: 346.5352367
        }, {
            x: 400.9734425,
            y: 219.4779668,
            z: 355.1215382
        }],
        [{
            x: 400.9734425,
            y: 219.4779668,
            z: 355.1215382
        }, {
            x: 392.1015186,
            y: 210.2224609,
            z: 363.9118182
        }, {
            x: 387.1774414,
            y: 195.8488319,
            z: 371.0447277
        }, {
            x: 383.304393,
            y: 179.8745904,
            z: 377.0392616
        }],
        [{
            x: 383.304393,
            y: 179.8745904,
            z: 377.0392616
        }, {
            x: 379.8942858,
            y: 165.8097325,
            z: 382.3172755
        }, {
            x: 377.2989671,
            y: 150.5040326,
            z: 386.7127873
        }, {
            x: 372.2852435,
            y: 138.1648128,
            z: 392.6947784
        }],
        [{
            x: 372.2852435,
            y: 138.1648128,
            z: 392.6947784
        }, {
            x: 365.4185657,
            y: 121.265308,
            z: 400.8875725
        }, {
            x: 354.0155901,
            y: 109.9301474,
            z: 412.056189
        }, {
            x: 346.3153707,
            y: 102.2757221,
            z: 427.1369153
        }]
    ],
    [
        [{
            x: 646.7559339,
            y: 392.7879484,
            z: 305.0238845
        }, {
            x: 640.7076704,
            y: 390.9204199,
            z: 304.5387142
        }, {
            x: 637.2574966,
            y: 386.5100511,
            z: 303.4751958
        }, {
            x: 633.4529445,
            y: 382.7191232,
            z: 301.8927812
        }],
        [{
            x: 633.4529445,
            y: 382.7191232,
            z: 301.8927812
        }, {
            x: 606.5506986,
            y: 355.9132154,
            z: 290.7034196
        }, {
            x: 561.9295353,
            y: 360.0793565,
            z: 253.569242
        }, {
            x: 533.972775,
            y: 345.293321,
            z: 223.6151973
        }],
        [{
            x: 533.972775,
            y: 345.293321,
            z: 223.6151973
        }, {
            x: 523.8197398,
            y: 339.9234878,
            z: 212.7368099
        }, {
            x: 515.8646043,
            y: 332.0540161,
            z: 202.8054238
        }, {
            x: 506.1880078,
            y: 326.3037285,
            z: 192.3041638
        }],
        [{
            x: 506.1880078,
            y: 326.3037285,
            z: 192.3041638
        }, {
            x: 496.1035942,
            y: 320.3110968,
            z: 181.3603313
        }, {
            x: 484.1495611,
            y: 316.6200379,
            z: 169.7975784
        }, {
            x: 469.0252049,
            y: 316.4476903,
            z: 157.8620269
        }],
        [{
            x: 469.0252049,
            y: 316.4476903,
            z: 157.8620269
        }, {
            x: 453.9008487,
            y: 316.2753427,
            z: 145.9264755
        }, {
            x: 435.6061692,
            y: 319.6217064,
            z: 133.6181255
        }, {
            x: 418.5253526,
            y: 323.4185119,
            z: 120.2887867
        }],
        [{
            x: 418.5253526,
            y: 323.4185119,
            z: 120.2887867
        }, {
            x: 403.3926199,
            y: 326.7822879,
            z: 108.4796721
        }, {
            x: 389.2126549,
            y: 330.4996183,
            z: 95.86917762
        }, {
            x: 378.2662797,
            y: 331.9280324,
            z: 82.71554645
        }],
        [{
            x: 378.2662797,
            y: 331.9280324,
            z: 82.71554645
        }, {
            x: 362.255272,
            y: 334.0173405,
            z: 63.47603685
        }, {
            x: 353.1622807,
            y: 331.2096884,
            z: 43.07452806
        }, {
            x: 340.2636184,
            y: 326.7274596,
            z: 26.3556861
        }],
        [{
            x: 340.2636184,
            y: 326.7274596,
            z: 26.3556861
        }, {
            x: 325.9367642,
            y: 321.7489405,
            z: 7.785666616
        }, {
            x: 306.9148237,
            y: 314.704483,
            z: -6.241017841
        }, {
            x: 297.0389437,
            y: 299.7502517,
            z: -20.6108642
        }],
        [{
            x: 297.0389437,
            y: 299.7502517,
            z: -20.6108642
        }, {
            x: 289.5344868,
            y: 288.386871,
            z: -31.53018394
        }, {
            x: 287.3110886,
            y: 272.4562804,
            z: -42.64765
        }, {
            x: 290.2821778,
            y: 252.6058456,
            z: -55.05300108
        }]
    ],
    [
        [{
            x: 99.59995617,
            y: 144.1116004,
            z: -1112.494482
        }, {
            x: 99.68444775,
            y: 165.2136997,
            z: -1085.87185
        }, {
            x: 101.7034384,
            y: 175.7394131,
            z: -1049.794581
        }, {
            x: 95.31331389,
            y: 174.8760046,
            z: -1015.539022
        }],
        [{
            x: 95.31331389,
            y: 174.8760046,
            z: -1015.539022
        }, {
            x: 92.19576783,
            y: 174.454774,
            z: -998.8267841
        }, {
            x: 87.07671336,
            y: 171.3227442,
            z: -982.548143
        }, {
            x: 84.75424366,
            y: 166.9976872,
            z: -966.6597933
        }],
        [{
            x: 84.75424366,
            y: 166.9976872,
            z: -966.6597933
        }, {
            x: 81.18555822,
            y: 160.3518447,
            z: -942.2459024
        }, {
            x: 84.21992014,
            y: 150.8891325,
            z: -918.7535332
        }, {
            x: 91.83764553,
            y: 139.3490332,
            z: -895.9973745
        }],
        [{
            x: 91.83764553,
            y: 139.3490332,
            z: -895.9973745
        }, {
            x: 100.1517096,
            y: 126.7540498,
            z: -871.1610682
        }, {
            x: 113.9253684,
            y: 111.6845319,
            z: -847.2017186
        }, {
            x: 123.1338098,
            y: 91.8163534,
            z: -828.6576677
        }],
        [{
            x: 123.1338098,
            y: 91.8163534,
            z: -828.6576677
        }, {
            x: 134.1535818,
            y: 68.04003886,
            z: -806.465941
        }, {
            x: 138.6355117,
            y: 37.39156853,
            z: -792.029457
        }, {
            x: 145.5110668,
            y: 8.643755826,
            z: -775.4720376
        }],
        [{
            x: 145.5110668,
            y: 8.643755826,
            z: -775.4720376
        }, {
            x: 152.1759419,
            y: -19.22316996,
            z: -759.4219686
        }, {
            x: 161.0899994,
            y: -45.30413299,
            z: -741.3789517
        }, {
            x: 167.9862958,
            y: -70.44626172,
            z: -721.619438
        }],
        [{
            x: 167.9862958,
            y: -70.44626172,
            z: -721.619438
        }, {
            x: 176.4350821,
            y: -101.2483702,
            z: -697.4116754
        }, {
            x: 181.8553757,
            y: -130.641366,
            z: -670.6275929
        }, {
            x: 172.0950966,
            y: -154.3933113,
            z: -639.2599517
        }]
    ],
    [
        [{
            x: 98.40453571,
            y: 195.1000809,
            z: -1086.303866
        }, {
            x: 87.9333244,
            y: 214.3440397,
            z: -1056.127207
        }, {
            x: 77.3851039,
            y: 228.3959676,
            z: -1022.787587
        }, {
            x: 68.47956971,
            y: 240.17331,
            z: -988.8311377
        }],
        [{
            x: 68.47956971,
            y: 240.17331,
            z: -988.8311377
        }, {
            x: 62.17808853,
            y: 248.5068585,
            z: -964.8038394
        }, {
            x: 56.69907718,
            y: 255.7015542,
            z: -940.4677034
        }, {
            x: 40.90767198,
            y: 264.2471196,
            z: -916.7248686
        }],
        [{
            x: 40.90767198,
            y: 264.2471196,
            z: -916.7248686
        }, {
            x: 26.00665102,
            y: 272.310851,
            z: -894.3207523
        }, {
            x: 1.923361563,
            y: 281.5774115,
            z: -872.4449179
        }, {
            x: -13.58073872,
            y: 289.8040158,
            z: -850.0012428
        }],
        [{
            x: -13.58073872,
            y: 289.8040158,
            z: -850.0012428
        }, {
            x: -29.08483901,
            y: 298.0306202,
            z: -827.5575678
        }, {
            x: -36.00975013,
            y: 305.2172682,
            z: -804.5460522
        }, {
            x: -36.14575797,
            y: 316.0443806,
            z: -783.2619342
        }],
        [{
            x: -36.14575797,
            y: 316.0443806,
            z: -783.2619342
        }, {
            x: -36.29087658,
            y: 327.5967702,
            z: -760.5520541
        }, {
            x: -28.70708992,
            y: 343.2936885,
            z: -739.8087498
        }, {
            x: -33.32474069,
            y: 359.5312292,
            z: -719.9346071
        }],
        [{
            x: -33.32474069,
            y: 359.5312292,
            z: -719.9346071
        }, {
            x: -38.5677353,
            y: 377.9677336,
            z: -697.3690151
        }, {
            x: -59.54069112,
            y: 397.1012025,
            z: -675.9239368
        }, {
            x: -28.92083594,
            y: 417.6955851,
            z: -654.6615352
        }]
    ],
    [
        [{
            x: 86.0583885,
            y: 177.1750197,
            z: -1094.689065
        }, {
            x: 96.29489665,
            y: 187.6029325,
            z: -1074.38758
        }, {
            x: 106.7422153,
            y: 190.5909781,
            z: -1048.55682
        }, {
            x: 109.4138589,
            y: 200.262228,
            z: -1025.461797
        }],
        [{
            x: 109.4138589,
            y: 200.262228,
            z: -1025.461797
        }, {
            x: 112.027864,
            y: 209.7248285,
            z: -1002.86503
        }, {
            x: 107.1980826,
            y: 225.5853745,
            z: -982.8872301
        }, {
            x: 110.8374167,
            y: 233.7795403,
            z: -961.6168834
        }],
        [{
            x: 110.8374167,
            y: 233.7795403,
            z: -961.6168834
        }, {
            x: 113.4352847,
            y: 239.6287866,
            z: -946.4334589
        }, {
            x: 120.3486256,
            y: 241.5715972,
            z: -930.5914118
        }, {
            x: 134.1326067,
            y: 240.0192337,
            z: -914.4767074
        }],
        [{
            x: 134.1326067,
            y: 240.0192337,
            z: -914.4767074
        }, {
            x: 156.5713023,
            y: 237.492169,
            z: -888.2438692
        }, {
            x: 197.2171944,
            y: 225.7028928,
            z: -861.288489
        }, {
            x: 221.8126723,
            y: 223.30191,
            z: -833.7765752
        }],
        [{
            x: 221.8126723,
            y: 223.30191,
            z: -833.7765752
        }, {
            x: 249.7998115,
            y: 220.5698371,
            z: -802.4708301
        }, {
            x: 257.0046927,
            y: 229.9938328,
            z: -770.4444791
        }, {
            x: 258.4332001,
            y: 214.4580683,
            z: -743.0604394
        }],
        [{
            x: 258.4332001,
            y: 214.4580683,
            z: -743.0604394
        }, {
            x: 259.8154223,
            y: 199.425679,
            z: -716.5636715
        }, {
            x: 255.7895276,
            y: 161.0247711,
            z: -694.4132571
        }, {
            x: 255.3830676,
            y: 135.0585897,
            z: -669.9248194
        }],
        [{
            x: 255.3830676,
            y: 135.0585897,
            z: -669.9248194
        }, {
            x: 254.9882402,
            y: 109.8355517,
            z: -646.1372323
        }, {
            x: 258.0086381,
            y: 96.34567004,
            z: -620.1435337
        }, {
            x: 262.853887,
            y: 81.17616818,
            z: -594.835437
        }],
        [{
            x: 262.853887,
            y: 81.17616818,
            z: -594.835437
        }, {
            x: 268.9668796,
            y: 62.03761512,
            z: -562.9055585
        }, {
            x: 277.9845836,
            y: 40.22552224,
            z: -532.0669885
        }, {
            x: 328.0543691,
            y: 14.88026253,
            z: -504.8931164
        }]
    ],
    [
        [{
            x: 95.13798849,
            y: 233.6364701,
            z: -1082.064303
        }, {
            x: 94.21063147,
            y: 242.45558,
            z: -1037.336266
        }, {
            x: 106.8783715,
            y: 263.7618119,
            z: -995.3336248
        }, {
            x: 119.8460947,
            y: 288.4595453,
            z: -956.5059311
        }],
        [{
            x: 119.8460947,
            y: 288.4595453,
            z: -956.5059311
        }, {
            x: 125.0123223,
            y: 298.2989064,
            z: -941.0373172
        }, {
            x: 130.2261619,
            y: 308.6765517,
            z: -926.0726171
        }, {
            x: 128.4931647,
            y: 319.016434,
            z: -911.2544755
        }],
        [{
            x: 128.4931647,
            y: 319.016434,
            z: -911.2544755
        }, {
            x: 126.8902675,
            y: 328.580078,
            z: -897.5487653
        }, {
            x: 119.3444116,
            y: 338.1114161,
            z: -883.9684347
        }, {
            x: 108.0251984,
            y: 347.1909938,
            z: -869.7684584
        }],
        [{
            x: 108.0251984,
            y: 347.1909938,
            z: -869.7684584
        }, {
            x: 90.86885052,
            y: 360.9527623,
            z: -848.2457847
        }, {
            x: 65.04397887,
            y: 373.6767029,
            z: -825.2996008
        }, {
            x: 47.71895309,
            y: 392.5714835,
            z: -809.3127509
        }],
        [{
            x: 47.71895309,
            y: 392.5714835,
            z: -809.3127509
        }, {
            x: 34.1681168,
            y: 407.3501095,
            z: -796.8085741
        }, {
            x: 25.81718622,
            y: 425.9038377,
            z: -788.5618723
        }, {
            x: 25.57314112,
            y: 443.2249261,
            z: -778.3802057
        }],
        [{
            x: 25.57314112,
            y: 443.2249261,
            z: -778.3802057
        }, {
            x: 25.32329842,
            y: 460.9574992,
            z: -767.9566606
        }, {
            x: 33.57009533,
            y: 477.3981711,
            z: -755.5051236
        }, {
            x: 44.31418773,
            y: 492.7602608,
            z: -741.9176644
        }],
        [{
            x: 44.31418773,
            y: 492.7602608,
            z: -741.9176644
        }, {
            x: 53.77405058,
            y: 506.2861368,
            z: -729.9542993
        }, {
            x: 65.16989022,
            y: 518.9758644,
            z: -717.1103341
        }, {
            x: 71.50060599,
            y: 533.2124865,
            z: -705.6845601
        }],
        [{
            x: 71.50060599,
            y: 533.2124865,
            z: -705.6845601
        }, {
            x: 80.17100521,
            y: 552.7106289,
            z: -690.0360892
        }, {
            x: 79.34055078,
            y: 575.1103429,
            z: -677.0477758
        }, {
            x: 78.58849583,
            y: 599.2671876,
            z: -672.5117069
        }]
    ],
    [
        [{
            x: 103.5430044,
            y: 218.7484997,
            z: -1104.432221
        }, {
            x: 99.47665455,
            y: 221.2026405,
            z: -1097.502467
        }, {
            x: 100.234512,
            y: 223.0719692,
            z: -1090.234765
        }, {
            x: 100.1003504,
            y: 224.1899228,
            z: -1082.911479
        }],
        [{
            x: 100.1003504,
            y: 224.1899228,
            z: -1082.911479
        }, {
            x: 99.15168448,
            y: 232.0950484,
            z: -1031.128029
        }, {
            x: 53.60206516,
            y: 202.431418,
            z: -976.5654086
        }, {
            x: 41.13412829,
            y: 183.1767698,
            z: -923.6830709
        }],
        [{
            x: 41.13412829,
            y: 183.1767698,
            z: -923.6830709
        }, {
            x: 36.60615722,
            y: 176.1840739,
            z: -904.4778329
        }, {
            x: 36.44139184,
            y: 170.5642384,
            z: -885.4942105
        }, {
            x: 32.70735446,
            y: 164.0068738,
            z: -866.5721465
        }],
        [{
            x: 32.70735446,
            y: 164.0068738,
            z: -866.5721465
        }, {
            x: 28.8159472,
            y: 157.173151,
            z: -846.8526178
        }, {
            x: 21.0480765,
            y: 149.3212101,
            z: -827.1999455
        }, {
            x: 7.040759694,
            y: 140.7154119,
            z: -807.7109169
        }],
        [{
            x: 7.040759694,
            y: 140.7154119,
            z: -807.7109169
        }, {
            x: -6.966557107,
            y: 132.1096137,
            z: -788.2218882
        }, {
            x: -27.21332001,
            y: 122.7499581,
            z: -768.8965032
        }, {
            x: -46.68474306,
            y: 111.5261518,
            z: -750.3986402
        }],
        [{
            x: -46.68474306,
            y: 111.5261518,
            z: -750.3986402
        }, {
            x: -63.93543116,
            y: 101.5824316,
            z: -734.0104767
        }, {
            x: -80.57755063,
            y: 90.1755289,
            z: -718.2718399
        }, {
            x: -92.02054563,
            y: 77.88271389,
            z: -702.9640715
        }],
        [{
            x: -92.02054563,
            y: 77.88271389,
            z: -702.9640715
        }, {
            x: -108.7579476,
            y: 59.90230002,
            z: -680.5737555
        }, {
            x: -114.3722227,
            y: 40.02654493,
            z: -659.1052494
        }, {
            x: -122.1897342,
            y: 26.68298117,
            z: -635.0875747
        }],
        [{
            x: -122.1897342,
            y: 26.68298117,
            z: -635.0875747
        }, {
            x: -130.8728319,
            y: 11.86196429,
            z: -608.4105663
        }, {
            x: -142.2740797,
            y: 5.099763234,
            z: -578.5886287
        }, {
            x: -137.6878462,
            y: -2.133050498,
            z: -549.4225921
        }],
        [{
            x: -137.6878462,
            y: -2.133050498,
            z: -549.4225921
        }, {
            x: -134.2028715,
            y: -7.629101317,
            z: -527.2599836
        }, {
            x: -121.4865097,
            y: -13.39689024,
            z: -505.4761011
        }, {
            x: -100.1856426,
            y: -20.98289197,
            z: -483.9424433
        }]
    ],
    [
        [{
            x: 390.9943644,
            y: 300.1683027,
            z: -775.1901231
        }, {
            x: 386.5804863,
            y: 289.225265,
            z: -736.4053534
        }, {
            x: 390.2250215,
            y: 258.5320966,
            z: -700.7749852
        }, {
            x: 387.2061599,
            y: 237.9716602,
            z: -663.908177
        }],
        [{
            x: 387.2061599,
            y: 237.9716602,
            z: -663.908177
        }, {
            x: 384.9420138,
            y: 222.5513329,
            z: -636.2580708
        }, {
            x: 378.929707,
            y: 212.8306674,
            z: -607.9124672
        }, {
            x: 373.2878401,
            y: 208.6596985,
            z: -576.0426754
        }],
        [{
            x: 373.2878401,
            y: 208.6596985,
            z: -576.0426754
        }, {
            x: 367.5172449,
            y: 204.3935622,
            z: -543.4457225
        }, {
            x: 362.134187,
            y: 205.9332624,
            z: -507.1619265
        }, {
            x: 355.5325139,
            y: 203.0543314,
            z: -476.9070506
        }],
        [{
            x: 355.5325139,
            y: 203.0543314,
            z: -476.9070506
        }, {
            x: 348.776239,
            y: 200.1079799,
            z: -445.943648
        }, {
            x: 340.7436041,
            y: 192.5336176,
            z: -421.2948495
        }, {
            x: 345.8255956,
            y: 179.2185517,
            z: -393.9739097
        }],
        [{
            x: 345.8255956,
            y: 179.2185517,
            z: -393.9739097
        }, {
            x: 348.9674638,
            y: 170.9867036,
            z: -377.0831314
        }, {
            x: 357.1219428,
            y: 160.5606707,
            z: -359.1710198
        }, {
            x: 365.0252614,
            y: 155.6925596,
            z: -342.1697229
        }],
        [{
            x: 365.0252614,
            y: 155.6925596,
            z: -342.1697229
        }, {
            x: 376.8560208,
            y: 148.4053104,
            z: -316.7198756
        }, {
            x: 388.1239761,
            y: 153.5723428,
            z: -293.3109966
        }, {
            x: 393.5494259,
            y: 160.5988197,
            z: -263.6769317
        }],
        [{
            x: 393.5494259,
            y: 160.5988197,
            z: -263.6769317
        }, {
            x: 398.504867,
            y: 167.0165904,
            z: -236.6100767
        }, {
            x: 398.5862324,
            y: 174.9855917,
            z: -204.3498973
        }, {
            x: 404.7995169,
            y: 171.0010152,
            z: -174.5396949
        }],
        [{
            x: 404.7995169,
            y: 171.0010152,
            z: -174.5396949
        }, {
            x: 408.5849271,
            y: 168.5734333,
            z: -156.3779886
        }, {
            x: 414.6463772,
            y: 161.7089339,
            z: -139.1256626
        }, {
            x: 417.6365889,
            y: 160.1608682,
            z: -123.0847881
        }]
    ],
    [
        [{
            x: 452.279591,
            y: 319.4962469,
            z: -741.1390511
        }, {
            x: 433.2254149,
            y: 311.9708969,
            z: -751.9835212
        }, {
            x: 414.3686329,
            y: 297.3052697,
            z: -757.8420917
        }, {
            x: 398.4712891,
            y: 279.1838976,
            z: -758.8939555
        }],
        [{
            x: 398.4712891,
            y: 279.1838976,
            z: -758.8939555
        }, {
            x: 380.1416327,
            y: 258.2899338,
            z: -760.1067557
        }, {
            x: 365.7462881,
            y: 232.8018621,
            z: -754.9294636
        }, {
            x: 357.0126106,
            y: 214.5441009,
            z: -738.3866205
        }],
        [{
            x: 357.0126106,
            y: 214.5441009,
            z: -738.3866205
        }, {
            x: 349.1643327,
            y: 198.1372687,
            z: -723.520852
        }, {
            x: 345.8879763,
            y: 187.5690722,
            z: -699.4771495
        }, {
            x: 346.5026622,
            y: 167.3776459,
            z: -686.0356989
        }],
        [{
            x: 346.5026622,
            y: 167.3776459,
            z: -686.0356989
        }, {
            x: 347.0959238,
            y: 147.8899711,
            z: -673.0627362
        }, {
            x: 351.3137181,
            y: 119.4381918,
            z: -669.9658438
        }, {
            x: 356.9326946,
            y: 90.77853609,
            z: -664.4799921
        }],
        [{
            x: 356.9326946,
            y: 90.77853609,
            z: -664.4799921
        }, {
            x: 362.7943276,
            y: 60.88120834,
            z: -658.7572331
        }, {
            x: 370.1807765,
            y: 30.75766217,
            z: -650.4347243
        }, {
            x: 370.2415327,
            y: 11.35232768,
            z: -636.3654932
        }],
        [{
            x: 370.2415327,
            y: 11.35232768,
            z: -636.3654932
        }, {
            x: 370.3205599,
            y: -13.8887235,
            z: -618.0652582
        }, {
            x: 358.0053071,
            y: -20.99571923,
            z: -590.0421902
        }, {
            x: 362.3480501,
            y: -45.00931123,
            z: -570.199232
        }]
    ],
    [
        [{
            x: 392.1384491,
            y: 300.646741,
            z: -778.0389429
        }, {
            x: 375.1440392,
            y: 273.8655962,
            z: -771.2252428
        }, {
            x: 362.2346559,
            y: 245.9549119,
            z: -759.7081986
        }, {
            x: 342.8514529,
            y: 222.4874996,
            z: -736.7023043
        }],
        [{
            x: 342.8514529,
            y: 222.4874996,
            z: -736.7023043
        }, {
            x: 332.4170624,
            y: 209.8544923,
            z: -724.317742
        }, {
            x: 320.106625,
            y: 198.5090999,
            z: -708.6038283
        }, {
            x: 310.4683784,
            y: 185.4570333,
            z: -697.6021539
        }],
        [{
            x: 310.4683784,
            y: 185.4570333,
            z: -697.6021539
        }, {
            x: 300.4625962,
            y: 171.9072518,
            z: -686.1809523
        }, {
            x: 293.3366879,
            y: 156.5181529,
            z: -679.8382261
        }, {
            x: 288.5559363,
            y: 138.5456074,
            z: -679.0027958
        }],
        [{
            x: 288.5559363,
            y: 138.5456074,
            z: -679.0027958
        }, {
            x: 281.2858922,
            y: 111.2149257,
            z: -677.7323648
        }, {
            x: 279.4390228,
            y: 77.91002357,
            z: -689.1975553
        }, {
            x: 267.4980643,
            y: 52.76428944,
            z: -696.9341103
        }],
        [{
            x: 267.4980643,
            y: 52.76428944,
            z: -696.9341103
        }, {
            x: 256.7734295,
            y: 30.17993693,
            z: -703.8826083
        }, {
            x: 237.9063693,
            y: 14.17720023,
            z: -707.8233919
        }, {
            x: 219.4712561,
            y: -2.299387739,
            z: -710.9698487
        }],
        [{
            x: 219.4712561,
            y: -2.299387739,
            z: -710.9698487
        }, {
            x: 193.400069,
            y: -25.6008019,
            z: -715.4196107
        }, {
            x: 168.1927757,
            y: -49.84991858,
            z: -718.2807191
        }, {
            x: 139.0538056,
            y: -68.55956548,
            z: -722.6217437
        }],
        [{
            x: 139.0538056,
            y: -68.55956548,
            z: -722.6217437
        }, {
            x: 119.5123886,
            y: -81.1067835,
            z: -725.5329574
        }, {
            x: 98.20273046,
            y: -91.16266782,
            z: -729.109752
        }, {
            x: 81.7170845,
            y: -108.7525236,
            z: -733.2735583
        }],
        [{
            x: 81.7170845,
            y: -108.7525236,
            z: -733.2735583
        }, {
            x: 61.98475959,
            y: -129.8065212,
            z: -738.2573835
        }, {
            x: 49.16362699,
            y: -161.6541743,
            z: -744.0821997
        }, {
            x: 31.85471319,
            y: -183.658355,
            z: -741.5608273
        }]
    ],
    [
        [{
            x: 440.9208473,
            y: 322.7523464,
            z: -749.7247128
        }, {
            x: 441.7199236,
            y: 301.0723646,
            z: -723.500968
        }, {
            x: 444.3831861,
            y: 271.2123735,
            z: -692.074298
        }, {
            x: 441.6855085,
            y: 257.3395554,
            z: -663.3253253
        }],
        [{
            x: 441.6855085,
            y: 257.3395554,
            z: -663.3253253
        }, {
            x: 440.276372,
            y: 250.0930653,
            z: -648.3082506
        }, {
            x: 437.4044961,
            y: 247.2086961,
            z: -634.0217891
        }, {
            x: 435.6085091,
            y: 243.0072899,
            z: -616.8045089
        }],
        [{
            x: 435.6085091,
            y: 243.0072899,
            z: -616.8045089
        }, {
            x: 433.9141783,
            y: 239.0436912,
            z: -600.5617593
        }, {
            x: 433.1773887,
            y: 233.9079296,
            z: -581.7105811
        }, {
            x: 435.2321655,
            y: 224.2665537,
            z: -567.8378062
        }],
        [{
            x: 435.2321655,
            y: 224.2665537,
            z: -567.8378062
        }, {
            x: 437.2869422,
            y: 214.6251779,
            z: -553.9650313
        }, {
            x: 442.1332853,
            y: 200.4781878,
            z: -545.0706596
        }, {
            x: 452.2637007,
            y: 187.7497091,
            z: -535.7807022
        }],
        [{
            x: 452.2637007,
            y: 187.7497091,
            z: -535.7807022
        }, {
            x: 464.7172573,
            y: 172.1022927,
            z: -524.3603405
        }, {
            x: 485.1562941,
            y: 158.5985815,
            z: -512.3421555
        }, {
            x: 500.5358892,
            y: 147.9982515,
            z: -497.9988503
        }],
        [{
            x: 500.5358892,
            y: 147.9982515,
            z: -497.9988503
        }, {
            x: 520.8216682,
            y: 134.0163521,
            z: -479.079944
        }, {
            x: 532.3051473,
            y: 125.0856882,
            z: -456.1158471
        }, {
            x: 532.8118161,
            y: 125.3808369,
            z: -426.5740744
        }],
        [{
            x: 532.8118161,
            y: 125.3808369,
            z: -426.5740744
        }, {
            x: 533.1784092,
            y: 125.5943875,
            z: -405.1995397
        }, {
            x: 527.7985953,
            y: 130.6376904,
            z: -380.3815637
        }, {
            x: 535.4009409,
            y: 123.5005279,
            z: -363.5767294
        }],
        [{
            x: 535.4009409,
            y: 123.5005279,
            z: -363.5767294
        }, {
            x: 544.3567333,
            y: 115.0927351,
            z: -343.7801275
        }, {
            x: 571.3285838,
            y: 89.7814393,
            z: -335.103804
        }, {
            x: 579.9243174,
            y: 87.72307963,
            z: -303.3593027
        }]
    ],
    [
        [{
            x: 410.8831029,
            y: 312.5493686,
            z: -772.1577995
        }, {
            x: 410.8592596,
            y: 305.2841539,
            z: -769.7731759
        }, {
            x: 411.5899822,
            y: 298.0835968,
            z: -766.9132341
        }, {
            x: 412.6773087,
            y: 291.3507244,
            z: -762.9407804
        }],
        [{
            x: 412.6773087,
            y: 291.3507244,
            z: -762.9407804
        }, {
            x: 416.3232175,
            y: 268.7747665,
            z: -749.6207674
        }, {
            x: 423.9785117,
            y: 251.4571015,
            z: -723.7925124
        }, {
            x: 429.4013523,
            y: 235.6996341,
            z: -700.5908483
        }],
        [{
            x: 429.4013523,
            y: 235.6996341,
            z: -700.5908483
        }, {
            x: 434.4460067,
            y: 221.0410845,
            z: -679.0072565
        }, {
            x: 437.55873,
            y: 207.732706,
            z: -659.696676
        }, {
            x: 454.9889834,
            y: 191.7332875,
            z: -649.7814667
        }],
        [{
            x: 454.9889834,
            y: 191.7332875,
            z: -649.7814667
        }, {
            x: 461.6134989,
            y: 185.6525729,
            z: -646.0131073
        }, {
            x: 470.3061021,
            y: 179.1831526,
            z: -643.6018571
        }, {
            x: 479.7096627,
            y: 172.908779,
            z: -642.6714068
        }],
        [{
            x: 479.7096627,
            y: 172.908779,
            z: -642.6714068
        }, {
            x: 500.0794064,
            y: 159.317397,
            z: -640.6558898
        }, {
            x: 523.7851807,
            y: 146.6412342,
            z: -645.5887413
        }, {
            x: 544.0171872,
            y: 138.282023,
            z: -643.150065
        }],
        [{
            x: 544.0171872,
            y: 138.282023,
            z: -643.150065
        }, {
            x: 584.2525294,
            y: 121.65808,
            z: -638.3002753
        }, {
            x: 610.7494047,
            y: 122.1073268,
            z: -604.2966974
        }, {
            x: 643.3888117,
            y: 120.9478219,
            z: -582.1299813
        }],
        [{
            x: 643.3888117,
            y: 120.9478219,
            z: -582.1299813
        }, {
            x: 672.5256062,
            y: 119.9127462,
            z: -562.3420282
        }, {
            x: 706.5573302,
            y: 117.5956703,
            z: -551.9867659
        }, {
            x: 738.3708935,
            y: 119.1449947,
            z: -537.9779493
        }],
        [{
            x: 738.3708935,
            y: 119.1449947,
            z: -537.9779493
        }, {
            x: 770.3289239,
            y: 120.7013545,
            z: -523.905518
        }, {
            x: 800.0486024,
            y: 126.1593094,
            z: -506.146275
        }, {
            x: 824.7833472,
            y: 129.800364,
            z: -479.9392038
        }],
        [{
            x: 824.7833472,
            y: 129.800364,
            z: -479.9392038
        }, {
            x: 853.3767572,
            y: 134.0094298,
            z: -449.6437817
        }, {
            x: 875.3086,
            y: 135.7904988,
            z: -408.059187
        }, {
            x: 911.2290409,
            y: 92.86099853,
            z: -371.7506359
        }]
    ],
    [
        [{
            x: 423.4920056,
            y: 316.0915639,
            z: -763.8324556
        }, {
            x: 413.7837509,
            y: 315.7072473,
            z: -727.3710515
        }, {
            x: 420.675425,
            y: 331.3446224,
            z: -696.9696292
        }, {
            x: 430.0718016,
            y: 342.6509413,
            z: -663.2263502
        }],
        [{
            x: 430.0718016,
            y: 342.6509413,
            z: -663.2263502
        }, {
            x: 435.2060179,
            y: 348.8287574,
            z: -644.7888929
        }, {
            x: 441.0880311,
            y: 353.7135057,
            z: -625.3537004
        }, {
            x: 443.1942211,
            y: 349.27357,
            z: -602.359293
        }],
        [{
            x: 443.1942211,
            y: 349.27357,
            z: -602.359293
        }, {
            x: 444.9828583,
            y: 345.5030486,
            z: -582.8317794
        }, {
            x: 444.0484111,
            y: 335.0076621,
            z: -560.7373971
        }, {
            x: 446.0078865,
            y: 330.2965444,
            z: -540.7309929
        }],
        [{
            x: 446.0078865,
            y: 330.2965444,
            z: -540.7309929
        }, {
            x: 449.2921441,
            y: 322.4002861,
            z: -507.1984548
        }, {
            x: 460.7062358,
            y: 330.7536487,
            z: -479.5316282
        }, {
            x: 469.5787915,
            y: 346.6572133,
            z: -454.701629
        }],
        [{
            x: 469.5787915,
            y: 346.6572133,
            z: -454.701629
        }, {
            x: 475.2796695,
            y: 356.8757212,
            z: -438.7476244
        }, {
            x: 479.9312895,
            y: 370.211285,
            z: -423.964787
        }, {
            x: 485.4148498,
            y: 379.390158,
            z: -407.9956841
        }],
        [{
            x: 485.4148498,
            y: 379.390158,
            z: -407.9956841
        }, {
            x: 491.1252465,
            y: 388.9487299,
            z: -391.3659934
        }, {
            x: 497.7378362,
            y: 393.9996017,
            z: -373.4498636
        }, {
            x: 503.4833154,
            y: 403.886887,
            z: -357.1733623
        }],
        [{
            x: 503.4833154,
            y: 403.886887,
            z: -357.1733623
        }, {
            x: 509.0005646,
            y: 413.3814158,
            z: -341.543419
        }, {
            x: 513.7182243,
            y: 427.3357521,
            z: -327.4254281
        }, {
            x: 519.5027208,
            y: 437.9604634,
            z: -312.2575815
        }],
        [{
            x: 519.5027208,
            y: 437.9604634,
            z: -312.2575815
        }, {
            x: 528.7015051,
            y: 454.8563897,
            z: -288.1369429
        }, {
            x: 540.5982013,
            y: 463.3320641,
            z: -261.3613357
        }, {
            x: 557.9695846,
            y: 473.7893463,
            z: -239.3924013
        }],
        [{
            x: 557.9695846,
            y: 473.7893463,
            z: -239.3924013
        }, {
            x: 567.4367862,
            y: 479.4884433,
            z: -227.419588
        }, {
            x: 578.5300372,
            y: 485.7761021,
            z: -216.8744157
        }, {
            x: 588.4415893,
            y: 488.3921044,
            z: -203.5432985
        }],
        [{
            x: 588.4415893,
            y: 488.3921044,
            z: -203.5432985
        }, {
            x: 605.1802935,
            y: 492.8100288,
            z: -181.029607
        }, {
            x: 618.5487084,
            y: 486.7561278,
            z: -150.5702031
        }, {
            x: 639.6337469,
            y: 491.7082216,
            z: -131.9191228
        }],
        [{
            x: 639.6337469,
            y: 491.7082216,
            z: -131.9191228
        }, {
            x: 656.0730987,
            y: 495.5692157,
            z: -117.377453
        }, {
            x: 677.203261,
            y: 506.120576,
            z: -110.0138719
        }, {
            x: 696.7364201,
            y: 514.8581338,
            z: -112.279417
        }]
    ],
    [
        [{
            x: 769.9340338,
            y: 538.6228329,
            z: -305.1419833
        }, {
            x: 741.1018882,
            y: 511.0594952,
            z: -297.8990178
        }, {
            x: 725.5165107,
            y: 466.7448442,
            z: -293.6347388
        }, {
            x: 702.0525903,
            y: 431.7070075,
            z: -290.0568828
        }],
        [{
            x: 702.0525903,
            y: 431.7070075,
            z: -290.0568828
        }, {
            x: 684.45465,
            y: 405.42863,
            z: -287.3734907
        }, {
            x: 662.4250293,
            y: 384.3684605,
            z: -285.0762116
        }, {
            x: 636.0690476,
            y: 366.1980269,
            z: -278.7430574
        }],
        [{
            x: 636.0690476,
            y: 366.1980269,
            z: -278.7430574
        }, {
            x: 609.1117113,
            y: 347.6130053,
            z: -272.2654019
        }, {
            x: 577.6283356,
            y: 332.051092,
            z: -261.5656003
        }, {
            x: 551.578121,
            y: 315.9604253,
            z: -256.1154066
        }],
        [{
            x: 551.578121,
            y: 315.9604253,
            z: -256.1154066
        }, {
            x: 524.917847,
            y: 299.4929379,
            z: -250.5375769
        }, {
            x: 503.9481876,
            y: 282.4716418,
            z: -250.4581111
        }, {
            x: 489.9220146,
            y: 256.7047185,
            z: -241.030306
        }],
        [{
            x: 489.9220146,
            y: 256.7047185,
            z: -241.030306
        }, {
            x: 481.2505348,
            y: 240.7746886,
            z: -235.201701
        }, {
            x: 475.2329617,
            y: 221.5019463,
            z: -225.8000178
        }, {
            x: 467.9764409,
            y: 207.4374703,
            z: -214.6313801
        }],
        [{
            x: 467.9764409,
            y: 207.4374703,
            z: -214.6313801
        }, {
            x: 457.1138962,
            y: 186.3838549,
            z: -197.9126481
        }, {
            x: 443.4750974,
            y: 177.0010076,
            z: -177.2344954
        }, {
            x: 421.2368879,
            y: 167.2607895,
            z: -158.062253
        }],
        [{
            x: 421.2368879,
            y: 167.2607895,
            z: -158.062253
        }, {
            x: 400.9251827,
            y: 158.36437,
            z: -140.5509091
        }, {
            x: 373.43947,
            y: 149.169816,
            z: -124.295862
        }, {
            x: 355.0727417,
            y: 129.8586809,
            z: -109.0356763
        }],
        [{
            x: 355.0727417,
            y: 129.8586809,
            z: -109.0356763
        }, {
            x: 343.8829108,
            y: 118.0934752,
            z: -99.73848981
        }, {
            x: 336.0778557,
            y: 102.5732066,
            z: -90.81057503
        }, {
            x: 325.808642,
            y: 92.75590727,
            z: -82.63675033
        }]
    ],
    [
        [{
            x: 770.7103298,
            y: 526.2134424,
            z: -233.4877557
        }, {
            x: 771.1608892,
            y: 528.914237,
            z: -256.5050918
        }, {
            x: 770.1849383,
            y: 523.0640812,
            z: -280.3756781
        }, {
            x: 768.2280657,
            y: 511.3339726,
            z: -301.3707082
        }],
        [{
            x: 768.2280657,
            y: 511.3339726,
            z: -301.3707082
        }, {
            x: 765.9717892,
            y: 497.8091433,
            z: -325.578003
        }, {
            x: 762.4114638,
            y: 476.4674357,
            z: -345.9624995
        }, {
            x: 750.7986394,
            y: 454.5336375,
            z: -354.1685601
        }],
        [{
            x: 750.7986394,
            y: 454.5336375,
            z: -354.1685601
        }, {
            x: 740.3630958,
            y: 434.8234363,
            z: -361.5427095
        }, {
            x: 723.4249811,
            y: 414.6351088,
            z: -359.0825025
        }, {
            x: 719.9303633,
            y: 390.7683496,
            z: -361.7111103
        }],
        [{
            x: 719.9303633,
            y: 390.7683496,
            z: -361.7111103
        }, {
            x: 716.5575468,
            y: 367.7334418,
            z: -364.2481008
        }, {
            x: 725.7074397,
            y: 341.2720496,
            z: -371.5253572
        }, {
            x: 733.8412325,
            y: 313.1810751,
            z: -376.7962109
        }],
        [{
            x: 733.8412325,
            y: 313.1810751,
            z: -376.7962109
        }, {
            x: 742.3262844,
            y: 283.8769871,
            z: -382.2946873
        }, {
            x: 749.7055803,
            y: 252.7995302,
            z: -385.6097253
        }, {
            x: 745.1684258,
            y: 229.3970381,
            z: -388.1100363
        }],
        [{
            x: 745.1684258,
            y: 229.3970381,
            z: -388.1100363
        }, {
            x: 739.2668245,
            y: 198.9567744,
            z: -391.3622594
        }, {
            x: 713.2038775,
            y: 181.5017214,
            z: -393.2360522
        }, {
            x: 708.0331617,
            y: 150.5068284,
            z: -391.8813114
        }]
    ],
    [
        [{
            x: 772.6041413,
            y: 540.211585,
            z: -305.1419833
        }, {
            x: 766.783231,
            y: 517.4737923,
            z: -327.5378753
        }, {
            x: 759.9184225,
            y: 490.6582971,
            z: -345.2043521
        }, {
            x: 739.2012879,
            y: 463.1987552,
            z: -361.7111103
        }],
        [{
            x: 739.2012879,
            y: 463.1987552,
            z: -361.7111103
        }, {
            x: 728.0488137,
            y: 448.4166997,
            z: -370.5970494
        }, {
            x: 712.8820768,
            y: 433.4480061,
            z: -379.1469139
        }, {
            x: 703.3685611,
            y: 418.838265,
            z: -388.1100363
        }],
        [{
            x: 703.3685611,
            y: 418.838265,
            z: -388.1100363
        }, {
            x: 693.4922662,
            y: 403.67141,
            z: -397.4149498
        }, {
            x: 689.708562,
            y: 388.8914054,
            z: -407.1652397
        }, {
            x: 692.2995402,
            y: 374.1839385,
            z: -418.2802374
        }],
        [{
            x: 692.2995402,
            y: 374.1839385,
            z: -418.2802374
        }, {
            x: 696.2396165,
            y: 351.8184314,
            z: -435.1827101
        }, {
            x: 714.9211458,
            y: 329.6206676,
            z: -455.2410694
        }, {
            x: 722.570105,
            y: 314.4668614,
            z: -478.6206395
        }],
        [{
            x: 722.570105,
            y: 314.4668614,
            z: -478.6206395
        }, {
            x: 729.4399299,
            y: 300.8566443,
            z: -499.6187321
        }, {
            x: 727.4103012,
            y: 292.9284554,
            z: -523.2958887
        }, {
            x: 725.1558653,
            y: 284.1221186,
            z: -546.503592
        }],
        [{
            x: 725.1558653,
            y: 284.1221186,
            z: -546.503592
        }, {
            x: 721.9676115,
            y: 271.6680777,
            z: -579.3242407
        }, {
            x: 718.3297433,
            y: 257.4577413,
            z: -611.205983
        }, {
            x: 711.8940937,
            y: 249.44001,
            z: -644.5567455
        }],
        [{
            x: 711.8940937,
            y: 249.44001,
            z: -644.5567455
        }, {
            x: 707.5781652,
            y: 244.0630928,
            z: -666.9227094
        }, {
            x: 702.0039561,
            y: 241.4712519,
            z: -689.9493539
        }, {
            x: 701.9597181,
            y: 231.8432042,
            z: -712.4396979
        }],
        [{
            x: 701.9597181,
            y: 231.8432042,
            z: -712.4396979
        }, {
            x: 701.9067678,
            y: 220.3190113,
            z: -739.3592846
        }, {
            x: 709.7764121,
            y: 198.7142929,
            z: -765.5105326
        }, {
            x: 705.5500849,
            y: 182.2053026,
            z: -787.8652006
        }]
    ],
    [
        [{
            x: 770.1024714,
            y: 535.431072,
            z: -244.8015811
        }, {
            x: 757.310142,
            y: 503.9547008,
            z: -242.8168165
        }, {
            x: 744.1693381,
            y: 462.6136219,
            z: -240.6762343
        }, {
            x: 724.9827936,
            y: 437.1885318,
            z: -237.2590309
        }],
        [{
            x: 724.9827936,
            y: 437.1885318,
            z: -237.2590309
        }, {
            x: 714.9606694,
            y: 423.9076928,
            z: -235.4740489
        }, {
            x: 703.2889572,
            y: 414.969552,
            z: -233.3407392
        }, {
            x: 690.3655162,
            y: 403.2579534,
            z: -229.7164806
        }],
        [{
            x: 690.3655162,
            y: 403.2579534,
            z: -229.7164806
        }, {
            x: 678.1735663,
            y: 392.2092534,
            z: -226.2973619
        }, {
            x: 664.867578,
            y: 378.6921759,
            z: -221.5512988
        }, {
            x: 658.3952409,
            y: 363.2705347,
            z: -218.4026552
        }],
        [{
            x: 658.3952409,
            y: 363.2705347,
            z: -218.4026552
        }, {
            x: 651.9229037,
            y: 347.8488936,
            z: -215.2540116
        }, {
            x: 652.2842178,
            y: 330.5226888,
            z: -213.7027875
        }, {
            x: 654.756967,
            y: 313.1709202,
            z: -207.0888298
        }],
        [{
            x: 654.756967,
            y: 313.1709202,
            z: -207.0888298
        }, {
            x: 657.7967753,
            y: 291.8399856,
            z: -198.9581369
        }, {
            x: 664.0274602,
            y: 270.4704181,
            z: -183.1764589
        }, {
            x: 664.7611212,
            y: 251.4010556,
            z: -169.3760784
        }],
        [{
            x: 664.7611212,
            y: 251.4010556,
            z: -169.3760784
        }, {
            x: 665.7288243,
            y: 226.248451,
            z: -151.1732932
        }, {
            x: 657.1329319,
            y: 205.09769,
            z: -136.4175237
        }, {
            x: 634.4744169,
            y: 190.6601737,
            z: -124.1207768
        }],
        [{
            x: 634.4744169,
            y: 190.6601737,
            z: -124.1207768
        }, {
            x: 618.0801667,
            y: 180.2141116,
            z: -115.2236384
        }, {
            x: 594.3240735,
            y: 173.2824617,
            z: -107.6138087
        }, {
            x: 587.7929205,
            y: 157.4511507,
            z: -97.72185087
        }],
        [{
            x: 587.7929205,
            y: 157.4511507,
            z: -97.72185087
        }, {
            x: 580.0990251,
            y: 138.8013884,
            z: -86.06882598
        }, {
            x: 596.3091283,
            y: 107.8010759,
            z: -71.24876598
        }, {
            x: 577.1187321,
            y: 88.70149814,
            z: -52.46654925
        }]
    ],
    [
        [{
            x: 774.3995194,
            y: 543.7313119,
            z: -282.5143325
        }, {
            x: 774.9041355,
            y: 536.4023032,
            z: -284.6359684
        }, {
            x: 775.4312892,
            y: 528.7459613,
            z: -285.9258455
        }, {
            x: 775.1406163,
            y: 520.8668549,
            z: -286.2856076
        }],
        [{
            x: 775.1406163,
            y: 520.8668549,
            z: -286.2856076
        }, {
            x: 774.1659624,
            y: 494.4474658,
            z: -287.4919242
        }, {
            x: 763.9962853,
            y: 465.5234805,
            z: -278.2407324
        }, {
            x: 754.1341269,
            y: 439.654672,
            z: -271.2005071
        }],
        [{
            x: 754.1341269,
            y: 439.654672,
            z: -271.2005071
        }, {
            x: 744.9597505,
            y: 415.589941,
            z: -264.6512635
        }, {
            x: 736.0514959,
            y: 394.1691131,
            z: -260.0153564
        }, {
            x: 743.0801641,
            y: 372.3110822,
            z: -248.5728563
        }],
        [{
            x: 743.0801641,
            y: 372.3110822,
            z: -248.5728563
        }, {
            x: 745.751469,
            y: 364.0037523,
            z: -244.2240372
        }, {
            x: 750.7247739,
            y: 355.6332708,
            z: -238.8920435
        }, {
            x: 757.1667508,
            y: 348.0204487,
            z: -233.4877557
        }],
        [{
            x: 757.1667508,
            y: 348.0204487,
            z: -233.4877557
        }, {
            x: 771.1211906,
            y: 331.5297551,
            z: -221.7811307
        }, {
            x: 791.9670914,
            y: 318.5942323,
            z: -209.7352796
        }, {
            x: 803.8189446,
            y: 306.3548722,
            z: -195.7750044
        }],
        [{
            x: 803.8189446,
            y: 306.3548722,
            z: -195.7750044
        }, {
            x: 827.3886961,
            y: 282.0144866,
            z: -168.012239
        }, {
            x: 815.3877262,
            y: 260.4273676,
            z: -132.6780835
        }, {
            x: 816.4064749,
            y: 242.1226525,
            z: -97.72185087
        }],
        [{
            x: 816.4064749,
            y: 242.1226525,
            z: -97.72185087
        }, {
            x: 817.3158992,
            y: 225.7822599,
            z: -66.51685471
        }, {
            x: 828.6006226,
            y: 212.057586,
            z: -35.61302202
        }, {
            x: 834.5960057,
            y: 200.2380927,
            z: -3.439972488
        }],
        [{
            x: 834.5960057,
            y: 200.2380927,
            z: -3.439972488
        }, {
            x: 840.6186141,
            y: 188.3649264,
            z: 28.87917663
        }, {
            x: 841.3037348,
            y: 178.414283,
            z: 62.47909593
        }, {
            x: 833.3330427,
            y: 163.7260528,
            z: 94.61318103
        }],
        [{
            x: 833.3330427,
            y: 163.7260528,
            z: 94.61318103
        }, {
            x: 824.1189081,
            y: 146.7464319,
            z: 131.7602419
        }, {
            x: 803.3376629,
            y: 123.4357832,
            z: 166.9484495
        }, {
            x: 808.7855875,
            y: 62.11334475,
            z: 192.6663345
        }]
    ],
    [
        [{
            x: 773.6801999,
            y: 540.1584637,
            z: -267.429232
        }, {
            x: 740.3288474,
            y: 523.7234194,
            z: -260.9942885
        }, {
            x: 715.497579,
            y: 520.6141608,
            z: -236.7041718
        }, {
            x: 690.8546288,
            y: 511.695482,
            z: -210.8601049
        }],
        [{
            x: 690.8546288,
            y: 511.695482,
            z: -210.8601049
        }, {
            x: 677.3896261,
            y: 506.8222817,
            z: -196.7388075
        }, {
            x: 663.9808472,
            y: 500.2146373,
            z: -182.1535671
        }, {
            x: 648.7942012,
            y: 484.6857693,
            z: -173.1473536
        }],
        [{
            x: 648.7942012,
            y: 484.6857693,
            z: -173.1473536
        }, {
            x: 635.8972637,
            y: 471.4982071,
            z: -165.4990175
        }, {
            x: 621.7181469,
            y: 451.8767512,
            z: -161.8742176
        }, {
            x: 608.8497253,
            y: 437.6223278,
            z: -154.2909779
        }],
        [{
            x: 608.8497253,
            y: 437.6223278,
            z: -154.2909779
        }, {
            x: 587.2810901,
            y: 413.7306284,
            z: -141.5807841
        }, {
            x: 569.3945626,
            y: 404.916417,
            z: -117.7502311
        }, {
            x: 549.8630188,
            y: 404.3971143,
            z: -93.95057574
        }],
        [{
            x: 549.8630188,
            y: 404.3971143,
            z: -93.95057574
        }, {
            x: 537.3134275,
            y: 404.063447,
            z: -78.65859735
        }, {
            x: 524.0847011,
            y: 407.1542834,
            z: -63.3793749
        }, {
            x: 511.7429372,
            y: 405.9746916,
            z: -48.69527411
        }],
        [{
            x: 511.7429372,
            y: 405.9746916,
            z: -48.69527411
        }, {
            x: 498.8906361,
            y: 404.7463041,
            z: -33.40374166
        }, {
            x: 487.0001967,
            y: 398.8868745,
            z: -18.75758558
        }, {
            x: 474.3332246,
            y: 398.1045801,
            z: -3.439972488
        }],
        [{
            x: 474.3332246,
            y: 398.1045801,
            z: -3.439972488
        }, {
            x: 462.1694277,
            y: 397.3533611,
            z: 11.26917293
        }, {
            x: 449.2895659,
            y: 401.2839265,
            z: 26.59748978
        }, {
            x: 437.2612834,
            y: 401.6663446,
            z: 41.81532914
        }],
        [{
            x: 437.2612834,
            y: 401.6663446,
            z: 41.81532914
        }, {
            x: 418.1333304,
            y: 402.2744843,
            z: 66.01546863
        }, {
            x: 401.1589268,
            y: 393.9097538,
            z: 89.93622282
        }, {
            x: 390.2386219,
            y: 388.5283919,
            z: 117.2408318
        }],
        [{
            x: 390.2386219,
            y: 388.5283919,
            z: 117.2408318
        }, {
            x: 384.2871829,
            y: 385.5956124,
            z: 132.1215257
        }, {
            x: 380.1338858,
            y: 383.5489301,
            z: 148.007266
        }, {
            x: 374.3763739,
            y: 377.2461692,
            z: 162.4961335
        }],
        [{
            x: 374.3763739,
            y: 377.2461692,
            z: 162.4961335
        }, {
            x: 364.6530442,
            y: 366.602019,
            z: 186.9650425
        }, {
            x: 350.3543803,
            y: 343.8192307,
            z: 207.4499708
        }, {
            x: 345.8110872,
            y: 334.6861852,
            z: 234.150361
        }],
        [{
            x: 345.8110872,
            y: 334.6861852,
            z: 234.150361
        }, {
            x: 342.2688222,
            y: 327.5654324,
            z: 254.9678288
        }, {
            x: 344.6566892,
            y: 328.7421373,
            z: 279.5635753
        }, {
            x: 354.2083553,
            y: 333.4490545,
            z: 298.2620383
        }]
    ],
    [
        [{
            x: -293.9601717,
            y: 0.5258114405,
            z: -800.1995972
        }, {
            x: -278.3619072,
            y: 0.1877518356,
            z: -801.8622391
        }, {
            x: -262.3491231,
            y: 0.4647837051,
            z: -800.4997433
        }, {
            x: -248.5897753,
            y: -1.893611292,
            z: -799.2844921
        }],
        [{
            x: -248.5897753,
            y: -1.893611292,
            z: -799.2844921
        }, {
            x: -238.1542263,
            y: -3.682296854,
            z: -798.362805
        }, {
            x: -229.0149017,
            y: -6.986935924,
            z: -797.5258161
        }, {
            x: -217.5505549,
            y: -9.386634419,
            z: -795.2785941
        }],
        [{
            x: -217.5505549,
            y: -9.386634419,
            z: -795.2785941
        }, {
            x: -197.7958159,
            y: -13.52166433,
            z: -791.4063034
        }, {
            x: -171.1375491,
            y: -14.96971684,
            z: -783.3467051
        }, {
            x: -151.6504377,
            y: -6.886341854,
            z: -774.0071185
        }],
        [{
            x: -151.6504377,
            y: -6.886341854,
            z: -774.0071185
        }, {
            x: -144.9967859,
            y: -4.126365695,
            z: -770.8182232
        }, {
            x: -139.1791496,
            y: -0.2552127546,
            z: -767.4801065
        }, {
            x: -133.5008152,
            y: 3.970965835,
            z: -764.0129312
        }],
        [{
            x: -133.5008152,
            y: 3.970965835,
            z: -764.0129312
        }, {
            x: -120.2784355,
            y: 13.81190397,
            z: -755.9393825
        }, {
            x: -107.8113814,
            y: 25.57787008,
            z: -747.1660492
        }, {
            x: -97.01248531,
            y: 35.76559341,
            z: -736.5913057
        }],
        [{
            x: -97.01248531,
            y: 35.76559341,
            z: -736.5913057
        }, {
            x: -69.59366212,
            y: 61.63262481,
            z: -709.7416147
        }, {
            x: -52.92898837,
            y: 77.32516488,
            z: -671.2787343
        }, {
            x: -34.63544126,
            y: 85.29693925,
            z: -634.3005179
        }],
        [{
            x: -34.63544126,
            y: 85.29693925,
            z: -634.3005179
        }, {
            x: -17.22985899,
            y: 92.8817654,
            z: -599.1172162
        }, {
            x: 1.650304106,
            y: 93.47716347,
            z: -565.2779459
        }, {
            x: 20.92120706,
            y: 90.76434197,
            z: -528.335302
        }],
        [{
            x: 20.92120706,
            y: 90.76434197,
            z: -528.335302
        }, {
            x: 39.76100031,
            y: 88.11220905,
            z: -492.2191027
        }, {
            x: 58.97424646,
            y: 82.2982174,
            z: -453.1368279
        }, {
            x: 85.26558818,
            y: 74.05794852,
            z: -420.3042166
        }],
        [{
            x: 85.26558818,
            y: 74.05794852,
            z: -420.3042166
        }, {
            x: 112.2057253,
            y: 65.6143333,
            z: -386.66139
        }, {
            x: 146.5776027,
            y: 54.62321613,
            z: -359.5804805
        }, {
            x: 168.7351053,
            y: 56.62689171,
            z: -326.9364442
        }],
        [{
            x: 168.7351053,
            y: 56.62689171,
            z: -326.9364442
        }, {
            x: 177.3894252,
            y: 57.40949116,
            z: -314.1862739
        }, {
            x: 184.1803904,
            y: 60.17450126,
            z: -300.587425
        }, {
            x: 191.360848,
            y: 62.90323486,
            z: -287.1669909
        }],
        [{
            x: 191.360848,
            y: 62.90323486,
            z: -287.1669909
        }, {
            x: 201.8026231,
            y: 66.87134168,
            z: -267.6510822
        }, {
            x: 213.0680495,
            y: 70.76273536,
            z: -248.5124633
        }, {
            x: 222.6718815,
            y: 74.75679843,
            z: -228.8688967
        }],
        [{
            x: 222.6718815,
            y: 74.75679843,
            z: -228.8688967
        }, {
            x: 228.4867172,
            y: 77.17508525,
            z: -216.9752993
        }, {
            x: 233.6924218,
            y: 79.63101009,
            z: -204.8965909
        }, {
            x: 238.3273983,
            y: 87.05810798,
            z: -194.4564719
        }],
        [{
            x: 238.3273983,
            y: 87.05810798,
            z: -194.4564719
        }, {
            x: 245.50256,
            y: 98.55560504,
            z: -178.2946762
        }, {
            x: 251.3100007,
            y: 121.9662683,
            z: -166.0596774
        }, {
            x: 266.5073285,
            y: 129.7436545,
            z: -154.9546257
        }],
        [{
            x: 266.5073285,
            y: 129.7436545,
            z: -154.9546257
        }, {
            x: 271.952874,
            y: 132.5304675,
            z: -150.9754351
        }, {
            x: 278.6040346,
            y: 133.3100449,
            z: -147.1413242
        }, {
            x: 285.2939486,
            y: 134.0343696,
            z: -143.5789567
        }],
        [{
            x: 285.2939486,
            y: 134.0343696,
            z: -143.5789567
        }, {
            x: 297.7406741,
            y: 135.3819909,
            z: -136.9510973
        }, {
            x: 310.3215459,
            y: 136.5383527,
            z: -131.2638885
        }, {
            x: 322.8671888,
            y: 137.7776002,
            z: -125.1690322
        }]
    ],
    [
        [{
            x: -306.4845851,
            y: 2.17278011,
            z: -803.2854717
        }, {
            x: -273.6735719,
            y: -1.648953767,
            z: -782.8786198
        }, {
            x: -243.0821386,
            y: -6.079451662,
            z: -759.2211604
        }, {
            x: -212.5514845,
            y: -10.50721519,
            z: -735.5783016
        }],
        [{
            x: -212.5514845,
            y: -10.50721519,
            z: -735.5783016
        }, {
            x: -200.2414486,
            y: -12.2925005,
            z: -726.0454418
        }, {
            x: -187.9412936,
            y: -14.07734128,
            z: -716.5149558
        }, {
            x: -174.9782443,
            y: -15.69448599,
            z: -707.8799138
        }],
        [{
            x: -174.9782443,
            y: -15.69448599,
            z: -707.8799138
        }, {
            x: -162.5145272,
            y: -17.24933884,
            z: -699.5774907
        }, {
            x: -149.4380011,
            y: -18.64916598,
            z: -692.1028558
        }, {
            x: -137.4050041,
            y: -21.2174579,
            z: -681.8910278
        }],
        [{
            x: -137.4050041,
            y: -21.2174579,
            z: -681.8910278
        }, {
            x: -115.5682829,
            y: -25.87823147,
            z: -663.359249
        }, {
            x: -97.16818405,
            y: -34.38707414,
            z: -635.8131561
        }, {
            x: -81.04514372,
            y: -41.42663513,
            z: -609.5292453
        }],
        [{
            x: -81.04514372,
            y: -41.42663513,
            z: -609.5292453
        }, {
            x: -53.358052,
            y: -53.51523421,
            z: -564.3935244
        }, {
            x: -32.38578991,
            y: -61.271061,
            z: -522.9798589
        }, {
            x: -5.898663245,
            y: -62.42481717,
            z: -484.4546557
        }],
        [{
            x: -5.898663245,
            y: -62.42481717,
            z: -484.4546557
        }, {
            x: 26.5145326,
            y: -63.83670775,
            z: -437.310057
        }, {
            x: 67.18637465,
            y: -55.36183416,
            z: -394.491002
        }, {
            x: 97.4277474,
            y: -57.24620837,
            z: -346.264079
        }],
        [{
            x: 97.4277474,
            y: -57.24620837,
            z: -346.264079
        }, {
            x: 120.5538649,
            y: -58.68722296,
            z: -309.3840902
        }, {
            x: 137.580313,
            y: -66.18625718,
            z: -269.3416159
        }, {
            x: 163.5402471,
            y: -84.83050389,
            z: -240.4524702
        }],
        [{
            x: 163.5402471,
            y: -84.83050389,
            z: -240.4524702
        }, {
            x: 191.1224344,
            y: -104.6398417,
            z: -209.758023
        }, {
            x: 228.7895125,
            y: -137.030858,
            z: -191.6544165
        }, {
            x: 265.6996504,
            y: -142.2794607,
            z: -163.6285356
        }]
    ],
    [
        [{
            x: -319.0089985,
            y: 7.124478677,
            z: -782.6542179
        }, {
            x: -312.8893248,
            y: 6.622603414,
            z: -783.8842373
        }, {
            x: -306.1609208,
            y: 6.591682975,
            z: -783.9600186
        }, {
            x: -300.2223784,
            y: 7.124478677,
            z: -782.6542179
        }],
        [{
            x: -300.2223784,
            y: 7.124478677,
            z: -782.6542179
        }, {
            x: -278.5153113,
            y: 9.07199901,
            z: -777.8811435
        }, {
            x: -267.361673,
            y: 18.55139335,
            z: -754.6485983
        }, {
            x: -253.2558281,
            y: 24.37708443,
            z: -734.2608884
        }],
        [{
            x: -253.2558281,
            y: 24.37708443,
            z: -734.2608884
        }, {
            x: -238.185846,
            y: 30.60096264,
            z: -712.4796744
        }, {
            x: -219.7462976,
            y: 32.65460573,
            z: -693.9454761
        }, {
            x: -206.2892778,
            y: 35.84948209,
            z: -670.8916594
        }],
        [{
            x: -206.2892778,
            y: 35.84948209,
            z: -670.8916594
        }, {
            x: -198.5838107,
            y: 37.67886296,
            z: -657.6910823
        }, {
            x: -192.5119596,
            y: 39.88241868,
            z: -643.0086632
        }, {
            x: -187.5026577,
            y: 43.30752041,
            z: -629.2120765
        }],
        [{
            x: -187.5026577,
            y: 43.30752041,
            z: -629.2120765
        }, {
            x: -180.5275059,
            y: 48.07676874,
            z: -610.0011588
        }, {
            x: -175.6125191,
            y: 55.21445906,
            z: -592.5077719
        }, {
            x: -171.847141,
            y: 62.41961411,
            z: -574.8490392
        }],
        [{
            x: -171.847141,
            y: 62.41961411,
            z: -574.8490392
        }, {
            x: -168.1690878,
            y: 69.45767038,
            z: -557.5998401
        }, {
            x: -165.5879393,
            y: 76.56009843,
            z: -540.1928756
        }, {
            x: -162.4538309,
            y: 86.47339167,
            z: -525.6787644
        }],
        [{
            x: -162.4538309,
            y: 86.47339167,
            z: -525.6787644
        }, {
            x: -159.3433408,
            y: 96.3119793,
            z: -511.2740301
        }, {
            x: -155.6881936,
            y: 108.9192269,
            z: -499.7187128
        }, {
            x: -156.1916242,
            y: 123.5384416,
            z: -490.0353723
        }],
        [{
            x: -156.1916242,
            y: 123.5384416,
            z: -490.0353723
        }, {
            x: -157.0622614,
            y: 148.8210353,
            z: -473.2889212
        }, {
            x: -170.3705963,
            y: 180.1211278,
            z: -462.1412787
        }, {
            x: -184.3715544,
            y: 204.2092603,
            z: -443.6422367
        }],
        [{
            x: -184.3715544,
            y: 204.2092603,
            z: -443.6422367
        }, {
            x: -199.7660546,
            y: 230.6949308,
            z: -423.3019495
        }, {
            x: -215.9979156,
            y: 248.4615547,
            z: -394.0740375
        }, {
            x: -231.3381047,
            y: 261.2875613,
            z: -360.9584407
        }],
        [{
            x: -231.3381047,
            y: 261.2875613,
            z: -360.9584407
        }, {
            x: -245.901989,
            y: 273.4644956,
            z: -329.51869
        }, {
            x: -259.6621657,
            y: 281.1882091,
            z: -294.5747781
        }, {
            x: -272.0424482,
            y: 278.3667478,
            z: -255.2076367
        }],
        [{
            x: -272.0424482,
            y: 278.3667478,
            z: -255.2076367
        }, {
            x: -285.5736744,
            y: 275.2829869,
            z: -212.1806952
        }, {
            x: -297.4565142,
            y: 259.6022305,
            z: -163.8698772
        }, {
            x: -306.4845851,
            y: 273.2733801,
            z: -130.3639813
        }]
    ],
    [
        [{
            x: -295.2377273,
            y: 3.340714387,
            z: -816.1317067
        }, {
            x: -233.1379669,
            y: -14.12761104,
            z: -811.2722671
        }, {
            x: -172.472437,
            y: -33.06686077,
            z: -796.4557885
        }, {
            x: -111.8119289,
            y: -46.09773227,
            z: -778.3509202
        }],
        [{
            x: -111.8119289,
            y: -46.09773227,
            z: -778.3509202
        }, {
            x: -47.81690409,
            y: -59.84491262,
            z: -759.2508247
        }, {
            x: 16.1725316,
            y: -67.01629254,
            z: -736.4908763
        }, {
            x: 81.03045695,
            y: -80.41296811,
            z: -722.4165071
        }],
        [{
            x: 81.03045695,
            y: -80.41296811,
            z: -722.4165071
        }, {
            x: 136.6180888,
            y: -91.8948258,
            z: -710.3538202
        }, {
            x: 192.8436827,
            y: -107.9495679,
            z: -704.6712559
        }, {
            x: 263.9853237,
            y: -121.2626024,
            z: -657.9558155
        }],
        [{
            x: 263.9853237,
            y: -121.2626024,
            z: -657.9558155
        }, {
            x: 340.3590879,
            y: -135.554746,
            z: -607.8046807
        }, {
            x: 433.9235843,
            y: -146.6870739,
            z: -510.3631889
        }, {
            x: 488.7823103,
            y: -165.7633511,
            z: -486.4900794
        }]
    ],
    [
        [{
            x: -306.4845851,
            y: 3.428255598,
            z: -802.9024005
        }, {
            x: -253.4910264,
            y: -3.669824345,
            z: -786.4435597
        }, {
            x: -207.397058,
            y: -18.83758905,
            z: -751.2729464
        }, {
            x: -171.847141,
            y: -36.24967413,
            z: -710.8982621
        }],
        [{
            x: -171.847141,
            y: -36.24967413,
            z: -710.8982621
        }, {
            x: -151.795779,
            y: -46.07068215,
            z: -688.1255673
        }, {
            x: -135.098842,
            y: -56.60568558,
            z: -663.6972788
        }, {
            x: -118.618384,
            y: -72.69002037,
            z: -640.3805198
        }],
        [{
            x: -118.618384,
            y: -72.69002037,
            z: -640.3805198
        }, {
            x: -103.1902842,
            y: -87.74729129,
            z: -618.5526505
        }, {
            x: -87.95189965,
            y: -107.6678165,
            z: -597.6988896
        }, {
            x: -71.65183366,
            y: -127.136263,
            z: -579.9248997
        }],
        [{
            x: -71.65183366,
            y: -127.136263,
            z: -579.9248997
        }, {
            x: -55.03934231,
            y: -146.9778636,
            z: -561.8102336
        }, {
            x: -37.32408075,
            y: -166.3498893,
            z: -546.8945306
        }, {
            x: -21.55418001,
            y: -181.782441,
            z: -527.050566
        }],
        [{
            x: -21.55418001,
            y: -181.782441,
            z: -527.050566
        }, {
            x: -3.678764356,
            y: -199.2754665,
            z: -504.5571387
        }, {
            x: 11.69714322,
            y: -211.7068362,
            z: -475.7316062
        }, {
            x: 19.15016358,
            y: -224.8685002,
            z: -445.2126862
        }],
        [{
            x: 19.15016358,
            y: -224.8685002,
            z: -445.2126862
        }, {
            x: 23.27855034,
            y: -232.1590263,
            z: -428.3076063
        }, {
            x: 24.97596703,
            y: -239.6736278,
            z: -410.8829463
        }, {
            x: 25.41237028,
            y: -246.5520834,
            z: -389.6488446
        }],
        [{
            x: 25.41237028,
            y: -246.5520834,
            z: -389.6488446
        }, {
            x: 25.8467732,
            y: -253.3990104,
            z: -368.5120732
        }, {
            x: 25.03169634,
            y: -259.6156099,
            z: -343.6007025
        }, {
            x: 22.28126693,
            y: -262.3869987,
            z: -322.7234187
        }],
        [{
            x: 22.28126693,
            y: -262.3869987,
            z: -322.7234187
        }, {
            x: 19.49719188,
            y: -265.1922896,
            z: -301.5907459
        }, {
            x: 14.73012483,
            y: -264.4675646,
            z: -284.5914606
        }, {
            x: 12.88795687,
            y: -263.0637651,
            z: -266.026868
        }],
        [{
            x: 12.88795687,
            y: -263.0637651,
            z: -266.026868
        }, {
            x: 10.45675454,
            y: -261.2110998,
            z: -241.5262352
        }, {
            x: 13.11997106,
            y: -258.1756621,
            z: -214.2992413
        }, {
            x: 6.625750167,
            y: -268.5377729,
            z: -180.2086315
        }],
        [{
            x: 6.625750167,
            y: -268.5377729,
            z: -180.2086315
        }, {
            x: 3.214818247,
            y: -273.9802201,
            z: -162.303366
        }, {
            x: -2.722303304,
            y: -283.1185428,
            z: -142.5046892
        }, {
            x: -5.898663245,
            y: -293.4120583,
            z: -126.7755862
        }],
        [{
            x: -5.898663245,
            y: -293.4120583,
            z: -126.7755862
        }, {
            x: -10.52896375,
            y: -308.4173067,
            z: -103.8466756
        }, {
            x: -9.292645829,
            y: -325.8773398,
            z: -89.56560938
        }, {
            x: -18.42307666,
            y: -336.6294716,
            z: -76.04028429
        }],
        [{
            x: -18.42307666,
            y: -336.6294716,
            z: -76.04028429
        }, {
            x: -25.46072394,
            y: -344.9171102,
            z: -65.61509525
        }, {
            x: -38.6574396,
            y: -349.2194662,
            z: -55.63890534
        }, {
            x: -52.86521354,
            y: -350.9079652,
            z: -51.72365831
        }]
    ],
    [
        [{
            x: -75.44014617,
            y: 56.12180437,
            z: 416.8398794
        }, {
            x: -77.64486699,
            y: 57.23297649,
            z: 478.8584464
        }, {
            x: -83.46309386,
            y: 42.32185075,
            z: 553.7644775
        }, {
            x: -79.95049883,
            y: 53.80520879,
            z: 639.00092
        }],
        [{
            x: -79.95049883,
            y: 53.80520879,
            z: 639.00092
        }, {
            x: -76.43790379,
            y: 65.28856682,
            z: 724.2373625
        }, {
            x: -47.48423511,
            y: 136.9964768,
            z: 816.0936638
        }, {
            x: -10.05429954,
            y: 156.4687042,
            z: 881.8584508
        }],
        [{
            x: -10.05429954,
            y: 156.4687042,
            z: 881.8584508
        }, {
            x: 10.05166703,
            y: 166.9284591,
            z: 917.1848437
        }, {
            x: 49.54753525,
            y: 184.3514501,
            z: 1009.490056
        }, {
            x: 83.42134712,
            y: 200.2696054,
            z: 1106.824032
        }],
        [{
            x: 83.42134712,
            y: 200.2696054,
            z: 1106.824032
        }, {
            x: 112.6081492,
            y: 213.9852171,
            z: 1190.69022
        }, {
            x: 137.6210702,
            y: 226.5836209,
            z: 1278.28982
        }, {
            x: 142.4602929,
            y: 232.6479039,
            z: 1336.391073
        }]
    ],
    [
        [{
            x: 181.1310389,
            y: 182.4028384,
            z: 506.3169496
        }, {
            x: 256.1469397,
            y: 181.1159826,
            z: 621.5966174
        }, {
            x: 246.0581264,
            y: 127.3485384,
            z: 686.2212735
        }, {
            x: 239.3139456,
            y: 101.6389268,
            z: 752.4468683
        }],
        [{
            x: 239.3139456,
            y: 101.6389268,
            z: 752.4468683
        }, {
            x: 232.5697648,
            y: 75.92931509,
            z: 818.6724631
        }, {
            x: 167.4629908,
            y: 62.9449301,
            z: 868.6595572
        }, {
            x: 153.6295618,
            y: 90.99936589,
            z: 940.1899851
        }],
        [{
            x: 153.6295618,
            y: 90.99936589,
            z: 940.1899851
        }, {
            x: 139.7961327,
            y: 119.0538017,
            z: 1011.720413
        }, {
            x: 141.1078059,
            y: 149.9898688,
            z: 1046.280417
        }, {
            x: 173.6899256,
            y: 173.307857,
            z: 1148.796377
        }],
        [{
            x: 173.6899256,
            y: 173.307857,
            z: 1148.796377
        }, {
            x: 206.2720453,
            y: 196.6258451,
            z: 1251.312338
        }, {
            x: 271.1873259,
            y: 195.7884074,
            z: 1266.459424
        }, {
            x: 279.2307724,
            y: 165.9507466,
            z: 1383.43306
        }]
    ]
];

function assignVariables() {
    cellLayout = "Object Name,TranslateX,TranslateY,TranslateZ,DimensionX,DimensionY,DimensionZ, RotateX, RotateY, RotateZ;NPC,197.8817968,187.1750856,343.3718791,24.13382446,34.22486206,30.38697693,46.23119528,-8.353895581,8.869973765;NPC,213.3709151,2.456316462,423.9885128,25.53621185,34.58618774,24.07834717,90.38281992,-7.832244708,0;NPC,33.34912856,2.090308135,365.04642,25.53621185,34.58618774,24.07834717,90.39885078,-38.49088523,0;NPC,397.5101386,159.5911591,319.6113615,25.53621185,34.58618774,24.07834717,50.3605842,37.83254121,0;NPC,156.5827125,-122.5087526,377.9210147,25.53621185,34.58618774,24.07834717,115.9481277,-11.66518146,0;golgi,-318.7046145,66.9773569,976.6478796,211.1744971,156.6775269,204.0570996,115.0039041,-35.67118658,-77.29622127;golgi,456.0077009,0,-837.4997205,235.0718115,174.4077612,227.1489771,-25.72753579,12.39046363,101.1906405;microtubule1,212.4149854,132.4649414,944.2434082,142.5869824,122.8758984,880.6406836,0,0,0;microtubule10,673.8832031,311.0911194,-148.9548248,200.3548828,452.0626831,200.2862878,0,0,0;microtubule11,732.2112598,361.0221094,-546.264873,89.10005859,364.5919922,488.656582,0,0,0;microtubule12,739.5973828,340.5306592,-314.7477979,71.43855469,381.5696191,163.5716895,0,0,0;microtubule13,547.683501,315.5453137,-194.0386853,450.7195605,452.5557788,230.4621606,0,0,0;microtubule14,556.663916,415.2724512,-436.4479395,283.920293,207.0583008,657.0294727,0,0,0;microtubule15,660.601377,201.7332788,-572.2047363,508.4347852,224.4388916,408.4554492,0,0,0;microtubule16,506.7585205,204.8784119,-527.3789355,154.4813965,242.6824146,450.410918,0,0,0;microtubule17,212.1601877,58.27322021,-728.5755762,367.6139996,489.4616455,107.694668,0,0,0;microtubule18,398.5259912,137.8724231,-665.1991992,112.6322754,371.4562866,196.6646484,0,0,0;microtubule19,380.7953174,226.3009131,-449.3071655,81.91825195,156.3450879,654.2884424,0,0,0;microtubule2,30.6969873,141.0839227,876.8226709,232.4791699,192.0579456,920.3092676,0,0,0;microtubule20,-17.70177979,102.1328238,-793.8511963,249.8578271,254.9617255,626.1412793,0,0,0;microtubule21,77.27729828,414.6652734,-875.5563281,112.0010284,370.8857812,414.7598437,0,0,0;microtubule22,206.3877283,127.7296783,-799.063623,248.5948755,233.9102527,596.1479883,0,0,0;microtubule23,26.83012939,306.4021143,-869.8149609,151.6807324,229.8870996,438.0902344,0,0,0;microtubule24,130.2055444,10.68804199,-875.882373,102.7668896,337.1855273,478.814707,0,0,0;microtubule25,466.3101562,323.6335986,125.3250128,363.4645312,146.882959,368.3232751,0,0,0;microtubule26,486.9932666,242.030116,363.5398682,289.2078809,287.3686157,132.0589746,0,0,0;microtubule27,577.1186426,218.112627,122.2437305,136.8993164,381.0901172,311.8405664,0,0,0;microtubule28,477.9364307,294.2746069,284.5083691,329.104834,203.73854,36.79857422,0,0,0;microtubule29,602.5964648,317.1294214,80.40759521,130.0769531,205.3262549,366.1346924,0,0,0;microtubule3,-139.0224133,-173.8191055,-427.1847363,337.7971655,363.020246,759.5431055,0,0,0;microtubule30,559.5994629,-337.2936108,210.2427393,166.3097461,468.162583,203.6533301,0,0,0;microtubule31,419.9002661,-500.184126,316.3121631,407.9912256,122.8493262,84.17920898,0,0,0;microtubule32,532.1963672,-447.1643115,104.0702747,257.4648047,199.2719238,389.7403491,0,0,0;microtubule33,482.8850098,-448.0571045,311.7472119,325.9098633,212.6162988,44.34436523,0,0,0;microtubule34,621.9338965,-416.982041,118.7065613,126.4250977,271.4240039,329.2003931,0,0,0;microtubule35,-118.3220728,110.4977856,955.7098828,622.6587451,493.0616748,154.8304687,0,0,0;microtubule36,-38.73700195,295.7910132,735.6735938,813.0149414,130.0887158,432.2794922,0,0,0;microtubule37,-147.0284253,183.0389575,716.1807568,518.7239502,341.7746631,490.7852051,0,0,0;microtubule38,-505.4994727,260.7653027,570.3792847,172.7321484,194.3118164,730.0565479,0,0,0;microtubule39,-330.203606,255.0918164,658.6816846,226.97896,194.4717773,593.7860449,0,0,0;microtubule4,97.25859375,-81.19509682,-651.4826221,787.3532813,177.7109001,338.228584,0,0,0;microtubule40,-200.9843848,60.38698975,720.7139062,519.6625781,586.7271533,430.7780859,0,0,0;microtubule41,287.1684814,114.7601161,777.7985156,734.9856152,206.0788889,468.8053125,0,0,0;microtubule42,589.3171875,134.296333,583.4237842,159.3030469,180.6350391,838.2054785,0,0,0;microtubule43,445.5135352,-95.13089355,938.1663281,392.8723828,599.7641895,169.9720312,0,0,0;microtubule44,428.9714355,45.15916992,676.3465869,449.8033008,347.0653711,673.1271387,0,0,0;microtubule45,505.1379199,-83.39869629,677.9993848,336.0916992,599.8922168,643.4302148,0,0,0;microtubule46,-279.8748102,-401.6762476,540.5428711,625.014364,334.9703174,104.5120312,0,0,0;microtubule47,-522.0736963,-360.1754443,303.4938977,123.0875684,374.5616895,402.6039624,0,0,0;microtubule48,-264.077915,-531.3914502,398.5253467,593.5828418,83.4909668,304.2609082,0,0,0;microtubule49,-462.3454541,-368.403479,406.0618945,204.1284668,400.7098389,246.7103906,0,0,0;microtubule5,-235.8658887,142.852633,-458.1761499,168.1641211,281.0347536,659.5777002,0,0,0;microtubule50,-270.6177164,-552.1942676,517.5230713,567.2812079,107.8213477,107.9428418,0,0,0;microtubule51,-309.9686316,345.4123975,-270.0235071,393.3619556,196.0721777,612.4700171,0,0,0;microtubule52,-461.5955127,164.0074951,-133.1996063,94.17870117,568.1300684,386.5102991,0,0,0;microtubule53,-349.3235669,233.7860156,-66.56753998,315.557124,399.5180273,164.2730255,0,0,0;microtubule54,-363.6500024,351.6772412,320.7801343,276.5947998,201.2884863,511.4768408,0,0,0;microtubule55,-393.347124,280.0743311,102.1417091,227.8053223,299.9349316,204.8680662,0,0,0;microtubule56,-227.535769,288.7856909,-53.67977051,539.6171338,325.6723096,250.9921875,0,0,0;microtubule6,-20.18329102,-70.09014084,-483.5474414,577.0610156,153.2483414,647.0252344,0,0,0;microtubule7,15.20736328,62.57915131,-463.1074878,619.3040039,159.3178107,683.8982666,0,0,0;microtubule8,557.853501,434.7157031,14.36625,435.7967871,218.8402734,572.1272461,0,0,0;microtubule9,790.0964648,302.6857306,-46.75898437,106.6875,484.5883044,487.1542383,0,0,0;mitochondria1,509.9553464,60.67585787,598.6037478,56.38573425,55.78621948,97.22390625,18.53970718,0,0;mitochondria2,236.1862652,8.347496228,771.7170695,69.44670044,68.70831848,160.0913306,0,-58.66119162,0;mitochondria3,-237.8879342,-80.8010362,794.9918666,74.3063324,72.65880615,278.4640576,12.50616674,-13.37802957,-34.78928216;mitochondria4,-480.3502638,0,623.5202907,56.38573425,55.78621948,97.22390625,0,0,0;mitochondria5,654.307804,246.0744739,370.0455458,69.44670044,68.70831848,160.0913306,-55.07883972,0,0;mitochondria6,0,-464.9341727,0,69.44670044,68.70831848,160.0913306,-25.64558815,-58.48676953,-21.37185167;mitochondria7,0,0,-581.7224431,69.44670044,68.70831848,160.0913306,4.948279917,-70.48661547,121.4859061;mitochondria8,624.0007,49.31601383,-87.21635338,69.44670044,68.70831848,160.0913306,89.76656115,-5.023110397,-48.14947773;nucleus,208.5936936,6.113100222,153.3202277,522.7149005,515.7176939,518.1826595,0,0,0;rough_ER1,253.871543,-69.16218018,184.1513013,624.6314062,553.1499756,621.0511084,0,0,0;rough_ER2,221.76854,64.61654297,146.9922729,621.8760059,558.2816016,585.9084229,0,0,0;smoothER,-132.954492,-89.25859014,80.62120204,376.5313623,386.0319287,318.3009668,122.954661,-71.58529221,-50.75833529;smoothER,-139.2748633,-37.55435675,275.4559992,350.3096777,359.3174121,299.3762402,-12.50517141,-27.97002065,5.954923214;smoothER,243.6573639,10.98815226,-278.1663356,402.7864746,412.9494434,340.4957227,0,0,0;";
}

Script.setTimeout(function(){

for (var i = 0; i < scenes.length; i++) {
    // print('setting up scene.  first, delete' + JSON.stringify(scenes[i]))

    CreateNavigationButton(scenes[i], i);

    ImportScene(scenes[i]);
    // print('setting up scene.  then import')


}

createLayoutLights();
},3500)


