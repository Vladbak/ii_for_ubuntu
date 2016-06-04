//
//
//  Created by The Content Team 4/10/216
//  Copyright 2016 High Fidelity, Inc.
//
//
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

var fishTankPath = Script.resolvePath('wrapper.js');
Script.include(fishTankPath);
var center = Vec3.sum(Vec3.sum(MyAvatar.position, {
    x: 0,
    y: 0.5,
    z: 0
}), Vec3.multiply(1, Quat.getFront(Camera.getOrientation())));
var fishTank = new FishTank(center, {
    x: 0,
    y: 0,
    z: 0
});

Script.scriptEnding.connect(function() {
    fishtank.cleanup()
})