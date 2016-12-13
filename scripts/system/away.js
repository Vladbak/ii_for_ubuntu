"use strict";

//
//  away.js
//
//  examples
//
//  Created by Howard Stearns 11/3/15
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
// Goes into "paused" when the '.' key (and automatically when started in HMD), and normal when pressing any key.
// See MAIN CONTROL, below, for what "paused" actually does.

(function() { // BEGIN LOCAL_SCOPE

var BASIC_TIMER_INTERVAL = 50; // 50ms = 20hz
var OVERLAY_WIDTH = 1920;
var OVERLAY_HEIGHT = 1080;
var OVERLAY_DATA = {
    width: OVERLAY_WIDTH,
    height: OVERLAY_HEIGHT,
    imageURL: "http://hifi-content.s3.amazonaws.com/alan/production/images/images/Overlay-Viz-blank.png",
    color: {red: 255, green: 255, blue: 255},
    alpha: 1
};
var AVATAR_MOVE_FOR_ACTIVE_DISTANCE = 0.8; // meters -- no longer away if avatar moves this far while away

var lastOverlayPosition = { x: 0, y: 0, z: 0};
var OVERLAY_DATA_HMD = {
    position: lastOverlayPosition,
    width: OVERLAY_WIDTH,
    height: OVERLAY_HEIGHT,
    url: "http://hifi-content.s3.amazonaws.com/alan/production/images/images/Overlay-Viz-blank.png",
    color: {red: 255, green: 255, blue: 255},
    alpha: 1,
    scale: 2,
    emissive: true,
    isFacingAvatar: true,
    drawInFront: true
};

var AWAY_INTRO = {
    url: "http://hifi-content.s3.amazonaws.com/ozan/dev/anim/standard_anims_160127/kneel.fbx",
    playbackRate: 30.0,
    loopFlag: false,
    startFrame: 0.0,
    endFrame: 83.0
};

// MAIN CONTROL
var isEnabled = true;
var wasMuted; // unknonwn?
var isAway = false; // we start in the un-away state
var eventMappingName = "io.highfidelity.away"; // goActive on hand controller button events, too.
var eventMapping = Controller.newMapping(eventMappingName);
var avatarPosition = MyAvatar.position;
var wasHmdMounted = HMD.mounted;


// some intervals we may create/delete
var avatarMovedInterval;


// prefetch the kneel animation and hold a ref so it's always resident in memory when we need it.
var _animation = AnimationCache.prefetch(AWAY_INTRO.url);

function playAwayAnimation() {
    MyAvatar.overrideAnimation(AWAY_INTRO.url,
            AWAY_INTRO.playbackRate,
            AWAY_INTRO.loopFlag,
            AWAY_INTRO.startFrame,
            AWAY_INTRO.endFrame);
}

function stopAwayAnimation() {
    MyAvatar.restoreAnimation();
}

// OVERLAY
var overlay = Overlays.addOverlay("image", OVERLAY_DATA);
var overlayHMD = Overlays.addOverlay("image3d", OVERLAY_DATA_HMD);

function moveCloserToCamera(positionAtHUD) {
    // we don't actually want to render at the slerped look at... instead, we want to render
    // slightly closer to the camera than that.
    var MOVE_CLOSER_TO_CAMERA_BY = -0.25;
    var cameraFront = Quat.getFront(Camera.orientation);
    var closerToCamera = Vec3.multiply(cameraFront, MOVE_CLOSER_TO_CAMERA_BY); // slightly closer to camera
    var slightlyCloserPosition = Vec3.sum(positionAtHUD, closerToCamera);

    return slightlyCloserPosition;
}

function showOverlay() {
    if (HMD.active) {
        // make sure desktop version is hidden
        Overlays.editOverlay(overlay, { visible: false });

        lastOverlayPosition = HMD.getHUDLookAtPosition3D();
        var actualOverlayPositon = moveCloserToCamera(lastOverlayPosition);
        Overlays.editOverlay(overlayHMD, { visible: true, position: actualOverlayPositon });
    } else {
        // make sure HMD is hidden
        Overlays.editOverlay(overlayHMD, { visible: false });

        // Update for current screen size, keeping overlay proportions constant.
        var screen = Controller.getViewportDimensions();

        // keep the overlay it's natural size and always center it...
        Overlays.editOverlay(overlay, { visible: true,
                    x: ((screen.x - OVERLAY_WIDTH) / 2),
                    y: ((screen.y - OVERLAY_HEIGHT) / 2) });
    }
}

function hideOverlay() {
    Overlays.editOverlay(overlay, {visible: false});
    Overlays.editOverlay(overlayHMD, {visible: false});
}

hideOverlay();

function maybeMoveOverlay() {
    if (isAway) {
        // if we switched from HMD to Desktop, make sure to hide our HUD overlay and show the
        // desktop overlay
        if (!HMD.active) {
            showOverlay(); // this will also recenter appropriately
        }

        if (HMD.active) {
            // Note: instead of moving it directly to the lookAt, we will move it slightly toward the
            // new look at. This will result in a more subtle slerp toward the look at and reduce jerkiness
            var EASE_BY_RATIO = 0.1;
            var lookAt = HMD.getHUDLookAtPosition3D();
            var lookAtChange = Vec3.subtract(lookAt, lastOverlayPosition);
            var halfWayBetweenOldAndLookAt = Vec3.multiply(lookAtChange, EASE_BY_RATIO);
            var newOverlayPosition = Vec3.sum(lastOverlayPosition, halfWayBetweenOldAndLookAt);
            lastOverlayPosition = newOverlayPosition;
            var actualOverlayPositon = moveCloserToCamera(lastOverlayPosition);
            Overlays.editOverlay(overlayHMD, { visible: true, position: actualOverlayPositon });

            // make sure desktop version is hidden
            Overlays.editOverlay(overlay, { visible: false });

            // also remember avatar position
            avatarPosition = MyAvatar.position;

        }
    }
}

function ifAvatarMovedGoActive() {
    var newAvatarPosition = MyAvatar.position;
    if (Vec3.distance(newAvatarPosition, avatarPosition) > AVATAR_MOVE_FOR_ACTIVE_DISTANCE) {
        goActive();
    }
    avatarPosition = newAvatarPosition;
}

function goAway(fromStartup) {
    if (!isEnabled || isAway) {
        return;
    }

    UserActivityLogger.toggledAway(true);

    isAway = true;
    wasMuted = AudioDevice.getMuted();
    if (!wasMuted) {
        AudioDevice.toggleMute();
    }
    MyAvatar.setEnableMeshVisible(false);  // just for our own display, without changing point of view
    playAwayAnimation(); // animation is still seen by others
    showOverlay();

    HMD.requestShowHandControllers();

    // tell the Reticle, we want to stop capturing the mouse until we come back
    Reticle.allowMouseCapture = false;
    // Allow users to find their way to other applications, our menus, etc.
    // For desktop, that means we want the reticle visible.
    // For HMD, the hmd preview will show the system mouse because of allowMouseCapture,
    // but we want to turn off our Reticle so that we don't get two in preview and a stuck one in headset.
    Reticle.visible = !HMD.active;
    wasHmdMounted = HMD.mounted; // always remember the correct state

    avatarPosition = MyAvatar.position;

    // If we're entering away mode from some other state than startup, then we create our move timer immediately.
    // However if we're just stating up, we need to delay this process so that we don't think the initial teleport
    // is actually a move.
    if (fromStartup === undefined || fromStartup === false) {
        avatarMovedInterval = Script.setInterval(ifAvatarMovedGoActive, BASIC_TIMER_INTERVAL);
    } else {
        var WAIT_FOR_MOVE_ON_STARTUP = 3000; // 3 seconds
        Script.setTimeout(function() {
            avatarMovedInterval = Script.setInterval(ifAvatarMovedGoActive, BASIC_TIMER_INTERVAL);
        }, WAIT_FOR_MOVE_ON_STARTUP);
    }
}

function goActive() {
    if (!isAway) {
        return;
    }

    UserActivityLogger.toggledAway(false);

    isAway = false;
    if (!wasMuted) {
        AudioDevice.toggleMute();
    }
    MyAvatar.setEnableMeshVisible(true); // IWBNI we respected Developer->Avatar->Draw Mesh setting.
    stopAwayAnimation();

    HMD.requestHideHandControllers();

    // update the UI sphere to be centered about the current HMD orientation.
    HMD.centerUI();

    // forget about any IK joint limits
    MyAvatar.clearIKJointLimitHistory();

    // update the avatar hips to point in the same direction as the HMD orientation.
    MyAvatar.centerBody();

    hideOverlay();

    // tell the Reticle, we are ready to capture the mouse again and it should be visible
    Reticle.allowMouseCapture = true;
    Reticle.visible = true;
    if (HMD.active) {
        Reticle.position = HMD.getHUDLookAtPosition2D();
    }
    wasHmdMounted = HMD.mounted; // always remember the correct state

    Script.clearInterval(avatarMovedInterval);
}

function maybeGoActive(event) {
    if (event.isAutoRepeat) {  // isAutoRepeat is true when held down (or when Windows feels like it)
        return;
    }
    if (!isAway && (event.text == 'ESC')) {
        goAway();
    } else {
        goActive();
    }
}

var wasHmdActive = HMD.active;
var wasMouseCaptured = Reticle.mouseCaptured;

function maybeGoAway() {
    // If our active state change (went to or from HMD mode), and we are now in the HMD, go into away
    if (HMD.active !== wasHmdActive) {
        wasHmdActive = !wasHmdActive;
        if (wasHmdActive) {
            goAway();
            return;
        }
    }

    // If the mouse has gone from captured, to non-captured state, then it likely means the person is still in the HMD,
    // but tabbed away from the application (meaning they don't have mouse control) and they likely want to go into
    // an away state
    if (Reticle.mouseCaptured !== wasMouseCaptured) {
        wasMouseCaptured = !wasMouseCaptured;
        if (!wasMouseCaptured) {
            goAway();
            return;
        }
    }

    // If you've removed your HMD from your head, and we can detect it, we will also go away...
    if (HMD.mounted != wasHmdMounted) {
        wasHmdMounted = HMD.mounted;
        print("HMD mounted changed...");

        // We're putting the HMD on... switch to those devices
        if (HMD.mounted) {
            print("NOW mounted...");
        } else {
            print("HMD NOW un-mounted...");

            if (HMD.active) {
                goAway();
                return;
            }
        }
    }
}

function setEnabled(value) {
    if (!value) {
        goActive();
    }
    isEnabled = value;
}

var CHANNEL_AWAY_ENABLE = "Hifi-Away-Enable";
var handleMessage = function(channel, message, sender) {
    if (channel === CHANNEL_AWAY_ENABLE && sender === MyAvatar.sessionUUID) {
        print("away.js | Got message on Hifi-Away-Enable: ", message);
        setEnabled(message === 'enable');
    }
}
Messages.subscribe(CHANNEL_AWAY_ENABLE);
Messages.messageReceived.connect(handleMessage);

var maybeIntervalTimer = Script.setInterval(function(){
    maybeMoveOverlay();
    maybeGoAway();
}, BASIC_TIMER_INTERVAL);


Controller.mousePressEvent.connect(goActive);
Controller.keyPressEvent.connect(maybeGoActive);
// Note peek() so as to not interfere with other mappings.
eventMapping.from(Controller.Standard.LeftPrimaryThumb).peek().to(goActive);
eventMapping.from(Controller.Standard.RightPrimaryThumb).peek().to(goActive);
eventMapping.from(Controller.Standard.LeftSecondaryThumb).peek().to(goActive);
eventMapping.from(Controller.Standard.RightSecondaryThumb).peek().to(goActive);
eventMapping.from(Controller.Standard.LT).peek().to(goActive);
eventMapping.from(Controller.Standard.LB).peek().to(goActive);
eventMapping.from(Controller.Standard.LS).peek().to(goActive);
eventMapping.from(Controller.Standard.LeftGrip).peek().to(goActive);
eventMapping.from(Controller.Standard.RT).peek().to(goActive);
eventMapping.from(Controller.Standard.RB).peek().to(goActive);
eventMapping.from(Controller.Standard.RS).peek().to(goActive);
eventMapping.from(Controller.Standard.RightGrip).peek().to(goActive);
eventMapping.from(Controller.Standard.Back).peek().to(goActive);
eventMapping.from(Controller.Standard.Start).peek().to(goActive);
Controller.enableMapping(eventMappingName);

Script.scriptEnding.connect(function () {
    Script.clearInterval(maybeIntervalTimer);
    goActive();
    Controller.disableMapping(eventMappingName);
    Controller.mousePressEvent.disconnect(goActive);
    Controller.keyPressEvent.disconnect(maybeGoActive);
    Messages.messageReceived.disconnect(handleMessage);
    Messages.unsubscribe(CHANNEL_AWAY_ENABLE);
});

if (HMD.active && !HMD.mounted) {
    print("Starting script, while HMD is active and not mounted...");
    goAway(true);
}


}()); // END LOCAL_SCOPE
