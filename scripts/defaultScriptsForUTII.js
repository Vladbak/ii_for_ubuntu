"use strict";
/* jslint vars: true, plusplus: true */

// based on:
//  defaultScripts.js
//  examples
//
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

var ROLE = {
    RankAndFile: 1 << 0,
    Trainers: 1 << 1,
    THERankAndFile: 1 << 2,
    THETrainers: 1 << 3,
    Admin: 1 << 4
}
ROLE['All'] = ROLE.RankAndFile | ROLE.Trainers | ROLE.THERankAndFile | ROLE.THETrainers | ROLE.Admin;

var DEFAULT_SCRIPTS = [
    "system/progress.js",
    "system/mute.js",
    "system/goto.js",
    "system/selectAudioDevice.js",
    "system/notifications.js",
    "system/controllers/grab.js",
    "http://hifi-assets.e-spaces.com/scripts/defaultScripts.js"
];

if (((Account.role & ROLE.Trainers) === ROLE.Trainers) || ((Account.role & ROLE.THETrainers) === ROLE.THETrainers) ||
    ((Account.role & ROLE.Admin) === ROLE.Admin)) {
    DEFAULT_SCRIPTS.push('system/away.js');
    DEFAULT_SCRIPTS.push('system/hmd.js');
    DEFAULT_SCRIPTS.push('system/edit.js');
    DEFAULT_SCRIPTS.push('system/mod.js');
    DEFAULT_SCRIPTS.push('system/marketplaces/marketplace.js');
    DEFAULT_SCRIPTS.push('system/controllers/handControllerGrab.js');
    DEFAULT_SCRIPTS.push('system/controllers/handControllerPointer.js');
    DEFAULT_SCRIPTS.push('system/controllers/squeezeHands.js');
    DEFAULT_SCRIPTS.push('system/controllers/teleport.js');
    DEFAULT_SCRIPTS.push('system/controllers/toggleAdvancedMovementForHandControllers.js');
    DEFAULT_SCRIPTS.push('system/firstPersonHMD.js');
}

// add a menu item for debugging
var MENU_CATEGORY = "Developer";
var MENU_ITEM = "Debug defaultScripts.js";

var SETTINGS_KEY = '_debugDefaultScriptsIsChecked';
var previousSetting = Settings.getValue(SETTINGS_KEY);

if (previousSetting === '' || previousSetting === false || previousSetting === 'false') {
    previousSetting = false;
}

if (previousSetting === true || previousSetting === 'true') {
    previousSetting = true;
}

if (Menu.menuExists(MENU_CATEGORY) && !Menu.menuItemExists(MENU_CATEGORY, MENU_ITEM)) {
    Menu.addMenuItem({
        menuName: MENU_CATEGORY,
        menuItemName: MENU_ITEM,
        isCheckable: true,
        isChecked: previousSetting,
        grouping: "Advanced"
    });
}

function runDefaultsTogether() {
    for (var j in DEFAULT_SCRIPTS) {
        Script.include(DEFAULT_SCRIPTS[j]);
    }
}

function runDefaultsSeparately() {
    for (var i in DEFAULT_SCRIPTS) {
        Script.load(DEFAULT_SCRIPTS[i]);
    }
}
// start all scripts
if (Menu.isOptionChecked(MENU_ITEM)) {
    // we're debugging individual default scripts
    // so we load each into its own ScriptEngine instance
    debuggingDefaultScripts = true;
    runDefaultsSeparately();
} else {
    // include all default scripts into this ScriptEngine
    runDefaultsTogether();
}

function menuItemEvent(menuItem) {
    if (menuItem == MENU_ITEM) {

        isChecked = Menu.isOptionChecked(MENU_ITEM);
        if (isChecked === true) {
            Settings.setValue(SETTINGS_KEY, true);
        } else if (isChecked === false) {
            Settings.setValue(SETTINGS_KEY, false);
        }
         Window.alert('You must reload all scripts for this to take effect.')
    }
}



function stopLoadedScripts() {
        // remove debug script loads
    var runningScripts = ScriptDiscoveryService.getRunning();
    for (var i in runningScripts) {
        var scriptName = runningScripts[i].name;
        for (var j in DEFAULT_SCRIPTS) {
            if (DEFAULT_SCRIPTS[j].slice(-scriptName.length) === scriptName) {
                ScriptDiscoveryService.stopScript(runningScripts[i].url);
            }
        }
    }
}

function removeMenuItem() {
    if (!Menu.isOptionChecked(MENU_ITEM)) {
        Menu.removeMenuItem(MENU_CATEGORY, MENU_ITEM);
    }
}

Script.scriptEnding.connect(function() {
    stopLoadedScripts();
    removeMenuItem();
});

Menu.menuItemEvent.connect(menuItemEvent);
