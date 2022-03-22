/*
* Copyright (c) 2019-2022, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef RTXGI_DDGI_VOLUME_DESC_GPU_H
#define RTXGI_DDGI_VOLUME_DESC_GPU_H

#ifndef HLSL
#include <rtxgi/Types.h>
using namespace rtxgi;
#endif

/**
 * Describes the properties of a DDGIVolume, with values packed to compact formats.
 * This version of the struct uses 128B to store some values at full precision.
 */
struct DDGIVolumeDescGPUPacked
{
    float3   origin;
    float    probeHysteresis;
    //------------------------------------------------- 16B
    float4   rotation;
    //------------------------------------------------- 32B
    float4   probeRayRotation;
    //------------------------------------------------- 64B
    float    probeMaxRayDistance;
    float    probeNormalBias;
    float    probeViewBias;
    float    probeDistanceExponent;
    //------------------------------------------------- 80B
    float3   probeSpacing;
    uint     packed0;       // probeCounts.x (8), probeCounts.y (8), probeCounts.z (8), unused (8)
    //------------------------------------------------- 96B
    float    probeIrradianceEncodingGamma;
    float    probeIrradianceThreshold;
    float    probeBrightnessThreshold;
    float    probeBackfaceThreshold;
    //------------------------------------------------- 112B
    float    probeMinFrontfaceDistance;
    uint     packed1;       // probeNumRays (16), probeNumIrradianceTexels (8), probeNumDistanceTexels (8)
    uint     packed2;       // probeScrollOffsets.x (15) sign bit (1), probeScrollOffsets.y (15) sign bit (1)
    uint     packed3;       // probeScrollOffsets.z (15) sign bit (1)
                            // movementType (1), rayDataFormat (1), irradianceFormat (1), probeRelocationEnabled (1), probeClassificationEnabled (1)
                            // probeScrollClear Y-Z plane (1), probeScrollClear X-Z plane (1), probeScrollClear X-Y plane (1)
                            // unused (8)
    //------------------------------------------------- 128B
};

/**
 * Describes the properties of a DDGIVolume.
 */
struct DDGIVolumeDescGPU
{
    float3   origin;                             // world-space location of the volume center

    float4   rotation;                           // rotation quaternion for the volume
    float4   probeRayRotation;                   // rotation quaternion for probe rays

    uint     movementType;                       // type of movement the volume allows. 0: default, 1: infinite scrolling

    float3   probeSpacing;                       // world-space distance between probes
    int3     probeCounts;                        // number of probes on each axis of the volume

    int      probeNumRays;                       // number of rays traced per probe
    int      probeNumIrradianceTexels;           // number of texels in one dimension of a probe's irradiance texture (does not include 1-texel border)
    int      probeNumDistanceTexels;             // number of texels in one dimension of a probe's distance texture (does not include 1-texel border)

    float    probeHysteresis;                    // weight of the previous irradiance and distance data store in probes
    float    probeMaxRayDistance;                // maximum world-space distance a probe ray can travel
    float    probeNormalBias;                    // offset along the surface normal, applied during lighting to avoid numerical instabilities when determining visibility
    float    probeViewBias;                      // offset along the camera view ray, applied during lighting to avoid numerical instabilities when determining visibility
    float    probeDistanceExponent;              // exponent used during visibility testing. High values react rapidly to depth discontinuities, but may cause banding
    float    probeIrradianceThreshold;           // threshold to identify when large lighting changes occur
    float    probeBrightnessThreshold;           // threshold that specifies the maximum allowed difference in brightness between the previous and current irradiance values
    float    probeIrradianceEncodingGamma;       // exponent that perceptually encodes irradiance for faster light-to-dark convergence

    // Probe Relocation, Probe Classification
    float    probeBackfaceThreshold;             // ratio of rays traced for a probe that may hit back facing triangles before the probe is considered inside geometry
    float    probeMinFrontfaceDistance;          // minimum world-space distance to a front facing triangle allowed before a probe is relocated

    // Infinite Scrolling Volumes
    int3     probeScrollOffsets;                 // grid-space offsets used for scrolling movement

