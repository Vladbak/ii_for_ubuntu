//
//  GL41BackendQuery.cpp
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 7/7/2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "GL41Backend.h"

#include "../gl/GLQuery.h"

using namespace gpu;
using namespace gpu::gl41; 

class GL41Query : public gpu::gl::GLQuery {
    using Parent = gpu::gl::GLBuffer;
public:
    static GLuint allocateQuery() {
        GLuint result;
        glGenQueries(1, &result);
        return result;
    }

    GL41Query(const Query& query) 
        : gl::GLQuery(query, allocateQuery()) { }
};

gl::GLQuery* GL41Backend::syncGPUObject(const Query& query) {
    return GL41Query::sync<GL41Query>(query);
}

GLuint GL41Backend::getQueryID(const QueryPointer& query) {
    return GL41Query::getId<GL41Query>(query);
}
