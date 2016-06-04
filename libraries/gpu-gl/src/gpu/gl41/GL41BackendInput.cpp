//
//  GL41BackendInput.cpp
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 3/8/2015.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "GL41Backend.h"

using namespace gpu;
using namespace gpu::gl41;

// Core 41 doesn't expose the features to really separate the vertex format from the vertex buffers binding
// Core 43 does :)
// FIXME crashing problem with glVertexBindingDivisor / glVertexAttribFormat
#if 1 || (GPU_INPUT_PROFILE == GPU_CORE_41)
#define NO_SUPPORT_VERTEX_ATTRIB_FORMAT
#else
#define SUPPORT_VERTEX_ATTRIB_FORMAT
#endif

void GL41Backend::updateInput() {
#if defined(SUPPORT_VERTEX_ATTRIB_FORMAT)
    if (_input._invalidFormat) {

        InputStageState::ActivationCache newActivation;

        // Assign the vertex format required
        if (_input._format) {
            for (auto& it : _input._format->getAttributes()) {
                const Stream::Attribute& attrib = (it).second;

                GLuint slot = attrib._slot;
                GLuint count = attrib._element.getLocationScalarCount();
                uint8_t locationCount = attrib._element.getLocationCount();
                GLenum type = _elementTypeToGL41Type[attrib._element.getType()];
                GLuint offset = attrib._offset;;
                GLboolean isNormalized = attrib._element.isNormalized();

                GLenum perLocationSize = attrib._element.getLocationSize();

                for (size_t locNum = 0; locNum < locationCount; ++locNum) {
                    newActivation.set(slot + locNum);
                    glVertexAttribFormat(slot + locNum, count, type, isNormalized, offset + locNum * perLocationSize);
                    glVertexAttribBinding(slot + locNum, attrib._channel);
                }
                glVertexBindingDivisor(attrib._channel, attrib._frequency);
            }
            (void) CHECK_GL_ERROR();
        }

        // Manage Activation what was and what is expected now
        for (size_t i = 0; i < newActivation.size(); i++) {
            bool newState = newActivation[i];
            if (newState != _input._attributeActivation[i]) {
                if (newState) {
                    glEnableVertexAttribArray(i);
                } else {
                    glDisableVertexAttribArray(i);
                }
                _input._attributeActivation.flip(i);
            }
        }
        (void) CHECK_GL_ERROR();

        _input._invalidFormat = false;
        _stats._ISNumFormatChanges++;
    }

    if (_input._invalidBuffers.any()) {
        int numBuffers = _input._buffers.size();
        auto buffer = _input._buffers.data();
        auto vbo = _input._bufferVBOs.data();
        auto offset = _input._bufferOffsets.data();
        auto stride = _input._bufferStrides.data();

        for (int bufferNum = 0; bufferNum < numBuffers; bufferNum++) {
            if (_input._invalidBuffers.test(bufferNum)) {
                glBindVertexBuffer(bufferNum, (*vbo), (*offset), (*stride));
            }
            buffer++;
            vbo++;
            offset++;
            stride++;
        }
        _input._invalidBuffers.reset();
        (void) CHECK_GL_ERROR();
    }
#else
    if (_input._invalidFormat || _input._invalidBuffers.any()) {

        if (_input._invalidFormat) {
            InputStageState::ActivationCache newActivation;

            _stats._ISNumFormatChanges++;

            // Check expected activation
            if (_input._format) {
                for (auto& it : _input._format->getAttributes()) {
                    const Stream::Attribute& attrib = (it).second;
                    uint8_t locationCount = attrib._element.getLocationCount();
                    for (int i = 0; i < locationCount; ++i) {
                        newActivation.set(attrib._slot + i);
                    }
                }
            }
            
            // Manage Activation what was and what is expected now
            for (unsigned int i = 0; i < newActivation.size(); i++) {
                bool newState = newActivation[i];
                if (newState != _input._attributeActivation[i]) {

                    if (newState) {
                        glEnableVertexAttribArray(i);
                    } else {
                        glDisableVertexAttribArray(i);
                    }
                    (void) CHECK_GL_ERROR();
                    
                    _input._attributeActivation.flip(i);
                }
            }
        }

        // now we need to bind the buffers and assign the attrib pointers
        if (_input._format) {
            const Buffers& buffers = _input._buffers;
            const Offsets& offsets = _input._bufferOffsets;
            const Offsets& strides = _input._bufferStrides;

            const Stream::Format::AttributeMap& attributes = _input._format->getAttributes();
            auto& inputChannels = _input._format->getChannels();
            _stats._ISNumInputBufferChanges++;

            GLuint boundVBO = 0;
            for (auto& channelIt : inputChannels) {
                const Stream::Format::ChannelMap::value_type::second_type& channel = (channelIt).second;
                if ((channelIt).first < buffers.size()) {
                    int bufferNum = (channelIt).first;

                    if (_input._invalidBuffers.test(bufferNum) || _input._invalidFormat) {
                      //  GLuint vbo = gpu::GL41Backend::getBufferID((*buffers[bufferNum]));
                        GLuint vbo = _input._bufferVBOs[bufferNum];
                        if (boundVBO != vbo) {
                            glBindBuffer(GL_ARRAY_BUFFER, vbo);
                            (void) CHECK_GL_ERROR();
                            boundVBO = vbo;
                        }
                        _input._invalidBuffers[bufferNum] = false;

                        for (unsigned int i = 0; i < channel._slots.size(); i++) {
                            const Stream::Attribute& attrib = attributes.at(channel._slots[i]);
                            GLuint slot = attrib._slot;
                            GLuint count = attrib._element.getLocationScalarCount();
                            uint8_t locationCount = attrib._element.getLocationCount();
                            GLenum type = gl::ELEMENT_TYPE_TO_GL[attrib._element.getType()];
                            // GLenum perLocationStride = strides[bufferNum];
                            GLenum perLocationStride = attrib._element.getLocationSize();
                            GLuint stride = (GLuint)strides[bufferNum];
                            GLuint pointer = (GLuint)(attrib._offset + offsets[bufferNum]);
                            GLboolean isNormalized = attrib._element.isNormalized();

                            for (size_t locNum = 0; locNum < locationCount; ++locNum) {
                                glVertexAttribPointer(slot + (GLuint)locNum, count, type, isNormalized, stride,
                                    reinterpret_cast<GLvoid*>(pointer + perLocationStride * (GLuint)locNum));
                                glVertexAttribDivisor(slot + (GLuint)locNum, attrib._frequency);
                            }
                            
                            // TODO: Support properly the IAttrib version

                            (void) CHECK_GL_ERROR();
                        }
                    }
                }
            }
        }
        // everything format related should be in sync now
        _input._invalidFormat = false;
    }
#endif
}

