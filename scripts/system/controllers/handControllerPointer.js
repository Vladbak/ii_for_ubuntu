"use strict";
/*jslint vars: true, plusplus: true*/
/*globals Script, Overlays, Controller, Reticle, HMD, Camera, Entities, MyAvatar, Settings, Menu, ScriptDiscoveryService, Window, Vec3, Quat, print */

//
//  handControllerPointer.js
//  examples/controllers
//
//  Created by Howard Stearns on 2016/04/22
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

// Control the "mouse" using hand controller. (HMD and desktop.)
// For now:
// Hydra thumb button 3 is left-mouse, button 4 is right-mouse.
// A click in the center of the vive thumb pad is left mouse. Vive menu button is context menu (right mouse).
// First-person only.
// Starts right handed, but switches to whichever is free: Whichever hand was NOT most recently squeezed.
//   (For now, the thumb buttons on both controllers are always on.)
// When over a HUD element, the reticle is shown where the active hand controller beam intersects the HUD.
// Otherwise, the active hand controller shows a red ball where a click will act.



// UTILITIES -------------
//

// Utility to make it easier to setup and disconnect cleanly.
function setupHandler(event, handler) {
    event.connect(handler);
    Script.scriptEnding.connect(function () {
        event.disconnect(handler);
    });
}
// If some capability is not available until expiration milliseconds after the last update.
function TimeLock(expiration) {
    var last = 0;
    this.update = function (optionalNow) {
        last = optionalNow || Date.now();
    };
    this.expired = function (optionalNow) {
        return ((optionalNow || Date.now()) - last) > expiration;
    };
}
var handControllerLockOut = new TimeLock(2000);

// Calls onFunction() or offFunction() when swtich(on), but only if it is to a new value.
function LatchedToggle(onFunction, offFunction, state) {
    this.getState = function () {
        return state;
    };
    this.setState = function (on) {
        if (state === on) {
            return;
        }
        state = on;
        if (on) {
            onFunction();
        } else {
            offFunction();
        }
    };
}

// VERTICAL FIELD OF VIEW ---------
//
// Cache the verticalFieldOfView setting and update it every so often.
var verticalFieldOfView, DEFAULT_VERTICAL_FIELD_OF_VIEW = 45; // degrees
function updateFieldOfView() {
    verticalFieldOfView = Settings.getValue('fieldOfView') || DEFAULT_VERTICAL_FIELD_OF_VIEW;
}

// SHIMS ----------
//
var weMovedReticle = false;
function ignoreMouseActivity() {
    // If we're paused, or if change in cursor position is from this script, not the hardware mouse.
    if (!Reticle.allowMouseCapture) {
        return true;
    }
    // Only we know if we moved it, which is why this script has to replace depthReticle.js
    if (!weMovedReticle) {
        return false;
    }
    weMovedReticle = false;
    return true;
}
var setReticlePosition = function (point2d) {
    weMovedReticle = true;
    Reticle.setPosition(point2d);
};

// Generalizations of utilities that work with system and overlay elements.
function findRayIntersection(pickRay) {
    // Check 3D overlays and entities. Argument is an object with origin and direction.
    var result = Overlays.findRayIntersection(pickRay);
    if (!result.intersects) {
        result = Entities.findRayIntersection(pickRay, true);
    }
    return result;
}
function isPointingAtOverlay(optionalHudPosition2d) {
    return Reticle.pointingAtSystemOverlay || Overlays.getOverlayAtPoint(optionalHudPosition2d || Reticle.position);
}

