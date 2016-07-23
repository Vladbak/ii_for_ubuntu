//
//  Created by Bradley Austin Davis on 2016/05/15
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_gl_GLQuery_h
#define hifi_gpu_gl_GLQuery_h

#include "GLShared.h"

namespace gpu { namespace gl {

class GLQuery : public GLObject<Query> {
    using Parent = gpu::gl::GLObject<Query>;
public:
    template <typename GLQueryType>
    static GLQueryType* sync(const Query& query) {
        GLQueryType* object = Backend::getGPUObject<GLQueryType>(query);

        // need to have a gpu object?
        if (!object) {
            // All is green, assign the gpuobject to the Query
            object = new GLQueryType(query);
            (void)CHECK_GL_ERROR();
            Backend::setGPUObject(query, object);
        }

        return object;
    }

    template <typename GLQueryType>
    static GLuint getId(const QueryPointer& query) {
        if (!query) {
            return 0;
        }

        GLQuery* object = sync<GLQueryType>(*query);
        if (!object) {
            return 0;
        } 

        return object->_endqo;
    }

    const GLuint& _endqo = { _id };
    const GLuint _beginqo = { 0 };
    GLuint64 _result { (GLuint64)-1 };

protected:
    GLQuery(const Query& query, GLuint endId, GLuint beginId) : Parent(query, endId), _beginqo(beginId){}
    ~GLQuery() {
        if (_id) {
            GLuint ids[2] = { _endqo, _beginqo };
            glDeleteQueries(2, ids);
        }
    }
};

} }

#endif
