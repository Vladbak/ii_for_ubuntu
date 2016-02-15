//
//  AudioHRTF.cpp
//  libraries/audio/src
//
//  Created by Ken Cooke on 1/17/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "AudioHRTF.h"
#include "AudioHRTFData.h"

//
// Equal-gain crossfade
//
// Cos(x)^2 window minimizes the modulation sidebands when a pure tone is panned.
// Transients in the time-varying Thiran allpass filter are eliminated by the initial delay.
// Valimaki, Laakso. "Elimination of Transients in Time-Varying Allpass Fractional Delay Filters"
//
static const float crossfadeTable[HRTF_BLOCK] = {
    1.0000000000f, 1.0000000000f, 1.0000000000f, 1.0000000000f, 1.0000000000f, 0.9999611462f, 0.9998445910f, 0.9996503524f, 
    0.9993784606f, 0.9990289579f, 0.9986018986f, 0.9980973490f, 0.9975153877f, 0.9968561049f, 0.9961196033f, 0.9953059972f, 
    0.9944154131f, 0.9934479894f, 0.9924038765f, 0.9912832366f, 0.9900862439f, 0.9888130845f, 0.9874639561f, 0.9860390685f, 
    0.9845386431f, 0.9829629131f, 0.9813121235f, 0.9795865307f, 0.9777864029f, 0.9759120199f, 0.9739636731f, 0.9719416652f, 
    0.9698463104f, 0.9676779344f, 0.9654368743f, 0.9631234783f, 0.9607381059f, 0.9582811279f, 0.9557529262f, 0.9531538935f, 
    0.9504844340f, 0.9477449623f, 0.9449359044f, 0.9420576968f, 0.9391107867f, 0.9360956322f, 0.9330127019f, 0.9298624749f, 
    0.9266454408f, 0.9233620996f, 0.9200129616f, 0.9165985472f, 0.9131193872f, 0.9095760221f, 0.9059690029f, 0.9022988899f, 
    0.8985662536f, 0.8947716742f, 0.8909157412f, 0.8869990541f, 0.8830222216f, 0.8789858616f, 0.8748906015f, 0.8707370778f, 
    0.8665259359f, 0.8622578304f, 0.8579334246f, 0.8535533906f, 0.8491184090f, 0.8446291692f, 0.8400863689f, 0.8354907140f, 
    0.8308429188f, 0.8261437056f, 0.8213938048f, 0.8165939546f, 0.8117449009f, 0.8068473974f, 0.8019022052f, 0.7969100928f, 
    0.7918718361f, 0.7867882182f, 0.7816600290f, 0.7764880657f, 0.7712731319f, 0.7660160383f, 0.7607176017f, 0.7553786457f, 
    0.7500000000f, 0.7445825006f, 0.7391269893f, 0.7336343141f, 0.7281053287f, 0.7225408922f, 0.7169418696f, 0.7113091309f, 
    0.7056435516f, 0.6999460122f, 0.6942173981f, 0.6884585998f, 0.6826705122f, 0.6768540348f, 0.6710100717f, 0.6651395310f, 
    0.6592433251f, 0.6533223705f, 0.6473775872f, 0.6414098993f, 0.6354202341f, 0.6294095226f, 0.6233786988f, 0.6173287002f, 
    0.6112604670f, 0.6051749422f, 0.5990730716f, 0.5929558036f, 0.5868240888f, 0.5806788803f, 0.5745211331f, 0.5683518042f, 
    0.5621718523f, 0.5559822381f, 0.5497839233f, 0.5435778714f, 0.5373650468f, 0.5311464151f, 0.5249229428f, 0.5186955971f, 
    0.5124653459f, 0.5062331573f, 0.5000000000f, 0.4937668427f, 0.4875346541f, 0.4813044029f, 0.4750770572f, 0.4688535849f, 
    0.4626349532f, 0.4564221286f, 0.4502160767f, 0.4440177619f, 0.4378281477f, 0.4316481958f, 0.4254788669f, 0.4193211197f, 
    0.4131759112f, 0.4070441964f, 0.4009269284f, 0.3948250578f, 0.3887395330f, 0.3826712998f, 0.3766213012f, 0.3705904774f, 
    0.3645797659f, 0.3585901007f, 0.3526224128f, 0.3466776295f, 0.3407566749f, 0.3348604690f, 0.3289899283f, 0.3231459652f, 
    0.3173294878f, 0.3115414002f, 0.3057826019f, 0.3000539878f, 0.2943564484f, 0.2886908691f, 0.2830581304f, 0.2774591078f, 
    0.2718946713f, 0.2663656859f, 0.2608730107f, 0.2554174994f, 0.2500000000f, 0.2446213543f, 0.2392823983f, 0.2339839617f, 
    0.2287268681f, 0.2235119343f, 0.2183399710f, 0.2132117818f, 0.2081281639f, 0.2030899072f, 0.1980977948f, 0.1931526026f, 
    0.1882550991f, 0.1834060454f, 0.1786061952f, 0.1738562944f, 0.1691570812f, 0.1645092860f, 0.1599136311f, 0.1553708308f, 
    0.1508815910f, 0.1464466094f, 0.1420665754f, 0.1377421696f, 0.1334740641f, 0.1292629222f, 0.1251093985f, 0.1210141384f, 
    0.1169777784f, 0.1130009459f, 0.1090842588f, 0.1052283258f, 0.1014337464f, 0.0977011101f, 0.0940309971f, 0.0904239779f, 
    0.0868806128f, 0.0834014528f, 0.0799870384f, 0.0766379004f, 0.0733545592f, 0.0701375251f, 0.0669872981f, 0.0639043678f, 
    0.0608892133f, 0.0579423032f, 0.0550640956f, 0.0522550377f, 0.0495155660f, 0.0468461065f, 0.0442470738f, 0.0417188721f, 
    0.0392618941f, 0.0368765217f, 0.0345631257f, 0.0323220656f, 0.0301536896f, 0.0280583348f, 0.0260363269f, 0.0240879801f, 
    0.0222135971f, 0.0204134693f, 0.0186878765f, 0.0170370869f, 0.0154613569f, 0.0139609315f, 0.0125360439f, 0.0111869155f, 
    0.0099137561f, 0.0087167634f, 0.0075961235f, 0.0065520106f, 0.0055845869f, 0.0046940028f, 0.0038803967f, 0.0031438951f, 
    0.0024846123f, 0.0019026510f, 0.0013981014f, 0.0009710421f, 0.0006215394f, 0.0003496476f, 0.0001554090f, 0.0000388538f, 
};

