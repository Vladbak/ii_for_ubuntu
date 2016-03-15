//
//  createTank.js
//  
//
//  created by James b. Pollack @imgntn on 3/9/2016
//  Copyright 2016 High Fidelity, Inc.   
//  
//  Adds a fish tank and base, decorations, particle bubble systems, and a bubble sound.  Attaches a script that does fish swimming.  
// 
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


var fishTank, tankBase, bubbleSystem, secondBubbleSystem, thirdBubbleSystem, innerContainer, bubbleInjector, lowerCorner, upperCorner, urchin, treasure, rocks;
var CLEANUP = true;

var TANK_DIMENSIONS = {
    x: 0.8212,
    y: 0.8116,
    z: 2.1404
};


var INNER_TANK_SCALE = 0.7;
var INNER_TANK_DIMENSIONS = Vec3.multiply(INNER_TANK_SCALE, TANK_DIMENSIONS);
INNER_TANK_DIMENSIONS.y = INNER_TANK_DIMENSIONS.y - 0.4;
var TANK_WIDTH = TANK_DIMENSIONS.z;
var TANK_HEIGHT = TANK_DIMENSIONS.y;

var DEBUG_COLOR = {
    red: 255,
    green: 0,
    blue: 255
}

var center = Vec3.sum(MyAvatar.position, Vec3.multiply(Quat.getFront(MyAvatar.orientation), 1 * TANK_WIDTH));

var TANK_POSITION = center;

var TANK_SCRIPT = Script.resolvePath('tank.js?' + Math.random())

var TANK_MODEL_URL = "http://hifi-content.s3.amazonaws.com/DomainContent/Home/fishTank/aquariumTank.fbx";

var TANK_BASE_MODEL_URL = 'http://hifi-content.s3.amazonaws.com/DomainContent/Home/fishTank/aquariumBase.fbx';

var TANK_BASE_COLLISION_HULL = 'http://hifi-content.s3.amazonaws.com/DomainContent/Home/fishTank/aquariumBase.obj'

var TANK_BASE_DIMENSIONS = {
    x: 0.8599,
    y: 1.8450,
    z: 2.1936
};

var BASE_VERTICAL_OFFSET = 0.42;

var BUBBLE_SYSTEM_FORWARD_OFFSET = TANK_DIMENSIONS.x + 0.06;
var BUBBLE_SYSTEM_LATERAL_OFFSET = 0.025;
var BUBBLE_SYSTEM_VERTICAL_OFFSET = -0.30;

var BUBBLE_SYSTEM_DIMENSIONS = {
    x: TANK_DIMENSIONS.x / 8,
    y: TANK_DIMENSIONS.y,
    z: TANK_DIMENSIONS.z / 8
}

var BUBBLE_SOUND_URL = "http://hifi-content.s3.amazonaws.com/DomainContent/Home/Sounds/aquarium_small.L.wav";
var bubbleSound = SoundCache.getSound(BUBBLE_SOUND_URL);


var URCHIN_FORWARD_OFFSET = TANK_DIMENSIONS.x - 0.35;
var URCHIN_LATERAL_OFFSET = -0.05;
var URCHIN_VERTICAL_OFFSET = -0.12;


var URCHIN_MODEL_URL = 'http://hifi-content.s3.amazonaws.com/DomainContent/Home/fishTank/Urchin.fbx';

var URCHIN_DIMENSIONS = {
    x: 0.4,
    y: 0.4,
    z: 0.4
}

var ROCKS_FORWARD_OFFSET = 0;
var ROCKS_LATERAL_OFFSET = 0.0;
var ROCKS_VERTICAL_OFFSET = (-TANK_DIMENSIONS.y / 2) + 0.25;

var ROCK_MODEL_URL = 'http://hifi-content.s3.amazonaws.com/DomainContent/Home/fishTank/Aquarium-Rocks-2.fbx';

var ROCK_DIMENSIONS = {
    x: 0.707,
    y: 0.33,
    z: 1.64
}

var TREASURE_FORWARD_OFFSET = -TANK_DIMENSIONS.x;
var TREASURE_LATERAL_OFFSET = -0.15;
var TREASURE_VERTICAL_OFFSET = -0.23;

var TREASURE_MODEL_URL = 'http://hifi-content.s3.amazonaws.com/DomainContent/Home/fishTank/Treasure-Chest2-SM.fbx';

var TREASURE_DIMENSIONS = {
    x: 0.1199,
    y: 0.1105,
    z: 0.1020
}


function createFishTank() {
    var tankProperties = {
        name: 'hifi-home-fishtank',
        type: 'Model',
        modelURL: TANK_MODEL_URL,
        dimensions: TANK_DIMENSIONS,
        position: TANK_POSITION,
        color: DEBUG_COLOR,
        collisionless: true,
        script: TANK_SCRIPT,
        visible: true
    }

    fishTank = Entities.addEntity(tankProperties);
}

