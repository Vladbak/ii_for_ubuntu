//
//  DeferredLightingEffect.h
//  interface/src/renderer
//
//  Created by Andrzej Kapolka on 9/11/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_DeferredLightingEffect_h
#define hifi_DeferredLightingEffect_h

#include <QVector>

#include <DependencyManager.h>
#include <NumericalConstants.h>

#include "model/Light.h"
#include "model/Stage.h"
#include "model/Geometry.h"

#include "LightStage.h"

class RenderArgs;
struct LightLocations;
using LightLocationsPtr = std::shared_ptr<LightLocations>;
/// Handles deferred lighting for the bits that require it (voxels...)
class DeferredLightingEffect : public Dependency {
    SINGLETON_DEPENDENCY
    
public:
    void init();
    
    /// Adds a point light to render for the current frame.
    void addPointLight(const glm::vec3& position, float radius, const glm::vec3& color = glm::vec3(0.0f, 0.0f, 0.0f),
        float intensity = 0.5f);
        
    /// Adds a spot light to render for the current frame.
    void addSpotLight(const glm::vec3& position, float radius, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        float intensity = 0.5f, const glm::quat& orientation = glm::quat(), float exponent = 0.0f, float cutoff = PI);
    
    void prepare(RenderArgs* args);
    void render(RenderArgs* args);

    void setupTransparent(RenderArgs* args, int lightBufferUnit);

    // update global lighting
    void setAmbientLightMode(int preset);
    void setGlobalLight(const glm::vec3& direction, const glm::vec3& diffuse, float intensity, float ambientIntensity);
    void setGlobalAtmosphere(const model::AtmospherePointer& atmosphere) { _atmosphere = atmosphere; }
    void setGlobalSkybox(const model::SkyboxPointer& skybox);

    const LightStage& getLightStage() { return _lightStage; }

private:
    LightStage _lightStage;

    DeferredLightingEffect() = default;

    model::MeshPointer _spotLightMesh;
    model::MeshPointer getSpotLightMesh();
    
    gpu::PipelinePointer _directionalSkyboxLight;
    LightLocationsPtr _directionalSkyboxLightLocations;

    gpu::PipelinePointer _directionalAmbientSphereLight;
    LightLocationsPtr _directionalAmbientSphereLightLocations;

    gpu::PipelinePointer _directionalLight;
    LightLocationsPtr _directionalLightLocations;

    gpu::PipelinePointer _pointLight;
    LightLocationsPtr _pointLightLocations;
    gpu::PipelinePointer _spotLight;
    LightLocationsPtr _spotLightLocations;

    using Lights = std::vector<model::LightPointer>;

    Lights _allocatedLights;
    std::vector<int> _globalLights;
    std::vector<int> _pointLights;
    std::vector<int> _spotLights;

    int _ambientLightMode = 0;
    model::AtmospherePointer _atmosphere;
    model::SkyboxPointer _skybox;

    // Class describing the uniform buffer with all the parameters common to the deferred shaders
    class DeferredTransform {
    public:
        glm::mat4 projection;
        glm::mat4 viewInverse;
        float stereoSide{ 0.f };
        float spareA, spareB, spareC;

        DeferredTransform() {}
    };
    typedef gpu::BufferView UniformBufferView;
    UniformBufferView _deferredTransformBuffer[2];
};

#endif // hifi_DeferredLightingEffect_h
