//
//  Stars.cpp
//  interface/src
//
//  Created by Tobias Schwinger on 3/22/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Stars.h"

#include <mutex>

#include <QElapsedTimer>
#include <gpu/Context.h>
#include <NumericalConstants.h>
#include <DependencyManager.h>
#include <GeometryCache.h>
#include <TextureCache.h>
#include <RenderArgs.h>
#include <ViewFrustum.h>

#include <render-utils/stars_vert.h>
#include <render-utils/stars_frag.h>

#include <render-utils/standardTransformPNTC_vert.h>
#include <render-utils/starsGrid_frag.h>

//static const float TILT = 0.23f;
static const float TILT = 0.0f;
static const unsigned int STARFIELD_NUM_STARS = 50000;
static const unsigned int STARFIELD_SEED = 1;
static const float STAR_COLORIZATION = 0.1f;

static const float TAU = 6.28318530717958f;
//static const float HALF_TAU = TAU / 2.0f;
//static const float QUARTER_TAU = TAU / 4.0f;
//static const float MILKY_WAY_WIDTH = TAU / 30.0f; // width in radians of one half of the Milky Way
//static const float MILKY_WAY_INCLINATION = 0.0f; // angle of Milky Way from horizontal in degrees
//static const float MILKY_WAY_RATIO = 0.4f;
static const char* UNIFORM_TIME_NAME = "iGlobalTime";



Stars::Stars() {
}

Stars::~Stars() {
}

// Produce a random float value between 0 and 1
static float frand() {
    return (float)rand() / (float)RAND_MAX;
}

// Produce a random radian value between 0 and 2 PI (TAU)
/*
static float rrand() {
    return frand() * TAU;
}
 */

// http://mathworld.wolfram.com/SpherePointPicking.html
static vec2 randPolar() {
    vec2 result(frand(), frand());
    result.x *= TAU;
    result.y = powf(result.y, 2.0) / 2.0f;
    if (frand() > 0.5f) {
        result.y = 0.5f - result.y;
    } else {
        result.y += 0.5f;
    }
    result.y = acos((2.0f * result.y) - 1.0f);
    return result;
}


static vec3 fromPolar(const vec2& polar) {
    float sinTheta = sin(polar.x);
    float cosTheta = cos(polar.x);
    float sinPhi = sin(polar.y);
    float cosPhi = cos(polar.y);
    return vec3(
        cosTheta * sinPhi,
        cosPhi,
        sinTheta * sinPhi);
}


// computeStarColor
// - Generate a star color.
//
// colorization can be a value between 0 and 1 specifying how colorful the resulting star color is.
//
// 0 = completely black & white
// 1 = very colorful
unsigned computeStarColor(float colorization) {
    unsigned char red, green, blue;
    if (randFloat() < 0.3f) {
        // A few stars are colorful
        red = 2 + (rand() % 254);
        green = 2 + round((red * (1 - colorization)) + ((rand() % 254) * colorization));
        blue = 2 + round((red * (1 - colorization)) + ((rand() % 254) * colorization));
    } else {
        // Most stars are dimmer and white
        red = green = blue = 2 + (rand() % 128);
    }
    return red | (green << 8) | (blue << 16);
}

struct StarVertex {
    vec4 position;
    vec4 colorAndSize;
};

