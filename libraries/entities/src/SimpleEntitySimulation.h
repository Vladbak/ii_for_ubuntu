//
//  SimpleEntitySimulation.h
//  libraries/entities/src
//
//  Created by Andrew Meadows on 2014.11.24
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SimpleEntitySimulation_h
#define hifi_SimpleEntitySimulation_h

#include "EntitySimulation.h"

/// provides simple velocity + gravity extrapolation of EntityItem's

class SimpleEntitySimulation : public EntitySimulation {
public:
    SimpleEntitySimulation() : EntitySimulation() { }
    virtual ~SimpleEntitySimulation() { clearEntitiesInternal(); }

protected:
    virtual void updateEntitiesInternal(const quint64& now) override;
    virtual void addEntityInternal(EntityItemPointer entity) override;
    virtual void removeEntityInternal(EntityItemPointer entity) override;
    virtual void changeEntityInternal(EntityItemPointer entity) override;
    virtual void clearEntitiesInternal() override;

    SetOfEntities _entitiesWithSimulator;
    quint64 _nextSimulationExpiry { 0 };
};

#endif // hifi_SimpleEntitySimulation_h
