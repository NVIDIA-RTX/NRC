/*
 * Copyright (c) 2019-2025, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#pragma once

#include "NrcStructures.h"

#ifdef EXPORTING_NRC
#define NRC_DECLSPEC __declspec(dllexport)
#else
#define NRC_DECLSPEC __declspec(dllimport)
#endif

#define NRC_VERSION_MAJOR 0
#define NRC_VERSION_MINOR 13
#define NRC_VERSION_DATE "22 January 2025"

namespace nrc
{
/**
 *  Log level of the message provided to the custom logger
 */
enum class LogLevel
{
    Debug = 1, //< Lowest level, only interesting for debugging
    Info = 2, //< Useful information about successful initialization, etc.
    Warning = 3, //< Looming problem
    Error = 4 //< General failure
};

/**
 *  Status of the SDK, returned as an error (or success) code from API calls
 */
enum class Status
{
    OK,
    SDKVersionMismatch, //< SDK version in the header file doesn't match library version - need to update header file?
    AlreadyInitialized, //< You're trying to initialize NRC SDK multiple times, please deinitialize old instance first.
    SDKNotInitialized, //< SDK was not yet initialized.
    InternalError, //< Unexpected condition occured during processing, see error log for more information.
    MemoryNotProvided, //< Memory allocation within SDK is disabled, but necessary memory was not provided.
    OutOfMemory, //< There is insufficient memory to create the GPU resource.
    AllocationFailed, //< Memory allocation failed.
    ErrorParsingJSON, //< Provided JSON string is malformed
    WrongParameter, //< Parameter provided to the SDK API call was invalid
    UnsupportedDriver, //< Installed driver version is not supported
    UnsupportedHardware, //< GPU Device is not supported
};

/**
 *  Type of the memory "event" reported by the Memory Events Logger
 */
enum class MemoryEventType
{
    Allocation,
    Deallocation,
    MemoryStats
};

/**
 *  Pointer to function handling logging of SDK messages
 */
typedef void (*CustomLoggerPtr)(const char* message, LogLevel logLevel);

/**
 *  Pointer to function handling memory allocator messages
 */
typedef void (*MemoryEventsLoggerPtr)(MemoryEventType eventType, size_t size, const char* bufferName);

/**
 *  Pointer to function handling CPU side allocations
 */
typedef void* (*CustomAllocatorPtr)(const size_t bytes);

/**
 *  Pointer to function handling CPU side deallocations
 */
typedef void (*CustomDeallocatorPtr)(void* pointer, const size_t bytes);

/**
 *  Global configuration of NRC Library provided by application once at initialization
 */
struct GlobalSettings
{
    // Please leave these as their default values. They are used to validate
    // the version of the DLL matches the version of the headers.
    int majorVersion = NRC_VERSION_MAJOR;
    int minorVersion = NRC_VERSION_MINOR;

    // Optional callbacks
    CustomLoggerPtr loggerFn = nullptr;
    MemoryEventsLoggerPtr memoryLoggerFn = nullptr;
    CustomAllocatorPtr allocatorFn = nullptr;
    CustomDeallocatorPtr deallocatorFn = nullptr;

    // If true, then the SDK will allocate all its buffers internally.
    // You can then access them with Context::GetBuffers
    // If false, then the application is responsible for allocating the buffers.
    // Use Context::GetBuffersAllocationInfo to get the info about the buffers
    // and then pass them to NRC using Context::Configure
    bool enableGPUMemoryAllocation = true;

    // If true, then additional debug buffers will be allocated, enabling
    // the use more some of the debug resolve modes.
    bool enableDebugBuffers = false;

    // How many frames in flight might the engine have.
    uint32_t maxNumFramesInFlight = 4; // Conservative default

    // Optional setting for specifying a custom directory path where the CUDA
    // run-time dll dependencies are located
    const char* depsDirectoryPath = nullptr;
};

/**
 *  Configuration of an NRC Context provided by application.
 *  These are expected to change infrequently.
 */
struct ContextSettings
{
    // Disable when learning each component individually.
    // Requires albedo demodulation.
    bool learnIrradiance = false;

    // Whether or not the radiance cache should include direct lighting.
    // If you're using NRC for path tracing early termination, then
    // you probably want this enabled.
    bool includeDirectLighting = false;

    // Use this to force a reset of the cache.
    // Lots of the time, calling Context::Configure will result in
    // a cache reset.  But sometimes it won't if the things that
    // have changed don't require it.
    // If you really want the cache to be reset (e.g. if you're
    // loading a new scene), then set this to true.
    bool requestReset = false;

    // The bounds of the scene and the feature resolution are used
    // to determine certain parameters for the cache.
    // The whole scene (i.e. anything that a ray can hit) *must*
    // be contained within the scene bounds.
    nrc_float3 sceneBoundsMin = {};
    nrc_float3 sceneBoundsMax = {};

    // Determines the resolution of the cache.
    float smallestResolvableFeatureSize = 0.01f;

    // Path tracing resolution
    nrc_uint2 frameDimensions = {};

