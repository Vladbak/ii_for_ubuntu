//
//  Engine.h
//  render/src/render
//
//  Created by Sam Gateau on 3/3/15.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_Engine_h
#define hifi_render_Engine_h

#include "Context.h"
#include "Task.h"

namespace render {

// The render engine holds all render tasks, and is itself a render task.
// State flows through tasks to jobs via the render and scene contexts -
// the engine should not be known from its jobs.
class Engine : public Task {
public:
    Engine();
    ~Engine() = default;

    // Register the scene
    void registerScene(const ScenePointer& scene) { _sceneContext->_scene = scene; }

    // Push a RenderContext
    void setRenderContext(const RenderContext& renderContext) { (*_renderContext) = renderContext; }
    RenderContextPointer getRenderContext() const { return _renderContext; }

    // Render a frame
    // A frame must have a scene registered and a context set to render
    void run();

protected:
    SceneContextPointer _sceneContext;
    RenderContextPointer _renderContext;
};
using EnginePointer = std::shared_ptr<Engine>;

}

#endif // hifi_render_Engine_h
