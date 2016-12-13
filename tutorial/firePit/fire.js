// Originally written by James Pollack, modified by Ryan Huffman for the tutorial
//
// this script turns an entity into an exploder -- anything that collides with it will be vaporized!

(function() {

    function debug() {
        var args = Array.prototype.slice.call(arguments);
        args.unshift("fire.js | ");
        print.apply(this, args);
    }

    var _this = this;

    function Fire() {
        _this = this;
    }

    var RED = {
        red: 255,
        green: 0,
        blue: 0
    };

    var ORANGE = {
        red: 255,
        green: 165,
        blue: 0
    };

    var YELLOW = {
        red: 255,
        green: 255,
        blue: 0
    };

    var GREEN = {
        red: 0,
        green: 255,
        blue: 0
    };

    var BLUE = {
        red: 0,
        green: 0,
        blue: 255
    };

    var INDIGO = {
        red: 128,
        green: 0,
        blue: 128
    };

    var VIOLET = {
        red: 75,
        green: 0,
        blue: 130
    };

    var colors = [RED, ORANGE, YELLOW, GREEN, BLUE, INDIGO, VIOLET];

    var firePitSoundURL = Script.resolvePath("fire_burst.wav");
    debug("Firepit burst sound url is: ", firePitSoundURL);

    var explodeTextureURL = Script.resolvePath("explode.png");
    debug("Firepit explode texture url is: ", explodeTextureURL);

    Fire.prototype = {
        preload: function(entityID) {
            debug("Preload");
            this.entityID = entityID;
            this.EXPLOSION_SOUND = SoundCache.getSound(firePitSoundURL);
        },
        collisionWithEntity: function(myID, otherID, collisionInfo) {
            debug("Collided with entity: ", myID, otherID);
            var otherProps = Entities.getEntityProperties(otherID);
            var data = null;
            try {
                data = JSON.parse(otherProps.userData);
            } catch (err) {
                debug('ERROR GETTING USERDATA!');
            }
            if (data === null || "") {
                debug("Data is null or empty", data);
                return;
            } else {
                debug("Got data", data);
                if (data.hasOwnProperty('hifiHomeKey')) {
                    debug("Has hifiHomeKey");
                    if (data.hifiHomeKey.reset === true) {
                        debug("Reset is true");
                        _this.playSoundAtCurrentPosition();
                        _this.explodeWithColor();
                        Entities.deleteEntity(otherID)
                        debug("Sending local message");
                        Messages.sendLocalMessage('Entity-Exploded', JSON.stringify({
                            entityID: otherID,
                            position: Entities.getEntityProperties(this.entityID).position
                        }));
                        debug("Done sending local message");
                    }
                }
            }
        },
        explodeWithColor: function() {
            var myProps = Entities.getEntityProperties(this.entityID);
            var color = colors[Math.floor(Math.random() * colors.length)];
            var explosionParticleProperties = {
                "color": color,
                "isEmitting": 1,
                "maxParticles": 1000,
                "lifespan": 0.25,
                "emitRate": 1,
                "emitSpeed": 0.1,
                "speedSpread": 1,
                "emitOrientation": Quat.getUp(myProps.rotation),
                "emitDimensions": {
                    "x": 0,
                    "y": 0,
                    "z": 0
                },
                "polarStart": 0,
                "polarFinish": 0,
                "azimuthStart": 0,
                "azimuthFinish": 0,
                "emitAcceleration": {
                    "x": 0,
                    "y": 0,
                    "z": 0
                },
                "accelerationSpread": {
                    "x": 0,
                    "y": 0,
                    "z": 0
                },
                "particleRadius": 0.829,
                "radiusSpread": 0,
                "radiusStart": 0.361,
                "radiusFinish": 0.294,
                "colorSpread": {
                    "red": 0,
                    "green": 0,
                    "blue": 0
                },
                "colorStart": {
                    "red": 255,
                    "green": 255,
                    "blue": 255
                },
                "colorFinish": {
                    "red": 255,
                    "green": 255,
                    "blue": 255
                },
                "alpha": 1,
                "alphaSpread": 0,
                "alphaStart": -0.2,
                "alphaFinish": 0.5,
                "emitterShouldTrail": 0,
                "textures": explodeTextureURL,
                "type": "ParticleEffect",
                lifetime: 1,
                position: myProps.position
            };

            var explosion = Entities.addEntity(explosionParticleProperties);
        },
        playSoundAtCurrentPosition: function() {

            var audioProperties = {
                volume: 0.5,
                position: Entities.getEntityProperties(this.entityID).position
            };

            Audio.playSound(this.EXPLOSION_SOUND, audioProperties);
        },
    }

    return new Fire();
});