function createBubbleSystems() {

    var tankProperties = Entities.getEntityProperties(fishTank);
    var bubbleProperties = {
        "name": 'hifi-home-fishtank-bubbles',
        "isEmitting": 1,
        "maxParticles": 1880,
        "lifespan": 1.6,
        "emitRate": 10,
        "emitSpeed": 0.025,
        "speedSpread": 0.025,
        "emitOrientation": {
            "x": 0,
            "y": 0.5,
            "z": 0.5,
            "w": 0
        },
        "emitDimensions": {
            "x": -0.2,
            "y": TANK_DIMENSIONS.y,
            "z": 0
        },
        "polarStart": 0,
        "polarFinish": 0,
        "azimuthStart": 0.2,
        "azimuthFinish": 0.1,
        "emitAcceleration": {
            "x": 0,
            "y": 0.3,
            "z": 0
        },
        "accelerationSpread": {
            "x": 0.01,
            "y": 0.01,
            "z": 0.01
        },
        "particleRadius": 0.005,
        "radiusSpread": 0,
        "radiusStart": 0.01,
        "radiusFinish": 0.01,
        "alpha": 0.2,
        "alphaSpread": 0,
        "alphaStart": 0.3,
        "alphaFinish": 0,
        "emitterShouldTrail": 0,
        "textures": "http://hifi-content.s3.amazonaws.com/DomainContent/Home/fishTank/bubble-white.png"
    };

    bubbleProperties.type = "ParticleEffect";
    bubbleProperties.parentID = fishTank;
    bubbleProperties.dimensions = BUBBLE_SYSTEM_DIMENSIONS;

    var finalOffset = getOffsetFromTankCenter(BUBBLE_SYSTEM_VERTICAL_OFFSET, BUBBLE_SYSTEM_FORWARD_OFFSET, BUBBLE_SYSTEM_LATERAL_OFFSET);

    bubbleProperties.position = finalOffset;
    bubbleSystem = Entities.addEntity(bubbleProperties);

    bubbleProperties.position.x += -0.076;
    secondBubbleSystem = Entities.addEntity(bubbleProperties)

    bubbleProperties.position.x += -0.076;
    thirdBubbleSystem = Entities.addEntity(bubbleProperties)

    createBubbleSound(finalOffset);
}

function getOffsetFromTankCenter(VERTICAL_OFFSET, FORWARD_OFFSET, LATERAL_OFFSET) {

    var tankProperties = Entities.getEntityProperties(fishTank);

    var upVector = Quat.getUp(tankProperties.rotation);
    var frontVector = Quat.getFront(tankProperties.rotation);
    var rightVector = Quat.getRight(tankProperties.rotation);

    var upOffset = Vec3.multiply(upVector, VERTICAL_OFFSET);
    var frontOffset = Vec3.multiply(frontVector, FORWARD_OFFSET);
    var rightOffset = Vec3.multiply(rightVector, LATERAL_OFFSET);

    var finalOffset = Vec3.sum(tankProperties.position, upOffset);
    finalOffset = Vec3.sum(finalOffset, frontOffset);
    finalOffset = Vec3.sum(finalOffset, rightOffset);
    return finalOffset
}

function createBubbleSound(position) {
    var audioProperties = {
        volume: 0.05,
        position: position,
        loop: true
    };

    bubbleInjector = Audio.playSound(bubbleSound, audioProperties);

}

function createInnerContainer(position) {

    var tankProperties = Entities.getEntityProperties(fishTank);

    var containerProps = {
        name: "hifi-home-fishtank-inner-container",
        type: 'Box',
        color: {
            red: 0,
            green: 0,
            blue: 255
        },
        parentID: fishTank,
        dimensions: INNER_TANK_DIMENSIONS,
        position: tankProperties.position,
        visible: false,
        collisionless: true,
        dynamic: false
    };

    innerContainer = Entities.addEntity(containerProps);
}

function createEntitiesAtCorners() {

    var bounds = Entities.getEntityProperties(innerContainer, "boundingBox").boundingBox;

    var lowerProps = {
        name: 'hifi-home-fishtank-lower-corner',
        type: "Box",
        parentID: fishTank,
        dimensions: {
            x: 0.2,
            y: 0.2,
            z: 0.2
        },
        color: {
            red: 255,
            green: 0,
            blue: 0
        },
        collisionless: true,
        position: bounds.brn,
        visible: false
    }

    var upperProps = {
        name: 'hifi-home-fishtank-upper-corner',
        type: "Box",
        parentID: fishTank,
        dimensions: {
            x: 0.2,
            y: 0.2,
            z: 0.2
        },
        color: {
            red: 0,
            green: 255,
            blue: 0
        },
        collisionless: true,
        position: bounds.tfl,
        visible: false
    }

    lowerCorner = Entities.addEntity(lowerProps);
    upperCorner = Entities.addEntity(upperProps);

}

