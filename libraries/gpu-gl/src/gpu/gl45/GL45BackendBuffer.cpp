//
//  Created by Bradley Austin Davis on 2016/05/15
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "GL45Backend.h"
#include "../gl/GLBuffer.h"

namespace gpu { namespace gl45 {
    using namespace gpu::gl;

    class GL45Buffer : public GLBuffer {
        using Parent = GLBuffer;
        static GLuint allocate() {
            GLuint result;
            glCreateBuffers(1, &result);
            return result;
        }

    public:
        GL45Buffer(const std::weak_ptr<GLBackend>& backend, const Buffer& buffer, GLBuffer* original) : Parent(backend, buffer, allocate()) {
            glNamedBufferStorage(_buffer, _size == 0 ? 256 : _size, nullptr, GL_DYNAMIC_STORAGE_BIT);
            if (original && original->_size) {
                glCopyNamedBufferSubData(original->_buffer, _buffer, 0, 0, std::min(original->_size, _size));
            }
            Backend::setGPUObject(buffer, this);
        }

        void transfer() override {
            Size offset;
            Size size;
            Size currentPage { 0 };
            auto data = _gpuObject._renderSysmem.readData();
            while (_gpuObject._renderPages.getNextTransferBlock(offset, size, currentPage)) {
                glNamedBufferSubData(_buffer, (GLintptr)offset, (GLsizeiptr)size, data + offset);
            }
            (void)CHECK_GL_ERROR();
            _gpuObject._renderPages._flags &= ~PageManager::DIRTY;
        }
    };
} }

using namespace gpu;
using namespace gpu::gl;
using namespace gpu::gl45;


GLuint GL45Backend::getBufferID(const Buffer& buffer) {
    return GL45Buffer::getId<GL45Buffer>(*this, buffer);
}

GLBuffer* GL45Backend::syncGPUObject(const Buffer& buffer) {
    return GL45Buffer::sync<GL45Buffer>(*this, buffer);
}
