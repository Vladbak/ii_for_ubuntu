//
//  Created by Bradley Austin Davis on 2016/07/26
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Frame.h"
#include <unordered_set>

using namespace gpu;

Frame::~Frame() {
    if (framebuffer && framebufferRecycler) {
        framebufferRecycler(framebuffer);
        framebuffer.reset();
    }

    assert(bufferUpdates.empty());
    if (!bufferUpdates.empty()) {
        qFatal("Buffer sync error... frame destroyed without buffer updates being applied");
    }
}

void Frame::finish() {
    for (Batch& batch : batches) {
        batch.finishFrame(bufferUpdates);
    }
}

void Frame::preRender() {
    for (auto& update : bufferUpdates) {
        update.apply();
    }
    bufferUpdates.clear();
}