function createRocks() {
    var finalPosition = getOffsetFromTankCenter(ROCKS_VERTICAL_OFFSET, ROCKS_FORWARD_OFFSET, ROCKS_LATERAL_OFFSET);

    var properties = {
        name: 'hifi-home-fishtank-rock',
        type: 'Model',
        parentID: fishTank,
        modelURL: ROCK_MODEL_URL,
        position: finalPosition,
        dimensions: ROCK_DIMENSIONS
    }

    rocks = Entities.addEntity(properties);
}

function createUrchin() {
    var finalPosition = getOffsetFromTankCenter(URCHIN_VERTICAL_OFFSET, URCHIN_FORWARD_OFFSET, URCHIN_LATERAL_OFFSET);

    var properties = {
        name: 'hifi-home-fishtank-urchin',
        type: 'Model',
        parentID: fishTank,
        modelURL: URCHIN_MODEL_URL,
        position: finalPosition,
        shapeType: 'Sphere',
        dimensions: URCHIN_DIMENSIONS
    }

    urchin = Entities.addEntity(properties);

}

function createTreasureChest() {
    var finalPosition = getOffsetFromTankCenter(TREASURE_VERTICAL_OFFSET, TREASURE_FORWARD_OFFSET, TREASURE_LATERAL_OFFSET);

    var properties = {
        name: 'hifi-home-fishtank-treasure-chest',
        type: 'Model',
        parentID: fishTank,
        modelURL: TREASURE_MODEL_URL,
        position: finalPosition,
        dimensions: TREASURE_DIMENSIONS,
        rotation: Quat.fromPitchYawRollDegrees(10, -45, 10)
    }

    treasure = Entities.addEntity(properties);
}

function createTankBase() {
    var properties = {
        name: 'hifi-home-fishtank-base',
        type: 'Model',
        modelURL: TANK_BASE_MODEL_URL,
        parentID: fishTank,
        shapeType: 'compound',
        compoundShapeURL: TANK_BASE_COLLISION_HULL,
        position: {
            x: TANK_POSITION.x,
            y: TANK_POSITION.y - BASE_VERTICAL_OFFSET,
            z: TANK_POSITION.z
        },
        dimensions: TANK_BASE_DIMENSIONS
    }

    tankBase = Entities.addEntity(properties);
}

createFishTank();

createInnerContainer();

createBubbleSystems();

createEntitiesAtCorners();

createUrchin();

createRocks();

createTankBase();

createTreasureChest();
var customKey = 'hifi-home-fishtank';


var data = {
    fishLoaded: false,
    bubbleSystem: bubbleSystem,
    bubbleSound: bubbleSound,
    corners: {
        brn: lowerCorner,
        tfl: upperCorner
    },
    innerContainer: innerContainer,

}

Script.setTimeout(function() {
    setEntityCustomData(customKey, fishTank, data);
}, 2000)


function cleanup() {
    Entities.deleteEntity(fishTank);
    Entities.deleteEntity(tankBase);
    Entities.deleteEntity(bubbleSystem);
    Entities.deleteEntity(secondBubbleSystem);
    Entities.deleteEntity(thirdBubbleSystem);
    Entities.deleteEntity(innerContainer);
    Entities.deleteEntity(lowerCorner);
    Entities.deleteEntity(upperCorner);
    Entities.deleteEntity(urchin);
    Entities.deleteEntity(rocks);
    bubbleInjector.stop();
    bubbleInjector = null;
}


if (CLEANUP === true) {
    Script.scriptEnding.connect(cleanup);
}

function setEntityUserData(id, data) {
    var json = JSON.stringify(data)
    Entities.editEntity(id, {
        userData: json
    });
}

function getEntityUserData(id) {
    var results = null;
    var properties = Entities.getEntityProperties(id, "userData");
    if (properties.userData) {
        try {
            results = JSON.parse(properties.userData);
        } catch (err) {
            //   print('error parsing json');
            //   print('properties are:'+ properties.userData);
        }
    }
    return results ? results : {};
}


// Non-destructively modify the user data of an entity.
function setEntityCustomData(customKey, id, data) {
    var userData = getEntityUserData(id);
    if (data == null) {
        delete userData[customKey];
    } else {
        userData[customKey] = data;
    }
    setEntityUserData(id, userData);
}

function getEntityCustomData(customKey, id, defaultValue) {
    var userData = getEntityUserData(id);
    if (undefined != userData[customKey]) {
        return userData[customKey];
    } else {
        return defaultValue;
    }
}