//
//  DebugDeferredBuffer.h
//  libraries/render-utils/src
//
//  Created by Clement on 12/3/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_DebugDeferredBuffer_h
#define hifi_DebugDeferredBuffer_h

#include <QFileInfo>

#include <render/DrawTask.h>

class DebugDeferredBufferConfig : public render::Job::Config {
    Q_OBJECT
    Q_PROPERTY(bool enabled MEMBER enabled)
    Q_PROPERTY(int mode MEMBER mode WRITE setMode)
    Q_PROPERTY(glm::vec4 size MEMBER size NOTIFY dirty)
public:
    DebugDeferredBufferConfig() : render::Job::Config(false) {}

    void setMode(int newMode);
 
    int mode{ 0 };
    glm::vec4 size{ 0.0f, 0.0f, 0.0f, 0.0f };
signals:
    void dirty();
};

class DebugDeferredBuffer {
public:
    using Config = DebugDeferredBufferConfig;
    using JobModel = render::Job::Model<DebugDeferredBuffer, Config>;
    
    DebugDeferredBuffer();

    void configure(const Config& config);
    void run(const render::SceneContextPointer& sceneContext, const render::RenderContextPointer& renderContext);
    
protected:
    friend class DebugDeferredBufferConfig;

    enum Mode : uint8_t {
        // Use Mode suffix to avoid collisions
        DiffuseMode = 0,
        SpecularMode,
        RoughnessMode,
        NormalMode,
        DepthMode,
        LightingMode,
        ShadowMode,
        PyramidDepthMode,
        AmbientOcclusionMode,
        AmbientOcclusionBlurredMode,
        CustomMode // Needs to stay last
    };

private:
    Mode _mode;
    glm::vec4 _size;

    struct CustomPipeline {
        gpu::PipelinePointer pipeline;
        mutable QFileInfo info;
    };
    using StandardPipelines = std::array<gpu::PipelinePointer, CustomMode>;
    using CustomPipelines = std::unordered_map<std::string, CustomPipeline>;
    
    bool pipelineNeedsUpdate(Mode mode, std::string customFile = std::string()) const;
    const gpu::PipelinePointer& getPipeline(Mode mode, std::string customFile = std::string());
    std::string getShaderSourceCode(Mode mode, std::string customFile = std::string());
    
    StandardPipelines _pipelines;
    CustomPipelines _customPipelines;
};

#endif // hifi_DebugDeferredBuffer_h