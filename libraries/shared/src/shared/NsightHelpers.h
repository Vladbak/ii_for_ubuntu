//
//  Created by Bradley Austin Davis on 2015/12/10
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once
#ifndef hifi_gl_NsightHelpers_h
#define hifi_gl_NsightHelpers_h

bool nsightActive();

#if defined(_WIN32) && defined(NSIGHT_FOUND)
#include <stdint.h>

class ProfileRange {
public:
    ProfileRange(const char *name);
    ProfileRange(const char *name, uint32_t argbColor, uint64_t payload);
    ~ProfileRange();

    static uint64_t beginRange(const char* name, uint32_t argbColor);
    static void endRange(uint64_t rangeId);
private:
    uint64_t _rangeId{ 0 };
};

#define PROFILE_RANGE(name) ProfileRange profileRangeThis(name);
#define PROFILE_RANGE_EX(name, argbColor, payload) ProfileRange profileRangeThis(name, argbColor, (uint64_t)payload);

#define PROFILE_RANGE_BEGIN(rangeId, name, argbColor) rangeId = ProfileRange::beginRange(name, argbColor)
#define PROFILE_RANGE_END(rangeId) ProfileRange::endRange(rangeId)

#else
#define PROFILE_RANGE(name)
#define PROFILE_RANGE_EX(name, argbColor, payload)


#define PROFILE_RANGE_BEGIN(rangeId, name, argbColor)
#define PROFILE_RANGE_END(rangeId)

#endif

#endif