    // Training resolution.
    // Use computeIdealTrainingDimensions to compute the best resolution.
    nrc_uint2 trainingDimensions = {};

    // How many samples (paths) per pixel you are doing in the main
    // path tracing pass.
    // Note that some buffer sizes will increase in proportion to
    // the number of samples per pixel.
    uint32_t samplesPerPixel = 1;

    // The maximum number of path tracer bounces that will be done
    // during training.
    uint32_t maxPathVertices = 8;

    NRC_DECLSPEC bool operator==(const ContextSettings& rhs) const;
    bool operator!=(const ContextSettings& rhs) const
    {
        return !(*this == rhs);
    }
};

/**
 *  Helper == operator for ContextSettings
 *  Typical usage:
 *    if( (newContextSettings != oldContextSettings) || newContextSettings.requestReset )
 *    {
 *        nrcContext.Configure( newContextSettings );
 *    }
 **/
inline bool ContextSettings::operator==(const ContextSettings& rhs) const
{
    // clang-format off
    return
        (learnIrradiance == rhs.learnIrradiance) &&
        (requestReset == rhs.requestReset) &&
        (includeDirectLighting == rhs.includeDirectLighting) &&
        (sceneBoundsMin.x == rhs.sceneBoundsMin.x) &&
        (sceneBoundsMin.y == rhs.sceneBoundsMin.y) &&
        (sceneBoundsMin.z == rhs.sceneBoundsMin.z) &&
        (sceneBoundsMax.x == rhs.sceneBoundsMax.x) &&
        (sceneBoundsMax.y == rhs.sceneBoundsMax.y) &&
        (sceneBoundsMax.z == rhs.sceneBoundsMax.z) &&
        (smallestResolvableFeatureSize == rhs.smallestResolvableFeatureSize) &&
        (frameDimensions.x == rhs.frameDimensions.x) &&
        (frameDimensions.y == rhs.frameDimensions.y) &&
        (trainingDimensions.x == rhs.trainingDimensions.x) &&
        (trainingDimensions.y == rhs.trainingDimensions.y) &&
        (samplesPerPixel == rhs.samplesPerPixel) &&
        (maxPathVertices == rhs.maxPathVertices);
    // clang-format on
}

/**
 *  Helper which computes a suitable training resolution for the network based on
 *  the framebuffer dimensions and numer of training iterations.
 *  You can pass 0 to numTrainingIterations and avgTrainingVerticesPerPath to use default values.
 *  It is recommended to use this function to set the values of `ContextSettings::trainingDimensions`
 *  by passing in the default `numTrainingIterations`.  Then if you wish to
 * lower
 *  the value of `numTrainingIterations`, you can call this again to calculate a resolution
 *  for the update/training dispatch and pass that resolution in
 * `FrameSettings::usedTrainingDimensions`
 **/
NRC_DECLSPEC nrc_uint2 ComputeIdealTrainingDimensions(nrc_uint2 const& frameDimensions, uint32_t numTrainingIterations, float avgTrainingVerticesPerPath = 0.f);

/**
 *  Helper for ImGui based debugging UIs.
 *  This returns a string of resolve modes, each null terminated, in the
 *  format expected by ImGui::Combo
 */
NRC_DECLSPEC char const* GetImGuiResolveModeComboString();

/**
 *  Per-frame settings of the NRC Context provided by application
 */
struct FrameSettings
{
    // NRC works better when the radiance values it sees internally are in a 'friendly'
    // range for it.
    // Applications often have quite different scales for their radiance units, so
    // we need to be able to scale these units in order to get that nice NRC-friendly range.
    // This value should broadly correspond to the average radiance that you might see
    // in one of your bright scenes (e.g. outdoors in daylight).
    // This is in FrameSettings, but it should not be adjusted very much on a frame
    // to frame basis.  Rather it's here to allow developers to experiment easily
    // with it to find a good value that works.
    float maxExpectedAverageRadianceValue = 1.0f;

    // This will prevent NRC from terminating on mirrors - it continue to the next vertex
    bool skipDeltaVertices = false;

    // Knob for the termination heuristic to determine when it terminates the path.
    // The default value should give good quality.  You can decrease the value to
    // bias the algorithm to terminating earlier, trading off quality for performance.
    float terminationHeuristicThreshold = 0.1f;
    // This should normally be set to the same value, and might be deprecated soon.
    float trainingTerminationHeuristicThreshold = 0.1f;

    // Controls the behaviour of the optional resolve pass, allowing it to be used
    // to provide various debug visualisations.
    NrcResolveMode resolveMode = NrcResolveMode::AddQueryResultToOutput;

    // Debugging aid. You can disable the training to 'freeze' the state of the cache.
    bool trainTheCache = true;

    // The training dimensions that were actually written to in the update pass.
    // Cannot be larger than ContextSettings::trainingDimensions.
    // The default value of (0,0) means that the whole ContextSettings::trainingDimensions was used.
    nrc_uint2 usedTrainingDimensions = { 0, 0 };


    // Development debugging aids.

