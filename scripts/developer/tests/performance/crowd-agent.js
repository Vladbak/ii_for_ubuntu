"use strict";
/*jslint vars: true, plusplus: true*/
/*global Agent, Avatar, Script, Entities, Vec3, Quat, print*/
//
//  crowd-agent.js
//  scripts/developer/tests/performance/
//
//  Created by Howard Stearns on 9/29/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
//  Add this to domain-settings scripts url with n instances. It will lie dormant until
//  a script like summon.js calls up to n avatars to be around you.

var MESSAGE_CHANNEL = "io.highfidelity.summon-crowd";

print('crowd-agent version 4');

/* Observations:
- File urls for AC scripts silently fail. Use a local server (e.g., python SimpleHTTPServer) for development.
- URLs are cached regardless of server headers. Must use cache-defeating query parameters.
- JSON.stringify(Avatar) silently fails (even when Agent.isAvatar)
- If you run from a dev build directory, you must link assignment-client/<target>/resources to ../../interface/<target>/resources
*/

function messageSend(message) {
    Messages.sendMessage(MESSAGE_CHANNEL, JSON.stringify(message));
}

function getSound(data, callback) { // callback(sound) when downloaded (which may be immediate).
    var sound = SoundCache.getSound(data.url);
    if (sound.downloaded) {
        return callback(sound);
    }
    function onDownloaded() {
        sound.ready.disconnect(onDownloaded);
        callback(sound);
    }
    sound.ready.connect(onDownloaded);
}
function onFinishedPlaying() {
    messageSend({key: 'finishedSound'});
}

var attachment;
var stopper;
function clearStopper() {
    if (!stopper) {
        return;
    }
    Script.clearTimeout(stopper);
    stopper = null;
}
function stopAgent(parameters) {
    function stop() {
        clearStopper();
        if (attachment) {
            Avatar.detachOne(attachment.modelURL, attachment.jointName);
            attachment = undefined;
        }
        Agent.isListeningToAudioStream = false;
        Agent.isAvatar = false;
        print('crowd-agent stopped', JSON.stringify(parameters), JSON.stringify(Agent));
    }
    // Shutting down lots of agents at once can be hard on other parts of the system. (See fogbugz 2095.)
    // For now, accept a parameter to delay for the given number of milliseconds before stopping.
    // (We cannot count on summoning scripts to spread out the STOP messages, because they might be doing so
    // on scriptEnding, in which case they are not allowed to create new delays.)
    if (parameters.delay) {
        if (!stopper) { // Let the first stopper do the deed.
            stopper = Script.setTimeout(stop, parameters.delay);
        }
    } else {
        stop();
    }
}


var MILLISECONDS_IN_SECOND = 1000;
function startAgent(parameters) { // Can also be used to update.
    print('crowd-agent starting params', JSON.stringify(parameters), JSON.stringify(Agent));
    clearStopper();
    var wasOff = !Agent.isAvatar;
    Agent.isAvatar = true;
    if (parameters.position) {
        Avatar.position = parameters.position;
    }
    if (parameters.orientation) {
        Avatar.orientation = parameters.orientation;
    }
    if (parameters.skeletonModelURL) {
        Avatar.skeletonModelURL = parameters.skeletonModelURL;
    }
    if (parameters.listen != undefined) {
        Agent.isListeningToAudioStream = parameters.listen; // Send silence when not chattering.
    } else if (wasOff) {
        Agent.isListeningToAudioStream = true;
    }
    if (parameters.soundData) {
        getSound(parameters.soundData, function (sound) {
            Script.setTimeout(onFinishedPlaying, sound.duration * MILLISECONDS_IN_SECOND);
            Agent.playAvatarSound(sound);
        });
    }
    if (parameters.animationData) {
        var data = parameters.animationData;
        Avatar.startAnimation(data.url, data.fps || 30, 1.0, (data.loopFlag === undefined) ? true : data.loopFlag, false, data.startFrame || 0, data.endFrame);
    }
    if (parameters.attachment) {
        attachment = parameters.attachment;
        Avatar.attach(attachment.modelURL, attachment.jointName, attachment.translation, attachment.rotation, attachment.scale, attachment.isSoft);
    } else {
        attachment = undefined;
    }
    print('crowd-agent avatars started');
}

function messageHandler(channel, messageString, senderID) {
    if (channel !== MESSAGE_CHANNEL) {
        return;
    }
    print('crowd-agent message', channel, messageString, senderID);
    if (Agent.sessionUUID === senderID) { // ignore my own
        return;
    }
    var message = {};
    try {
        message = JSON.parse(messageString);
    } catch (e) {
        print(e);
    }
    switch (message.key) {
    case "HELO":
        messageSend({key: 'hello'}); // Allow the coordinator to count responses and make assignments.
        break;
    case 'hello': // ignore responses (e.g., from other agents)
    case 'finishedSound':
        break;
    case "SUMMON":
        if (message.rcpt === Agent.sessionUUID) {
            startAgent(message);
        }
        break;
    case "STOP":
        if (message.rcpt === Agent.sessionUUID) {
            stopAgent(message);
        }
        break;
    default:
        print("crowd-agent received unrecognized message:", channel, messageString, senderID);
    }
}
Messages.subscribe(MESSAGE_CHANNEL);
Messages.messageReceived.connect(messageHandler);

Script.scriptEnding.connect(function () {
    print('crowd-agent shutting down');
    Messages.messageReceived.disconnect(messageHandler);
    Messages.unsubscribe(MESSAGE_CHANNEL);
    print('crowd-agent unsubscribed');
});