    // Feature Options
    uint     probeRayDataFormat;                 // format type of the ray data texture
    uint     probeIrradianceFormat;              // format type of the irradiance texture
    bool     probeRelocationEnabled;             // whether probe relocation is enabled for this volume
    bool     probeClassificationEnabled;         // whether probe classification is enabled for this volume
    bool     probeScrollClear[3];                // whether probes of a plane need to be cleared due to scrolling movement

#ifndef HLSL
    rtxgi::DDGIVolumeDescGPUPacked GetPackedData()
    {
        rtxgi::DDGIVolumeDescGPUPacked data;
        data.origin = origin;
        data.probeHysteresis = probeHysteresis;
        data.rotation = rotation;
        data.probeRayRotation = probeRayRotation;
        data.probeMaxRayDistance = probeMaxRayDistance;
        data.probeNormalBias = probeNormalBias;
        data.probeViewBias = probeViewBias;
        data.probeDistanceExponent = probeDistanceExponent;
        data.probeSpacing = probeSpacing;

        data.packed0 = (uint32_t)probeCounts.x;
        data.packed0 |= (uint32_t)probeCounts.y << 8;
        data.packed0 |= (uint32_t)probeCounts.z << 16;
      //data.packed0 |= unused

        data.probeIrradianceEncodingGamma = probeIrradianceEncodingGamma;
        data.probeIrradianceThreshold = probeIrradianceThreshold;
        data.probeBrightnessThreshold = probeBrightnessThreshold;
        data.probeBackfaceThreshold = probeBackfaceThreshold;
        data.probeMinFrontfaceDistance = probeMinFrontfaceDistance;

        data.packed1  = (uint32_t)probeNumRays;
        data.packed1 |= (uint32_t)probeNumIrradianceTexels << 16;
        data.packed1 |= (uint32_t)probeNumDistanceTexels << 24;

        // Probe Scroll Offsets
        data.packed2  = (uint32_t)abs(probeScrollOffsets.x);
        data.packed2 |= ((uint32_t)(probeScrollOffsets.x < 0) << 15);
        data.packed2 |= (uint32_t)abs(probeScrollOffsets.y) << 16;
        data.packed2 |= ((uint32_t)(probeScrollOffsets.y < 0) << 31);
        data.packed3  = (uint32_t)abs(probeScrollOffsets.z);
        data.packed3 |= ((uint32_t)(probeScrollOffsets.z < 0) << 15);

        // Feature Bits
        data.packed3 |= movementType << 16;
        data.packed3 |= probeRayDataFormat << 17;
        data.packed3 |= probeIrradianceFormat << 18;
        data.packed3 |= (uint32_t)probeRelocationEnabled << 19;
        data.packed3 |= (uint32_t)probeClassificationEnabled << 20;
        data.packed3 |= (uint32_t)probeScrollClear[0] << 21;
        data.packed3 |= (uint32_t)probeScrollClear[1] << 22;
        data.packed3 |= (uint32_t)probeScrollClear[2] << 23;

        return data;
    }
#endif
};

#ifdef HLSL
DDGIVolumeDescGPU UnpackDDGIVolumeDescGPU(DDGIVolumeDescGPUPacked input)
{
    DDGIVolumeDescGPU output;
    output.origin = input.origin;
    output.probeHysteresis = input.probeHysteresis;
    output.rotation = input.rotation;
    output.probeRayRotation = input.probeRayRotation;
    output.probeMaxRayDistance = input.probeMaxRayDistance;
    output.probeNormalBias = input.probeNormalBias;
    output.probeViewBias = input.probeViewBias;
    output.probeDistanceExponent = input.probeDistanceExponent;
    output.probeSpacing = input.probeSpacing;

    output.probeCounts.x = input.packed0 & 0x000000FF;
    output.probeCounts.y = (input.packed0 >> 8) & 0x000000FF;
    output.probeCounts.z = (input.packed0 >> 16) & 0x000000FF;

    output.probeIrradianceEncodingGamma = input.probeIrradianceEncodingGamma;
    output.probeIrradianceThreshold = input.probeIrradianceThreshold;
    output.probeBrightnessThreshold = input.probeBrightnessThreshold;
    output.probeBackfaceThreshold = input.probeBackfaceThreshold;
    output.probeMinFrontfaceDistance = input.probeMinFrontfaceDistance;

    output.probeNumRays = input.packed1 & 0x0000FFFF;
    output.probeNumIrradianceTexels = (input.packed1 >> 16) & 0x000000FF;
    output.probeNumDistanceTexels = (input.packed1 >> 24) & 0x000000FF;

    // Probe Scroll Offsets
    output.probeScrollOffsets.x = input.packed2 & 0x00007FFF;
    if((input.packed2 >> 15) & 0x00000001) output.probeScrollOffsets.x *= -1;
    output.probeScrollOffsets.y = (input.packed2 >> 16) & 0x00007FFF;
    if((input.packed2 >> 31) & 0x00000001) output.probeScrollOffsets.y *= -1;
    output.probeScrollOffsets.z = (input.packed3) & 0x00007FFF;
    if ((input.packed3 >> 15) & 0x00000001) output.probeScrollOffsets.z *= -1;

    // Feature Bits
    output.movementType = (input.packed3 >> 16) & 0x00000001;
    output.probeRayDataFormat = (uint)((input.packed3 >> 17) & 0x00000001);
    output.probeIrradianceFormat = (uint)((input.packed3 >> 18) & 0x00000001);
    output.probeRelocationEnabled = (bool)((input.packed3 >> 19) & 0x00000001);
    output.probeClassificationEnabled = (bool)((input.packed3 >> 20) & 0x00000001);
    output.probeScrollClear[0] = (bool)((input.packed3 >> 21) & 0x00000001);
    output.probeScrollClear[1] = (bool)((input.packed3 >> 22) & 0x00000001);
    output.probeScrollClear[2] = (bool)((input.packed3 >> 23) & 0x00000001);

    return output;
}
#endif // HLSL
#endif // RTXGI_DDGI_VOLUME_DESC_GPU_H