    // Controls what proportion of different path segments are used for training.
    // Proportion of primary segments used to train the cache.  Should be very low.
    float proportionPrimarySegmentsToTrainOn = 0.05f;

    // Proportion of tertiary and beyond segments to train on
    float proportionTertiaryPlusSegmentsToTrainOn = 1.0f;

    // Proportion of unbiased paths that we should do self-training on
    float proportionUnbiasedToSelfTrain = 1.0f;

    // Proportion of training paths that should be 'unbiased'
    float proportionUnbiased = 0.0625f;

    // Allows the radiance from self-training to be attenuated or completely disabled
    // to help debugging.
    // If there's an error in the path tracer that breaks energy conservation for example,
    // the self training feedback can sometimes lead to the cache getting brighter
    // and brighter. This control can help debug such issues.
    float selfTrainingAttenuation = 1.0f;

    // This controls how many training iterations are performed each frame,
    // which in turn determines the ideal number of training records that
    // the training/update path tracing pass is expected to generate.
    // Each training batch contains 16K training records derived from path segments
    // in the the NRC update path tracing pass.
    // E.g. if `numTrainingIterations` is 4, then we ideally expect the
    // update path tracing pass to generate 64K training records.
    // This value is taken into account in `ComputeIdealTrainingDimensions`.
    // It is recommended that you set `ContextSettings::trainingDimensions` using
    // the default value for `numTrainingIterations`, and size your buffers accordingly.
    // Then if you are running at a high frame rate, and you wish to reduce how
    // much training is done per frame, you can reduce this and call
    // `ComputeIdealTrainingDimensions` to set a lower resolution in
    // `FrameSettings::usedTrainingDimensions` (and your training/update dispatch).
    uint32_t numTrainingIterations = 4;

    // Tuning parameter which defines the step size taken in the gradinet descent
    // direction (determined from the gradient of the loss function).
    // The learning rate comes with a trade-off between the speed of convergence (low value)
    // and overshooting (oscillating) when using a high value.
    // For the Adam optimizer the recommended range is [0.0001 and 0.01].
    float learningRate = 1e-2f;
};

/**
 *  Data needed for creating the buffer and its views
 */
struct AllocationInfo
{
    size_t elementCount = 0;
    size_t elementSize = 0;
    bool isOnSharedHeap;
    bool allowUAV;
    bool useReadbackHeap;
    const char* debugName;
};

/**
 *  Enum of buffers required by NRC.
 *  The buffers can be allocated by the SDK, or they can be allocated by the app
 *  according to GlobalSettings::enableGPUMemoryAllocation
 */
enum class BufferIdx
{
    // Atomic counters storing total number of query and training records.
    // Total number of entries = 2
    Counter,

    // Buffer storing path info for the query resolve, such as query index and prefix throughput.
    // One entry per path.
    // Total number of entries = width * height * SPP
    QueryPathInfo,

    // Buffer storing path info for all the training paths such as length, flag to indicate path exitted, prefix throughput.
    // One entry per training path(either training or non - training).
    // Total number of entries = training-width * training-height
    TrainingPathInfo,

    // Buffer storing data for path vertices, one vertex per training path segment.
    // This data includes surface properties of the previous path vertex as well as throughput to next vertex.
    // Total number of entries = training-width * training-height * (maxBounces per path)
    TrainingPathVertices,

    // Training radiance, one entry per training path vertex.
    // Total number of entries = training-width * training-height
    TrainingRadiance,

    // Buffer containing the encoded values such as position, view direction, normal, roughness.
    // 1:1 correspondance with the TrainingRadiance buffer (same number of entries)
    TrainingRadianceParams,

    // The predicted radiance returned from NRC.  One entry per call to NRC's query creation.
    // There is potentially one query per path, and one query per training path,
    // Total number of entries = (training-width * training-height) +
    //                           (width * height * SPP)
    QueryRadiance,

    // Buffer containing the encoded values such as position, view direction, normal, roughness.
    // Essentially, this buffer contains the queries and QueryRadiance contains the answers.
    // 1:1 correspondance with the QueryRadiance buffer (same number of entries)
    QueryRadianceParams,

    // Some extra debug info to support the debug resolve modes
    // It's not very large.
    // Total number of entries = training-width * training-height.
    DebugTrainingPathInfo,

    Count
};

/**
 *  Information required for allocation of all NRC SDK buffers
 */
struct BuffersAllocationInfo
{
    // [] Operators to allow direct access to the array with BufferIdx
    const AllocationInfo& operator[](BufferIdx idx) const
    {
        return allocationInfo[(int)idx];
    }
    AllocationInfo& operator[](BufferIdx idx)
    {
        return allocationInfo[(int)idx];
    }

    AllocationInfo allocationInfo[(int)BufferIdx::Count];
};

[[deprecated("Use ComputeIdealTrainingDimensions instead")]] inline nrc_uint2 computeIdealTrainingDimensions(nrc_uint2 const& frameDimensions, float avgTrainingVerticesPerPath = 0.f)
{
    return ComputeIdealTrainingDimensions(frameDimensions, 0, avgTrainingVerticesPerPath);
}

} // namespace nrc
