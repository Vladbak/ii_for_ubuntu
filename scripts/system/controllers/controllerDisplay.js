//
//  controllerDisplay.js
//
//  Created by Anthony J. Thibault on 10/20/16
//  Originally created by Ryan Huffman on 9/21/2016
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

/* globals createControllerDisplay:true deleteControllerDisplay:true */

var PARENT_ID = "{00000000-0000-0000-0000-000000000001}";

function clamp(value, min, max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    }
    return value;
}

function resolveHardware(path) {
    if (typeof path === 'string') {
        var parts = path.split(".");
        function resolveInner(base, path, i) {
            if (i >= path.length) {
                return base;
            }
            return resolveInner(base[path[i]], path, ++i);
        }
        return resolveInner(Controller.Hardware, parts, 0);
    }
    return path;
}

var DEBUG = true;
function debug() {
    if (DEBUG) {
        var args = Array.prototype.slice.call(arguments);
        args.unshift("controllerDisplay.js | ");
        print.apply(this, args);
    }
}

createControllerDisplay = function(config) {
    var controllerDisplay = {
        overlays: [],
        partOverlays: {},
        parts: {},
        mappingName: "mapping-display-" + Math.random(),

        setVisible: function(visible) {
            for (var i = 0; i < this.overlays.length; ++i) {
                Overlays.editOverlay(this.overlays[i], {
                    visible: visible
                });
            }
        },

        setPartVisible: function(partName, visible) {
            return;
            if (partName in this.partOverlays) {
                for (var i = 0; i < this.partOverlays[partName].length; ++i) {
                    Overlays.editOverlay(this.partOverlays[partName][i], {
                        //visible: visible
                    });
                }
            }
        },

        setLayerForPart: function(partName, layerName) {
            if (partName in this.parts) {
                var part = this.parts[partName];
                if (part.textureLayers && layerName in part.textureLayers) {
                    var layer = part.textureLayers[layerName];
                    var textures = {};
                    if (layer.defaultTextureURL) {
                        textures[part.textureName] = layer.defaultTextureURL;
                    }
                    for (var i = 0; i < this.partOverlays[partName].length; ++i) {
                        Overlays.editOverlay(this.partOverlays[partName][i], {
                            textures: textures
                        });
                    }
                }
            }
        }
    };
    var mapping = Controller.newMapping(controllerDisplay.mappingName);
    for (var i = 0; i < config.controllers.length; ++i) {
        var controller = config.controllers[i];
        var position = controller.position;

        if (controller.naturalPosition) {
            position = Vec3.sum(Vec3.multiplyQbyV(controller.rotation, controller.naturalPosition), position);
        }

        var overlayID = Overlays.addOverlay("model", {
            url: controller.modelURL,
            dimensions: controller.dimensions,
            localRotation: controller.rotation,
            localPosition: position,
            parentID: PARENT_ID,
            parentJointIndex: controller.jointIndex,
            ignoreRayIntersection: true
        });

        controllerDisplay.overlays.push(overlayID);
        overlayID = null;

        if (controller.parts) {
            for (var partName in controller.parts) {
                var part = controller.parts[partName];
                var partPosition = Vec3.sum(controller.position, Vec3.multiplyQbyV(controller.rotation, part.naturalPosition));
                var innerRotation = controller.rotation;

                controllerDisplay.parts[partName] = controller.parts[partName];

                var properties = {
                    url: part.modelURL,
                    localPosition: partPosition,
                    localRotation: innerRotation,
                    parentID: PARENT_ID,
                    parentJointIndex: controller.jointIndex,
                    ignoreRayIntersection: true
                };

                if (part.defaultTextureLayer) {
                    var textures = {};
                    textures[part.textureName] = part.textureLayers[part.defaultTextureLayer].defaultTextureURL;
                    properties['textures'] = textures;
                }

                overlayID = Overlays.addOverlay("model", properties);

                if (part.type === "rotational") {
                    var input = resolveHardware(part.input);
                    print("Mapping to: ", part.input, input);
                    mapping.from([input]).peek().to(function(controller, overlayID, part) {
                        return function(value) {
                            value = clamp(value, part.minValue, part.maxValue);

                            var pct = (value - part.minValue) / part.maxValue;
                            var angle = pct * part.maxAngle;
                            var rotation = Quat.angleAxis(angle, part.axis);

                            var offset = { x: 0, y: 0, z: 0 };
                            if (part.origin) {
                                offset = Vec3.multiplyQbyV(rotation, part.origin);
                                offset = Vec3.subtract(offset, part.origin);
                            }

                            var partPosition = Vec3.sum(controller.position,
                                    Vec3.multiplyQbyV(controller.rotation, Vec3.sum(offset, part.naturalPosition)));

                            Overlays.editOverlay(overlayID, {
                                localPosition: partPosition,
                                localRotation: Quat.multiply(controller.rotation, rotation)
                            });
                        };
                    }(controller, overlayID, part));
                } else if (part.type === "touchpad") {
                    var visibleInput = resolveHardware(part.visibleInput);
                    var xInput = resolveHardware(part.xInput);
                    var yInput = resolveHardware(part.yInput);

                    // TODO: Touchpad inputs are currently only working for half
                    // of the touchpad. When that is fixed, it would be useful
                    // to update these to display the current finger position.
                    mapping.from([visibleInput]).peek().to(function(value) {
                    });
                    mapping.from([xInput]).peek().to(function(value) {
                    });
                    mapping.from([yInput]).peek().invert().to(function(value) {
                    });
                } else if (part.type === "joystick") {
                    (function(controller, overlayID, part) {
                        const xInput = resolveHardware(part.xInput);
                        const yInput = resolveHardware(part.yInput);

                        var xvalue = 0;
                        var yvalue = 0;

                        function calculatePositionAndRotation(xValue, yValue) {
                            var rotation = Quat.fromPitchYawRollDegrees(yValue * part.xHalfAngle, 0, xValue * part.yHalfAngle);

                            var offset = { x: 0, y: 0, z: 0 };
                            if (part.originOffset) {
                                offset = Vec3.multiplyQbyV(rotation, part.originOffset);
                                offset = Vec3.subtract(part.originOffset, offset);
                            }
                            
                            var partPosition = Vec3.sum(controller.position,
                                    Vec3.multiplyQbyV(controller.rotation, Vec3.sum(offset, part.naturalPosition)));

                            var partRotation = Quat.multiply(controller.rotation, rotation)

                            return {
                                position: partPosition,
                                rotation: partRotation
                            }
                        }

                        mapping.from([xInput]).peek().to(function(value) {
                            xvalue = value;
                            //print(overlayID, xvalue.toFixed(3), yvalue.toFixed(3));
                            var posRot = calculatePositionAndRotation(xvalue, yvalue);
                            Overlays.editOverlay(overlayID, {
                                localPosition: posRot.position,
                                localRotation: posRot.rotation
                            });
                        });

                        mapping.from([yInput]).peek().to(function(value) {
                            yvalue = value;
                            var posRot = calculatePositionAndRotation(xvalue, yvalue);
                            Overlays.editOverlay(overlayID, {
                                localPosition: posRot.position,
                                localRotation: posRot.rotation
                            });
                        });
                    })(controller, overlayID, part);

                } else if (part.type === "linear") {
                    (function(controller, overlayID, part) {
                        const input = resolveHardware(part.input);

                        mapping.from([input]).peek().to(function(value) {
                            //print(value);
                            var axis = Vec3.multiplyQbyV(controller.rotation, part.axis);
                            var offset = Vec3.multiply(part.maxTranslation * value, axis);

                            var partPosition = Vec3.sum(controller.position, Vec3.multiplyQbyV(controller.rotation, part.naturalPosition));
                            var position = Vec3.sum(partPosition, offset);

                            Overlays.editOverlay(overlayID, {
                                localPosition: position
                            });
                        });

                    })(controller, overlayID, part);

                } else if (part.type === "static") {
                    // do nothing
                } else {
                    debug("TYPE NOT SUPPORTED: ", part.type);
                }

                controllerDisplay.overlays.push(overlayID);
                if (!(partName in controllerDisplay.partOverlays)) {
                    controllerDisplay.partOverlays[partName] = [];
                }
                controllerDisplay.partOverlays[partName].push(overlayID);
            }
        }
    }
    Controller.enableMapping(controllerDisplay.mappingName);
    return controllerDisplay;
};

deleteControllerDisplay = function(controllerDisplay) {
    for (var i = 0; i < controllerDisplay.overlays.length; ++i) {
        Overlays.deleteOverlay(controllerDisplay.overlays[i]);
    }
    Controller.disableMapping(controllerDisplay.mappingName);
};
