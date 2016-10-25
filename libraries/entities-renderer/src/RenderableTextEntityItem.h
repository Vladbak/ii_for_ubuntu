//
//  RenderableTextEntityItem.h
//  interface/src/entities
//
//  Created by Brad Hefta-Gaub on 8/6/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_RenderableTextEntityItem_h
#define hifi_RenderableTextEntityItem_h

#include <TextEntityItem.h>
#include <TextRenderer3D.h>

#include "RenderableEntityItem.h"

const int FIXED_FONT_POINT_SIZE = 40;

class RenderableTextEntityItem : public TextEntityItem  {
public:
    static EntityItemPointer factory(const EntityItemID& entityID, const EntityItemProperties& properties);
    RenderableTextEntityItem(const EntityItemID& entityItemID) : TextEntityItem(entityItemID) { }
    ~RenderableTextEntityItem();

    virtual void render(RenderArgs* args) override;

    SIMPLE_RENDERABLE();
    
private:
    int _geometryID { 0 };
    TextRenderer3D* _textRenderer = TextRenderer3D::getInstance(SANS_FONT_FAMILY, FIXED_FONT_POINT_SIZE / 2.0f);
};


#endif // hifi_RenderableTextEntityItem_h