static const float TWOPI = 6.283185307f;

//
// on x86 architecture, assume that SSE2 is present
//
#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)

#include <emmintrin.h>

// 1 channel input, 4 channel output
static void FIR_1x4_SSE(float* src, float* dst0, float* dst1, float* dst2, float* dst3, float coef[4][HRTF_TAPS], int numFrames) {

    float* coef0 = coef[0] + HRTF_TAPS - 1;     // process backwards
    float* coef1 = coef[1] + HRTF_TAPS - 1;
    float* coef2 = coef[2] + HRTF_TAPS - 1;
    float* coef3 = coef[3] + HRTF_TAPS - 1;

    assert(numFrames % 4 == 0);

    for (int i = 0; i < numFrames; i += 4) {

        __m128 acc0 = _mm_setzero_ps();
        __m128 acc1 = _mm_setzero_ps();
        __m128 acc2 = _mm_setzero_ps();
        __m128 acc3 = _mm_setzero_ps();

        float* ps = &src[i - HRTF_TAPS + 1];    // process forwards

        assert(HRTF_TAPS % 4 == 0);

        for (int k = 0; k < HRTF_TAPS; k += 4) {

            acc0 = _mm_add_ps(acc0, _mm_mul_ps(_mm_load1_ps(&coef0[-k-0]), _mm_loadu_ps(&ps[k+0])));
            acc1 = _mm_add_ps(acc1, _mm_mul_ps(_mm_load1_ps(&coef1[-k-0]), _mm_loadu_ps(&ps[k+0])));
            acc2 = _mm_add_ps(acc2, _mm_mul_ps(_mm_load1_ps(&coef2[-k-0]), _mm_loadu_ps(&ps[k+0])));
            acc3 = _mm_add_ps(acc3, _mm_mul_ps(_mm_load1_ps(&coef3[-k-0]), _mm_loadu_ps(&ps[k+0])));

            acc0 = _mm_add_ps(acc0, _mm_mul_ps(_mm_load1_ps(&coef0[-k-1]), _mm_loadu_ps(&ps[k+1])));
            acc1 = _mm_add_ps(acc1, _mm_mul_ps(_mm_load1_ps(&coef1[-k-1]), _mm_loadu_ps(&ps[k+1])));
            acc2 = _mm_add_ps(acc2, _mm_mul_ps(_mm_load1_ps(&coef2[-k-1]), _mm_loadu_ps(&ps[k+1])));
            acc3 = _mm_add_ps(acc3, _mm_mul_ps(_mm_load1_ps(&coef3[-k-1]), _mm_loadu_ps(&ps[k+1])));

            acc0 = _mm_add_ps(acc0, _mm_mul_ps(_mm_load1_ps(&coef0[-k-2]), _mm_loadu_ps(&ps[k+2])));
            acc1 = _mm_add_ps(acc1, _mm_mul_ps(_mm_load1_ps(&coef1[-k-2]), _mm_loadu_ps(&ps[k+2])));
            acc2 = _mm_add_ps(acc2, _mm_mul_ps(_mm_load1_ps(&coef2[-k-2]), _mm_loadu_ps(&ps[k+2])));
            acc3 = _mm_add_ps(acc3, _mm_mul_ps(_mm_load1_ps(&coef3[-k-2]), _mm_loadu_ps(&ps[k+2])));

            acc0 = _mm_add_ps(acc0, _mm_mul_ps(_mm_load1_ps(&coef0[-k-3]), _mm_loadu_ps(&ps[k+3])));
            acc1 = _mm_add_ps(acc1, _mm_mul_ps(_mm_load1_ps(&coef1[-k-3]), _mm_loadu_ps(&ps[k+3])));
            acc2 = _mm_add_ps(acc2, _mm_mul_ps(_mm_load1_ps(&coef2[-k-3]), _mm_loadu_ps(&ps[k+3])));
            acc3 = _mm_add_ps(acc3, _mm_mul_ps(_mm_load1_ps(&coef3[-k-3]), _mm_loadu_ps(&ps[k+3])));
        }

        _mm_storeu_ps(&dst0[i], acc0);
        _mm_storeu_ps(&dst1[i], acc1);
        _mm_storeu_ps(&dst2[i], acc2);
        _mm_storeu_ps(&dst3[i], acc3);
    }
}