// FIXME star colors
void Stars::render(RenderArgs* renderArgs, float alpha) {
    static gpu::BufferPointer vertexBuffer;
    static gpu::Stream::FormatPointer streamFormat;
    static gpu::Element positionElement, colorElement;
    static gpu::PipelinePointer _gridPipeline;
    static gpu::PipelinePointer _starsPipeline;
    static int32_t _timeSlot{ -1 };
    static std::once_flag once;

    const int VERTICES_SLOT = 0;
    const int COLOR_SLOT = 1;

    std::call_once(once, [&] {
        {
            auto vs = gpu::Shader::createVertex(std::string(standardTransformPNTC_vert));
            auto ps = gpu::Shader::createPixel(std::string(starsGrid_frag));
            auto program = gpu::Shader::createProgram(vs, ps);
            gpu::Shader::makeProgram((*program));
            _timeSlot = program->getBuffers().findLocation(UNIFORM_TIME_NAME);
            if (_timeSlot == gpu::Shader::INVALID_LOCATION) {
                _timeSlot = program->getUniforms().findLocation(UNIFORM_TIME_NAME);
            }
            auto state = gpu::StatePointer(new gpu::State());
            // enable decal blend
            state->setDepthTest(gpu::State::DepthTest(false));
            state->setStencilTest(true, 0xFF, gpu::State::StencilTest(0, 0xFF, gpu::EQUAL, gpu::State::STENCIL_OP_KEEP, gpu::State::STENCIL_OP_KEEP, gpu::State::STENCIL_OP_KEEP));
            state->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::INV_SRC_ALPHA);
            _gridPipeline = gpu::Pipeline::create(program, state);
        }
        {
            auto vs = gpu::Shader::createVertex(std::string(stars_vert));
            auto ps = gpu::Shader::createPixel(std::string(stars_frag));
            auto program = gpu::Shader::createProgram(vs, ps);
            gpu::Shader::makeProgram((*program));
            auto state = gpu::StatePointer(new gpu::State());
            // enable decal blend
            state->setDepthTest(gpu::State::DepthTest(false));
            state->setStencilTest(true, 0xFF, gpu::State::StencilTest(0, 0xFF, gpu::EQUAL, gpu::State::STENCIL_OP_KEEP, gpu::State::STENCIL_OP_KEEP, gpu::State::STENCIL_OP_KEEP));
            state->setAntialiasedLineEnable(true); // line smoothing also smooth points
            state->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::INV_SRC_ALPHA);
            _starsPipeline = gpu::Pipeline::create(program, state);
            
        }

        QElapsedTimer startTime;
        startTime.start();
        vertexBuffer.reset(new gpu::Buffer);

        srand(STARFIELD_SEED);
        unsigned limit = STARFIELD_NUM_STARS;
        std::vector<StarVertex> points;
        points.resize(limit);
        for (size_t star = 0; star < limit; ++star) {
            points[star].position = vec4(fromPolar(randPolar()), 1);
            float size = frand() * 2.5f + 0.5f;
            if (frand() < STAR_COLORIZATION) {
                vec3 color(frand() / 2.0f + 0.5f, frand() / 2.0f + 0.5f, frand() / 2.0f + 0.5f);
                points[star].colorAndSize = vec4(color, size);
            } else {
                vec3 color(frand() / 2.0f + 0.5f);
                points[star].colorAndSize = vec4(color, size);
            }
        }
        double timeDiff = (double)startTime.nsecsElapsed() / 1000000.0; // ns to ms
        qDebug() << "Total time to generate stars: " << timeDiff << " msec";

        vertexBuffer->append(sizeof(StarVertex) * limit, (const gpu::Byte*)&points[0]);
        streamFormat.reset(new gpu::Stream::Format()); // 1 for everyone
        streamFormat->setAttribute(gpu::Stream::POSITION, VERTICES_SLOT, gpu::Element(gpu::VEC4, gpu::FLOAT, gpu::XYZW), 0);
        streamFormat->setAttribute(gpu::Stream::COLOR, COLOR_SLOT, gpu::Element(gpu::VEC4, gpu::FLOAT, gpu::RGBA));
        positionElement = streamFormat->getAttributes().at(gpu::Stream::POSITION)._element;
        colorElement = streamFormat->getAttributes().at(gpu::Stream::COLOR)._element;
    });

    auto modelCache = DependencyManager::get<ModelCache>();
    auto textureCache = DependencyManager::get<TextureCache>();
    auto geometryCache = DependencyManager::get<GeometryCache>();


    gpu::Batch& batch = *renderArgs->_batch;
    batch.setViewTransform(Transform());
    batch.setProjectionTransform(renderArgs->_viewFrustum->getProjection());
    batch.setModelTransform(Transform().setRotation(glm::inverse(renderArgs->_viewFrustum->getOrientation()) *
        quat(vec3(TILT, 0, 0))));
    batch.setResourceTexture(0, textureCache->getWhiteTexture());

    // Render the world lines
    batch.setPipeline(_gridPipeline);
    static auto start = usecTimestampNow();
    float msecs = (float)(usecTimestampNow() - start) / (float)USECS_PER_MSEC;
    float secs = msecs / (float)MSECS_PER_SECOND;
    batch._glUniform1f(_timeSlot, secs);
    geometryCache->renderCube(batch);

    static const size_t VERTEX_STRIDE = sizeof(StarVertex);
    size_t offset = offsetof(StarVertex, position);
    gpu::BufferView posView(vertexBuffer, offset, vertexBuffer->getSize(), VERTEX_STRIDE, positionElement);
    offset = offsetof(StarVertex, colorAndSize);
    gpu::BufferView colView(vertexBuffer, offset, vertexBuffer->getSize(), VERTEX_STRIDE, colorElement);
    
    // Render the stars
    batch.setPipeline(_starsPipeline);

    batch.setInputFormat(streamFormat);
    batch.setInputBuffer(VERTICES_SLOT, posView);
    batch.setInputBuffer(COLOR_SLOT, colView);
    batch.draw(gpu::Primitive::POINTS, STARFIELD_NUM_STARS);
}
