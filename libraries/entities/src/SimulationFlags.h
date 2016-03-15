//
//  SimulationFlags.h
//  libraries/physics/src
//
//  Created by Andrew Meadows 2015.10.14
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SimulationFlags_h
#define hifi_SimulationFlags_h

namespace Simulation {
    const uint32_t DIRTY_POSITION = 0x0001;
    const uint32_t DIRTY_ROTATION = 0x0002;
    const uint32_t DIRTY_LINEAR_VELOCITY = 0x0004;
    const uint32_t DIRTY_ANGULAR_VELOCITY = 0x0008;
    const uint32_t DIRTY_MASS = 0x0010;
    const uint32_t DIRTY_COLLISION_GROUP = 0x0020;
    const uint32_t DIRTY_MOTION_TYPE = 0x0040;
    const uint32_t DIRTY_SHAPE = 0x0080;
    const uint32_t DIRTY_LIFETIME = 0x0100;
    const uint32_t DIRTY_UPDATEABLE = 0x0200;
    const uint32_t DIRTY_MATERIAL = 0x00400;
    const uint32_t DIRTY_PHYSICS_ACTIVATION = 0x0800; // should activate object in physics engine
    const uint32_t DIRTY_SIMULATOR_ID = 0x1000; // the simulatorID has changed
    const uint32_t DIRTY_SIMULATION_OWNERSHIP_FOR_POKE = 0x2000; // bid for simulation ownership at "poke"
    const uint32_t DIRTY_SIMULATION_OWNERSHIP_FOR_GRAB = 0x4000; // bid for simulation ownership at "grab"

    const uint32_t DIRTY_TRANSFORM = DIRTY_POSITION | DIRTY_ROTATION;
    const uint32_t DIRTY_VELOCITIES = DIRTY_LINEAR_VELOCITY | DIRTY_ANGULAR_VELOCITY;
    const uint32_t DIRTY_SIMULATION_OWNERSHIP_PRIORITY = DIRTY_SIMULATION_OWNERSHIP_FOR_POKE | DIRTY_SIMULATION_OWNERSHIP_FOR_GRAB;
};

#endif // hifi_SimulationFlags_h