// Generalized HUD utilities, with or without HMD:
// These two "vars" are for documentation. Do not change their values!
var SPHERICAL_HUD_DISTANCE = 1; // meters.
var PLANAR_PERPENDICULAR_HUD_DISTANCE = SPHERICAL_HUD_DISTANCE;
function calculateRayUICollisionPoint(position, direction) {
    // Answer the 3D intersection of the HUD by the given ray, or falsey if no intersection.
    if (HMD.active) {
        return HMD.calculateRayUICollisionPoint(position, direction);
    }
    // interect HUD plane, 1m in front of camera, using formula:
    //   scale = hudNormal dot (hudPoint - position) / hudNormal dot direction
    //   intersection = postion + scale*direction
    var hudNormal = Quat.getFront(Camera.getOrientation());
    var hudPoint = Vec3.sum(Camera.getPosition(), hudNormal); // must also scale if PLANAR_PERPENDICULAR_HUD_DISTANCE!=1
    var denominator = Vec3.dot(hudNormal, direction);
    if (denominator === 0) {
        return null;
    } // parallel to plane
    var numerator = Vec3.dot(hudNormal, Vec3.subtract(hudPoint, position));
    var scale = numerator / denominator;
    return Vec3.sum(position, Vec3.multiply(scale, direction));
}
var DEGREES_TO_HALF_RADIANS = Math.PI / 360;
function overlayFromWorldPoint(point) {
    // Answer the 2d pixel-space location in the HUD that covers the given 3D point.
    // REQUIRES: that the 3d point be on the hud surface!
    // Note that this is based on the Camera, and doesn't know anything about any
    // ray that may or may not have been used to compute the point. E.g., the
    // overlay point is NOT the intersection of some non-camera ray with the HUD.
    if (HMD.active) {
        return HMD.overlayFromWorldPoint(point);
    }
    var cameraToPoint = Vec3.subtract(point, Camera.getPosition());
    var cameraX = Vec3.dot(cameraToPoint, Quat.getRight(Camera.getOrientation()));
    var cameraY = Vec3.dot(cameraToPoint, Quat.getUp(Camera.getOrientation()));
    var size = Controller.getViewportDimensions();
    var hudHeight = 2 * Math.tan(verticalFieldOfView * DEGREES_TO_HALF_RADIANS); // must adjust if PLANAR_PERPENDICULAR_HUD_DISTANCE!=1
    var hudWidth = hudHeight * size.x / size.y;
    var horizontalFraction = (cameraX / hudWidth + 0.5);
    var verticalFraction = 1 - (cameraY / hudHeight + 0.5);
    var horizontalPixels = size.x * horizontalFraction;
    var verticalPixels = size.y * verticalFraction;
    return { x: horizontalPixels, y: verticalPixels };
}

// MOUSE ACTIVITY --------
//
var isSeeking = false;
var averageMouseVelocity = 0, lastIntegration = 0, lastMouse;
var WEIGHTING = 1 / 20; // simple moving average over last 20 samples
var ONE_MINUS_WEIGHTING = 1 - WEIGHTING;
var AVERAGE_MOUSE_VELOCITY_FOR_SEEK_TO = 20;
function isShakingMouse() { // True if the person is waving the mouse around trying to find it.
    var now = Date.now(), mouse = Reticle.position, isShaking = false;
    if (lastIntegration && (lastIntegration !== now)) {
        var velocity = Vec3.length(Vec3.subtract(mouse, lastMouse)) / (now - lastIntegration);
        averageMouseVelocity = (ONE_MINUS_WEIGHTING * averageMouseVelocity) + (WEIGHTING * velocity);
        if (averageMouseVelocity > AVERAGE_MOUSE_VELOCITY_FOR_SEEK_TO) {
            isShaking = true;
        }
    }
    lastIntegration = now;
    lastMouse = mouse;
    return isShaking;
}
var NON_LINEAR_DIVISOR = 2;
var MINIMUM_SEEK_DISTANCE = 0.01;
function updateSeeking() {
    if (!Reticle.visible || isShakingMouse()) {
        if (!isSeeking) {
            print('Start seeking mouse.');
            isSeeking = true;
        }
    } // e.g., if we're about to turn it on with first movement.
    if (!isSeeking) {
        return;
    }
    averageMouseVelocity = lastIntegration = 0;
    var lookAt2D = HMD.getHUDLookAtPosition2D();
    if (!lookAt2D) {
        // FIXME - determine if this message is useful but make it so it doesn't spam the
        // log in the case that it is happening
        //print('Cannot seek without lookAt position');
        return;
    } // E.g., if parallel to location in HUD
    var copy = Reticle.position;
    function updateDimension(axis) {
        var distanceBetween = lookAt2D[axis] - Reticle.position[axis];
        var move = distanceBetween / NON_LINEAR_DIVISOR;
        if (Math.abs(move) < MINIMUM_SEEK_DISTANCE) {
            return false;
        }
        copy[axis] += move;
        return true;
    }
    var okX = !updateDimension('x'), okY = !updateDimension('y'); // Evaluate both. Don't short-circuit.
    if (okX && okY) {
        print('Finished seeking mouse');
        isSeeking = false;
    } else {
        Reticle.setPosition(copy); // Not setReticlePosition
    }
}