//
// Detect AVX/AVX2 support
//

#if defined(_MSC_VER)

#include <intrin.h>

static bool cpuSupportsAVX() {
    int info[4];
    int mask = (1 << 27) | (1 << 28);   // OSXSAVE and AVX

    __cpuidex(info, 0x1, 0);

    bool result = false;
    if ((info[2] & mask) == mask) {

        if ((_xgetbv(_XCR_XFEATURE_ENABLED_MASK) & 0x6) == 0x6) {
            result = true;
        }
    }
    return result;
}

#elif defined(__GNUC__)

#include <cpuid.h>

static bool cpuSupportsAVX() {
    unsigned int eax, ebx, ecx, edx;
    unsigned int mask = (1 << 27) | (1 << 28);   // OSXSAVE and AVX

    bool result = false;
    if (__get_cpuid(0x1, &eax, &ebx, &ecx, &edx) && ((ecx & mask) == mask)) {

        __asm__("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
        if ((eax & 0x6) == 0x6) {
            result = true;
        }
    }
    return result;    
}

#else

static bool cpuSupportsAVX() {
    return false;
}

#endif

//
// Runtime CPU dispatch
//

typedef void FIR_1x4_t(float* src, float* dst0, float* dst1, float* dst2, float* dst3, float coef[4][HRTF_TAPS], int numFrames);
FIR_1x4_t FIR_1x4_AVX;  // separate compilation with VEX-encoding enabled

static void FIR_1x4(float* src, float* dst0, float* dst1, float* dst2, float* dst3, float coef[4][HRTF_TAPS], int numFrames) {

    static FIR_1x4_t* f = cpuSupportsAVX() ? FIR_1x4_AVX : FIR_1x4_SSE; // init on first call
    (*f)(src, dst0, dst1, dst2, dst3, coef, numFrames);                 // dispatch
}

// 4 channel planar to interleaved
static void interleave_4x4(float* src0, float* src1, float* src2, float* src3, float* dst, int numFrames) {

    assert(numFrames % 4 == 0);

    for (int i = 0; i < numFrames; i += 4) {

        __m128 x0 = _mm_loadu_ps(&src0[i]);
        __m128 x1 = _mm_loadu_ps(&src1[i]);
        __m128 x2 = _mm_loadu_ps(&src2[i]);
        __m128 x3 = _mm_loadu_ps(&src3[i]);

        // interleave (4x4 matrix transpose)
        __m128 t0 = _mm_unpacklo_ps(x0, x1);
        __m128 t2 = _mm_unpacklo_ps(x2, x3);
        __m128 t1 = _mm_unpackhi_ps(x0, x1);
        __m128 t3 = _mm_unpackhi_ps(x2, x3);

        x0 = _mm_movelh_ps(t0, t2);
        x1 = _mm_movehl_ps(t2, t0);
        x2 = _mm_movelh_ps(t1, t3);
        x3 = _mm_movehl_ps(t3, t1);

        _mm_storeu_ps(&dst[4*i+0], x0);
        _mm_storeu_ps(&dst[4*i+4], x1);
        _mm_storeu_ps(&dst[4*i+8], x2);
        _mm_storeu_ps(&dst[4*i+12], x3);
    }
}

// 4 channels (interleaved)
static void biquad_4x4(float* src, float* dst, float coef[5][4], float state[2][4], int numFrames) {

    // enable flush-to-zero mode to prevent denormals
    unsigned int ftz = _MM_GET_FLUSH_ZERO_MODE();
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

    __m128 w1 = _mm_loadu_ps(state[0]);
    __m128 w2 = _mm_loadu_ps(state[1]);

    __m128 b0 = _mm_loadu_ps(coef[0]);
    __m128 b1 = _mm_loadu_ps(coef[1]);
    __m128 b2 = _mm_loadu_ps(coef[2]);
    __m128 a1 = _mm_loadu_ps(coef[3]);
    __m128 a2 = _mm_loadu_ps(coef[4]);

    for (int i = 0; i < numFrames; i++) {

        // transposed Direct Form II
        __m128 x0 = _mm_loadu_ps(&src[4*i]);
        __m128 y0;

        y0 = _mm_add_ps(w1, _mm_mul_ps(x0, b0));
        w1 = _mm_add_ps(w2, _mm_mul_ps(x0, b1));
        w2 = _mm_mul_ps(x0, b2);
        w1 = _mm_sub_ps(w1, _mm_mul_ps(y0, a1));
        w2 = _mm_sub_ps(w2, _mm_mul_ps(y0, a2));

        _mm_storeu_ps(&dst[4*i], y0);
    }

    // save state
    _mm_storeu_ps(state[0], w1);
    _mm_storeu_ps(state[1], w2);

    _MM_SET_FLUSH_ZERO_MODE(ftz);
}

// crossfade 4 inputs into 2 outputs with accumulation (interleaved)
static void crossfade_4x2(float* src, float* dst, const float* win, int numFrames) {

    assert(numFrames % 4 == 0);

    for (int i = 0; i < numFrames; i += 4) {

        __m128 f0 = _mm_loadu_ps(&win[i]);

        __m128 x0 = _mm_loadu_ps(&src[4*i+0]);
        __m128 x1 = _mm_loadu_ps(&src[4*i+4]);
        __m128 x2 = _mm_loadu_ps(&src[4*i+8]);
        __m128 x3 = _mm_loadu_ps(&src[4*i+12]);

        __m128 y0 = _mm_loadu_ps(&dst[2*i+0]);
        __m128 y1 = _mm_loadu_ps(&dst[2*i+4]);

        // deinterleave (4x4 matrix transpose)
        __m128 t0 = _mm_unpacklo_ps(x0, x1);
        __m128 t2 = _mm_unpacklo_ps(x2, x3);
        __m128 t1 = _mm_unpackhi_ps(x0, x1);
        __m128 t3 = _mm_unpackhi_ps(x2, x3);

        x0 = _mm_movelh_ps(t0, t2);
        x1 = _mm_movehl_ps(t2, t0);
        x2 = _mm_movelh_ps(t1, t3);
        x3 = _mm_movehl_ps(t3, t1);

        // crossfade
        x0 = _mm_sub_ps(x0, x2);
        x1 = _mm_sub_ps(x1, x3);
        x2 = _mm_add_ps(x2, _mm_mul_ps(f0, x0));
        x3 = _mm_add_ps(x3, _mm_mul_ps(f0, x1));

        // interleave
        x0 = _mm_unpacklo_ps(x2, x3);
        x1 = _mm_unpackhi_ps(x2, x3);

        // accumulate
        y0 = _mm_add_ps(y0, x0);
        y1 = _mm_add_ps(y1, x1);

        _mm_storeu_ps(&dst[2*i+0], y0);
        _mm_storeu_ps(&dst[2*i+4], y1);
    }
}

// linear interpolation with gain
static void interpolate(float* dst, const float* src0, const float* src1, float frac, float gain) {

    __m128 f0 = _mm_set1_ps(HRTF_GAIN * gain * (1.0f - frac));
    __m128 f1 = _mm_set1_ps(HRTF_GAIN * gain * frac);

    assert(HRTF_TAPS % 4 == 0);

    for (int k = 0; k < HRTF_TAPS; k += 4) {

        __m128 x0 = _mm_loadu_ps(&src0[k]);
        __m128 x1 = _mm_loadu_ps(&src1[k]);

        x0 = _mm_add_ps(_mm_mul_ps(f0, x0), _mm_mul_ps(f1, x1));

        _mm_storeu_ps(&dst[k], x0);
    }
}

#else   // portable reference code

// 1 channel input, 4 channel output
static void FIR_1x4(float* src, float* dst0, float* dst1, float* dst2, float* dst3, float coef[4][HRTF_TAPS], int numFrames) {

    float* coef0 = coef[0] + HRTF_TAPS - 1;     // process backwards
    float* coef1 = coef[1] + HRTF_TAPS - 1;
    float* coef2 = coef[2] + HRTF_TAPS - 1;
    float* coef3 = coef[3] + HRTF_TAPS - 1;

    assert(numFrames % 4 == 0);

    for (int i = 0; i < numFrames; i += 4) {

        dst0[i+0] = 0.0f;
        dst0[i+1] = 0.0f;
        dst0[i+2] = 0.0f;
        dst0[i+3] = 0.0f;

        dst1[i+0] = 0.0f;
        dst1[i+1] = 0.0f;
        dst1[i+2] = 0.0f;
        dst1[i+3] = 0.0f;

        dst2[i+0] = 0.0f;
        dst2[i+1] = 0.0f;
        dst2[i+2] = 0.0f;
        dst2[i+3] = 0.0f;

        dst3[i+0] = 0.0f;
        dst3[i+1] = 0.0f;
        dst3[i+2] = 0.0f;
        dst3[i+3] = 0.0f;

        float* ps = &src[i - HRTF_TAPS + 1];    // process forwards

        assert(HRTF_TAPS % 4 == 0);

        for (int k = 0; k < HRTF_TAPS; k += 4) {

            // channel 0
            dst0[i+0] += coef0[-k-0] * ps[k+0] + coef0[-k-1] * ps[k+1] + coef0[-k-2] * ps[k+2] + coef0[-k-3] * ps[k+3];
            dst0[i+1] += coef0[-k-0] * ps[k+1] + coef0[-k-1] * ps[k+2] + coef0[-k-2] * ps[k+3] + coef0[-k-3] * ps[k+4];
            dst0[i+2] += coef0[-k-0] * ps[k+2] + coef0[-k-1] * ps[k+3] + coef0[-k-2] * ps[k+4] + coef0[-k-3] * ps[k+5];
            dst0[i+3] += coef0[-k-0] * ps[k+3] + coef0[-k-1] * ps[k+4] + coef0[-k-2] * ps[k+5] + coef0[-k-3] * ps[k+6];

            // channel 1
            dst1[i+0] += coef1[-k-0] * ps[k+0] + coef1[-k-1] * ps[k+1] + coef1[-k-2] * ps[k+2] + coef1[-k-3] * ps[k+3];
            dst1[i+1] += coef1[-k-0] * ps[k+1] + coef1[-k-1] * ps[k+2] + coef1[-k-2] * ps[k+3] + coef1[-k-3] * ps[k+4];
            dst1[i+2] += coef1[-k-0] * ps[k+2] + coef1[-k-1] * ps[k+3] + coef1[-k-2] * ps[k+4] + coef1[-k-3] * ps[k+5];
            dst1[i+3] += coef1[-k-0] * ps[k+3] + coef1[-k-1] * ps[k+4] + coef1[-k-2] * ps[k+5] + coef1[-k-3] * ps[k+6];

            // channel 2
            dst2[i+0] += coef2[-k-0] * ps[k+0] + coef2[-k-1] * ps[k+1] + coef2[-k-2] * ps[k+2] + coef2[-k-3] * ps[k+3];
            dst2[i+1] += coef2[-k-0] * ps[k+1] + coef2[-k-1] * ps[k+2] + coef2[-k-2] * ps[k+3] + coef2[-k-3] * ps[k+4];
            dst2[i+2] += coef2[-k-0] * ps[k+2] + coef2[-k-1] * ps[k+3] + coef2[-k-2] * ps[k+4] + coef2[-k-3] * ps[k+5];
            dst2[i+3] += coef2[-k-0] * ps[k+3] + coef2[-k-1] * ps[k+4] + coef2[-k-2] * ps[k+5] + coef2[-k-3] * ps[k+6];

            // channel 3
            dst3[i+0] += coef3[-k-0] * ps[k+0] + coef3[-k-1] * ps[k+1] + coef3[-k-2] * ps[k+2] + coef3[-k-3] * ps[k+3];
            dst3[i+1] += coef3[-k-0] * ps[k+1] + coef3[-k-1] * ps[k+2] + coef3[-k-2] * ps[k+3] + coef3[-k-3] * ps[k+4];
            dst3[i+2] += coef3[-k-0] * ps[k+2] + coef3[-k-1] * ps[k+3] + coef3[-k-2] * ps[k+4] + coef3[-k-3] * ps[k+5];
            dst3[i+3] += coef3[-k-0] * ps[k+3] + coef3[-k-1] * ps[k+4] + coef3[-k-2] * ps[k+5] + coef3[-k-3] * ps[k+6];
        }
    }
}

// 4 channel planar to interleaved
static void interleave_4x4(float* src0, float* src1, float* src2, float* src3, float* dst, int numFrames) {

    for (int i = 0; i < numFrames; i++) {

        dst[4*i+0] = src0[i];
        dst[4*i+1] = src1[i];
        dst[4*i+2] = src2[i];
        dst[4*i+3] = src3[i];
    }
}

// 4 channels (interleaved)
static void biquad_4x4(float* src, float* dst, float coef[5][4], float state[2][4], int numFrames) {

    // channel 0
    float w10 = state[0][0];
    float w20 = state[1][0];

    float b00 = coef[0][0];
    float b10 = coef[1][0];
    float b20 = coef[2][0];
    float a10 = coef[3][0];
    float a20 = coef[4][0];

    // channel 1
    float w11 = state[0][1];
    float w21 = state[1][1];

    float b01 = coef[0][1];
    float b11 = coef[1][1];
    float b21 = coef[2][1];
    float a11 = coef[3][1];
    float a21 = coef[4][1];

    // channel 2
    float w12 = state[0][2];
    float w22 = state[1][2];

    float b02 = coef[0][2];
    float b12 = coef[1][2];
    float b22 = coef[2][2];
    float a12 = coef[3][2];
    float a22 = coef[4][2];

    // channel 3
    float w13 = state[0][3];
    float w23 = state[1][3];

    float b03 = coef[0][3];
    float b13 = coef[1][3];
    float b23 = coef[2][3];
    float a13 = coef[3][3];
    float a23 = coef[4][3];

    for (int i = 0; i < numFrames; i++) {

        float x00 = src[4*i+0] + 1.0e-20f;    // prevent denormals
        float x01 = src[4*i+1] + 1.0e-20f;
        float x02 = src[4*i+2] + 1.0e-20f;
        float x03 = src[4*i+3] + 1.0e-20f;
        float y00, y01, y02, y03;

        // transposed Direct Form II
        y00 = b00 * x00 + w10;
        w10 = b10 * x00 - a10 * y00 + w20;
        w20 = b20 * x00 - a20 * y00;

        y01 = b01 * x01 + w11;
        w11 = b11 * x01 - a11 * y01 + w21;
        w21 = b21 * x01 - a21 * y01;

        y02 = b02 * x02 + w12;
        w12 = b12 * x02 - a12 * y02 + w22;
        w22 = b22 * x02 - a22 * y02;

        y03 = b03 * x03 + w13;
        w13 = b13 * x03 - a13 * y03 + w23;
        w23 = b23 * x03 - a23 * y03;

        dst[4*i+0] = y00;
        dst[4*i+1] = y01;
        dst[4*i+2] = y02;
        dst[4*i+3] = y03;
    }

    // save state
    state[0][0] = w10;
    state[1][0] = w20;

    state[0][1] = w11;
    state[1][1] = w21;

    state[0][2] = w12;
    state[1][2] = w22;

    state[0][3] = w13;
    state[1][3] = w23;
}

// crossfade 4 inputs into 2 outputs with accumulation (interleaved)
static void crossfade_4x2(float* src, float* dst, const float* win, int numFrames) {

    for (int i = 0; i < numFrames; i++) {

        float frac = win[i];

        dst[2*i+0] += src[4*i+2] + frac * (src[4*i+0] - src[4*i+2]);
        dst[2*i+1] += src[4*i+3] + frac * (src[4*i+1] - src[4*i+3]);
    }
}

// linear interpolation with gain
static void interpolate(float* dst, const float* src0, const float* src1, float frac, float gain) {

    float f0 = HRTF_GAIN * gain * (1.0f - frac);
    float f1 = HRTF_GAIN * gain * frac;

    for (int k = 0; k < HRTF_TAPS; k++) {
        dst[k] = f0 * src0[k] + f1 * src1[k];
    }
}

#endif

// design a 2nd order Thiran allpass
static void ThiranBiquad(float f, float& b0, float& b1, float& b2, float& a1, float& a2) {

    a1 = -2.0f * (f - 2.0f) / (f + 1.0f);
    a2 = ((f - 1.0f) * (f - 2.0f)) / ((f + 1.0f) * (f + 2.0f));
    b0 = a2;
    b1 = a1;
    b2 = 1.0f;
}

// compute new filters for a given azimuth and gain
static void setAzimuthAndGain(float firCoef[4][HRTF_TAPS], float bqCoef[5][4], int delay[4], 
                              int index, float azimuth, float gain, int channel) {

    // convert from radians to table units
    azimuth *= HRTF_AZIMUTHS / TWOPI;

    // wrap to principle value
    while (azimuth < 0.0f) {
        azimuth += HRTF_AZIMUTHS;
    }
    while (azimuth >= HRTF_AZIMUTHS) {
        azimuth -= HRTF_AZIMUTHS;
    }

    // table parameters
    int az0 = (int)azimuth;
    int az1 = (az0 + 1) % HRTF_AZIMUTHS;
    float frac = azimuth - (float)az0;

    assert((az0 >= 0) && (az0 < HRTF_AZIMUTHS));
    assert((az1 >= 0) && (az1 < HRTF_AZIMUTHS));
    assert((frac >= 0.0f) && (frac < 1.0f));

    // interpolate FIR
    interpolate(firCoef[channel+0], ir_table_table[index][az0][0], ir_table_table[index][az1][0], frac, gain);
    interpolate(firCoef[channel+1], ir_table_table[index][az0][1], ir_table_table[index][az1][1], frac, gain);

    // interpolate ITD
    float itd = (1.0f - frac) * itd_table_table[index][az0] + frac * itd_table_table[index][az1];

    // split ITD into integer and fractional delay
    int itdi = (int)fabsf(itd);
    float itdf = fabsf(itd) - (float)itdi;

    assert(itdi <= HRTF_DELAY);
    assert(itdf <= 1.0f);

    //
    // Compute a 2nd-order Thiran allpass for the fractional delay.
    // With nominal delay of 2, the active range of [2.0, 3.0] results
    // in group delay flat to 1.5KHz and fast transient settling time.
    //
    float b0, b1, b2, a1, a2;
    ThiranBiquad(2.0f + itdf, b0, b1, b2, a1, a2);

    // positive ITD means left channel is delayed
    if (itd >= 0.0f) {

        // left (contralateral) = 2 + itdi + itdf
        bqCoef[0][channel+0] = b0;
        bqCoef[1][channel+0] = b1;
        bqCoef[2][channel+0] = b2;
        bqCoef[3][channel+0] = a1;
        bqCoef[4][channel+0] = a2;
        delay[channel+0] = itdi;

        // right (ipsilateral) = 2
        bqCoef[0][channel+1] = 0.0f;
        bqCoef[1][channel+1] = 0.0f;
        bqCoef[2][channel+1] = 1.0f;
        bqCoef[3][channel+1] = 0.0f;
        bqCoef[4][channel+1] = 0.0f;
        delay[channel+1] = 0;

    } else {

        // left (ipsilateral) = 2
        bqCoef[0][channel+0] = 0.0f;
        bqCoef[1][channel+0] = 0.0f;
        bqCoef[2][channel+0] = 1.0f;
        bqCoef[3][channel+0] = 0.0f;
        bqCoef[4][channel+0] = 0.0f;
        delay[channel+0] = 0;

        // right (contralateral) = 2 + itdi + itdf
        bqCoef[0][channel+1] = b0;
        bqCoef[1][channel+1] = b1;
        bqCoef[2][channel+1] = b2;
        bqCoef[3][channel+1] = a1;
        bqCoef[4][channel+1] = a2;
        delay[channel+1] = itdi;
    }
}

void AudioHRTF::render(int16_t* input, float* output, int index, float azimuth, float gain, int numFrames) {

    assert(index >= 0);
    assert(index < HRTF_TABLES);
    assert(numFrames == HRTF_BLOCK);

    float in[HRTF_TAPS + HRTF_BLOCK];               // mono
    float firCoef[4][HRTF_TAPS];                    // 4-channel
    float firBuffer[4][HRTF_DELAY + HRTF_BLOCK];    // 4-channel
    float bqCoef[5][4];                             // 4-channel (interleaved)
    float bqBuffer[4 * HRTF_BLOCK];                 // 4-channel (interleaved)
    int delay[4];                                   // 4-channel (interleaved)

    // to avoid polluting the cache, old filters are recomputed instead of stored
    setAzimuthAndGain(firCoef, bqCoef, delay, index, _azimuthState, _gainState, L0);

    // compute new filters
    setAzimuthAndGain(firCoef, bqCoef, delay, index, azimuth, gain, L1);

    // new parameters become old
    _azimuthState = azimuth;
    _gainState = gain;

    // convert mono input to float
    for (int i = 0; i < HRTF_BLOCK; i++) {
        in[HRTF_TAPS+i] = (float)input[i] * (1/32768.0f);
    }

    // FIR state update
    memcpy(in, _firState, HRTF_TAPS * sizeof(float));
    memcpy(_firState, &in[HRTF_BLOCK], HRTF_TAPS * sizeof(float));

    // process old/new FIR
    FIR_1x4(&in[HRTF_TAPS], 
            &firBuffer[L0][HRTF_DELAY], 
            &firBuffer[R0][HRTF_DELAY], 
            &firBuffer[L1][HRTF_DELAY], 
            &firBuffer[R1][HRTF_DELAY], 
            firCoef, HRTF_BLOCK);

    // delay state update
    memcpy(firBuffer[L0], _delayState[L0], HRTF_DELAY * sizeof(float));
    memcpy(firBuffer[R0], _delayState[R0], HRTF_DELAY * sizeof(float));
    memcpy(firBuffer[L1], _delayState[L1], HRTF_DELAY * sizeof(float));
    memcpy(firBuffer[R1], _delayState[R1], HRTF_DELAY * sizeof(float));

    memcpy(_delayState[L0], &firBuffer[L1][HRTF_BLOCK], HRTF_DELAY * sizeof(float));  // new state becomes old
    memcpy(_delayState[R0], &firBuffer[R1][HRTF_BLOCK], HRTF_DELAY * sizeof(float));  // new state becomes old
    memcpy(_delayState[L1], &firBuffer[L1][HRTF_BLOCK], HRTF_DELAY * sizeof(float));
    memcpy(_delayState[R1], &firBuffer[R1][HRTF_BLOCK], HRTF_DELAY * sizeof(float));

    // interleave with old/new integer delay
    interleave_4x4(&firBuffer[L0][HRTF_DELAY] - delay[L0],
                   &firBuffer[R0][HRTF_DELAY] - delay[R0],
                   &firBuffer[L1][HRTF_DELAY] - delay[L1],
                   &firBuffer[R1][HRTF_DELAY] - delay[R1],
                   bqBuffer, HRTF_BLOCK);

    // process old/new fractional delay
    biquad_4x4(bqBuffer, bqBuffer, bqCoef, _bqState, HRTF_BLOCK);

    // new state becomes old
    _bqState[0][L0] = _bqState[0][L1];
    _bqState[1][L0] = _bqState[1][L1];
    _bqState[0][R0] = _bqState[0][R1];
    _bqState[1][R0] = _bqState[1][R1];

    // crossfade old/new output and accumulate
    crossfade_4x2(bqBuffer, output, crossfadeTable, HRTF_BLOCK);

    _silentState = false;
}

void AudioHRTF::renderSilent(int16_t* input, float* output, int index, float azimuth, float gain, int numFrames) {

    // process the first silent block, to flush internal state
    if (!_silentState) {
        render(input, output, index, azimuth, gain, numFrames);
    } 

    // new parameters become old
    _azimuthState = azimuth;
    _gainState = gain;

    _silentState = true;
}