var mouseCursorActivity = new TimeLock(5000);
var APPARENT_MAXIMUM_DEPTH = 100.0; // this is a depth at which things all seem sufficiently distant
function updateMouseActivity(isClick) {
    if (ignoreMouseActivity()) {
        return;
    }
    var now = Date.now();
    mouseCursorActivity.update(now);
    if (isClick) {
        return;
    } // Bug: mouse clicks should keep going. Just not hand controller clicks
    handControllerLockOut.update(now);
    Reticle.visible = true;
}
function expireMouseCursor(now) {
    if (!isPointingAtOverlay() && mouseCursorActivity.expired(now)) {
        Reticle.visible = false;
    }
}
function onMouseMove() {
    // Display cursor at correct depth (as in depthReticle.js), and updateMouseActivity.
    if (ignoreMouseActivity()) {
        return;
    }

    if (HMD.active) { // set depth
        updateSeeking();
        if (isPointingAtOverlay()) {
            Reticle.setDepth(SPHERICAL_HUD_DISTANCE); // NOT CORRECT IF WE SWITCH TO OFFSET SPHERE!
        } else {
            var result = findRayIntersection(Camera.computePickRay(Reticle.position.x, Reticle.position.y));
            var depth = result.intersects ? result.distance : APPARENT_MAXIMUM_DEPTH;
            Reticle.setDepth(depth);
        }
    }
    updateMouseActivity(); // After the above, just in case the depth movement is awkward when becoming visible.
}
function onMouseClick() {
    updateMouseActivity(true);
}
setupHandler(Controller.mouseMoveEvent, onMouseMove);
setupHandler(Controller.mousePressEvent, onMouseClick);
setupHandler(Controller.mouseDoublePressEvent, onMouseClick);

// CONTROLLER MAPPING ---------
//

var activeHand = Controller.Standard.RightHand;
function toggleHand() {
    if (activeHand === Controller.Standard.RightHand) {
        activeHand = Controller.Standard.LeftHand;
    } else {
        activeHand = Controller.Standard.RightHand;
    }
}

var clickMapping = Controller.newMapping(Script.resolvePath('') + '-click');
Script.scriptEnding.connect(clickMapping.disable);

clickMapping.from(Controller.Standard.RightPrimaryThumb).peek().to(Controller.Actions.ReticleClick);
clickMapping.from(Controller.Standard.LeftPrimaryThumb).peek().to(Controller.Actions.ReticleClick);
clickMapping.from(Controller.Standard.RightSecondaryThumb).peek().to(Controller.Actions.ContextMenu);
clickMapping.from(Controller.Standard.LeftSecondaryThumb).peek().to(Controller.Actions.ContextMenu);
clickMapping.from(Controller.Standard.RightPrimaryThumb).peek().to(function (on) {
    if (on && (activeHand !== Controller.Standard.RightHand)) {
        toggleHand();
    }
});
clickMapping.from(Controller.Standard.LeftPrimaryThumb).peek().to(function (on) {
    if (on && (activeHand !== Controller.Standard.LeftHand)) {
        toggleHand();
    }
});
clickMapping.enable();

// VISUAL AID -----------
// Same properties as handControllerGrab search sphere
var BALL_SIZE = 0.011;
var BALL_ALPHA = 0.5;
var fakeProjectionBall = Overlays.addOverlay("sphere", {
    size: 5 * BALL_SIZE,
    color: {red: 255, green: 10, blue: 10},
    ignoreRayIntersection: true,
    alpha: BALL_ALPHA,
    visible: false,
    solid: true,
    drawInFront: true // Even when burried inside of something, show it.
});
var overlays = [fakeProjectionBall]; // If we want to try showing multiple balls and lasers.
Script.scriptEnding.connect(function () {
    overlays.forEach(Overlays.deleteOverlay);
});
var visualizationIsShowing = false; // Not whether it desired, but simply whether it is. Just an optimziation.
function turnOffVisualization(optionalEnableClicks) { // because we're showing cursor on HUD
    if (!optionalEnableClicks) {
        expireMouseCursor();
    }
    if (!visualizationIsShowing) {
        return;
    }
    visualizationIsShowing = false;
    overlays.forEach(function (overlay) {
        Overlays.editOverlay(overlay, {visible: false});
    });
}
var MAX_RAY_SCALE = 32000; // Anything large. It's a scale, not a distance.
function updateVisualization(controllerPosition, controllerDirection, hudPosition3d, hudPosition2d) {
    // Show an indication of where the cursor will appear when crossing a HUD element,
    // and where in-world clicking will occur.
    //
    // There are a number of ways we could do this, but for now, it's a blue sphere that rolls along
    // the HUD surface, and a red sphere that rolls along the 3d objects that will receive the click.
    // We'll leave it to other scripts (like handControllerGrab) to show a search beam when desired.

    function intersection3d(position, direction) {
        // Answer in-world intersection (entity or 3d overlay), or way-out point
        var pickRay = {origin: position, direction: direction};
        var result = findRayIntersection(pickRay);
        return result.intersects ? result.intersection : Vec3.sum(position, Vec3.multiply(MAX_RAY_SCALE, direction));
    }

    visualizationIsShowing = true;
    // We'd rather in-world interactions be done at the termination of the hand beam
    // -- intersection3d(controllerPosition, controllerDirection). Maybe have handControllerGrab
    // direclty manipulate both entity and 3d overlay objects.
    // For now, though, we present a false projection of the cursor onto whatever is below it. This is
    // different from the hand beam termination because the false projection is from the camera, while
    // the hand beam termination is from the hand.
    var eye = Camera.getPosition();
    var falseProjection = intersection3d(eye, Vec3.subtract(hudPosition3d, eye));
    Overlays.editOverlay(fakeProjectionBall, {visible: true, position: falseProjection});
    Reticle.visible = false;

    return visualizationIsShowing; // In case we change caller to act conditionally.
}

// MAIN OPERATIONS -----------
//
function update() {
    var now = Date.now();
    if (!handControllerLockOut.expired(now)) {
        return turnOffVisualization();
    } // Let them use mouse it in peace.
    if (!Menu.isOptionChecked("First Person")) {
        return turnOffVisualization();
    }  // What to do? menus can be behind hand!
    if (!Window.hasFocus()) { // Don't mess with other apps
        return turnOffVisualization();
    }
    var controllerPose = Controller.getPoseValue(activeHand);
    // Valid if any plugged-in hand controller is "on". (uncradled Hydra, green-lighted Vive...)
    if (!controllerPose.valid) {
        return turnOffVisualization();
    } // Controller is cradled.
    var controllerPosition = Vec3.sum(Vec3.multiplyQbyV(MyAvatar.orientation, controllerPose.translation),
                                      MyAvatar.position);
    // This gets point direction right, but if you want general quaternion it would be more complicated:
    var controllerDirection = Quat.getUp(Quat.multiply(MyAvatar.orientation, controllerPose.rotation));

    var hudPoint3d = calculateRayUICollisionPoint(controllerPosition, controllerDirection);
    if (!hudPoint3d) {
        // FIXME - determine if this message is useful but make it so it doesn't spam the
        // log in the case that it is happening
        //print('Controller is parallel to HUD');
        return turnOffVisualization();
    }
    var hudPoint2d = overlayFromWorldPoint(hudPoint3d);

    // We don't know yet if we'll want to make the cursor visble, but we need to move it to see if
    // it's pointing at a QML tool (aka system overlay).
    setReticlePosition(hudPoint2d);
    // If there's a HUD element at the (newly moved) reticle, just make it visible and bail.
    if (isPointingAtOverlay(hudPoint2d)) {
        if (HMD.active) {  // Doesn't hurt anything without the guard, but consider it documentation.
            Reticle.depth = SPHERICAL_HUD_DISTANCE; // NOT CORRECT IF WE SWITCH TO OFFSET SPHERE!
        }
        Reticle.visible = true;
        return turnOffVisualization(true);
    }
    // We are not pointing at a HUD element (but it could be a 3d overlay).
    updateVisualization(controllerPosition, controllerDirection, hudPoint3d, hudPoint2d);
}

var UPDATE_INTERVAL = 50; // milliseconds. Script.update is too frequent.
var updater = Script.setInterval(update, UPDATE_INTERVAL);
Script.scriptEnding.connect(function () {
    Script.clearInterval(updater);
});

// Check periodically for changes to setup.
var SETTINGS_CHANGE_RECHECK_INTERVAL = 10 * 1000; // milliseconds
function checkSettings() {
    updateFieldOfView();
}
checkSettings();
var settingsChecker = Script.setInterval(checkSettings, SETTINGS_CHANGE_RECHECK_INTERVAL);
Script.scriptEnding.connect(function () {
    Script.clearInterval(settingsChecker);
});
