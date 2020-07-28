/*
 * Copyright (C) 2018-2019 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GPUD_INCLUDE_GPUD_GPUD_VK_H_
#define GPUD_INCLUDE_GPUD_GPUD_VK_H_

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#define GPUD_VK_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#define GPUD_VK_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3ff)
#define GPUD_VK_VERSION_PATCH(version) ((uint32_t)(version) & 0xfff)

#define GPUD_VK_VENDOR_ID_ARM         (0x13B5)
#define GPUD_VK_VENDOR_ID_QUALCOMM    (0x5143)
#define GPUD_VK_VENDOR_ID_IMAGINATION (0x1010)

#define GPUD_VK_MAX_PHYSICAL_DEVICE_NAME_SIZE 256
#define GPUD_VK_UUID_SIZE                     16

#define GPUD_VK_DEFINE_HANDLE(object) typedef void *object;
#define GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
GPUD_VK_DEFINE_HANDLE(GpudVkInstance)
GPUD_VK_DEFINE_HANDLE(GpudVkPhysicalDevice)
GPUD_VK_DEFINE_HANDLE(GpudVkDevice)
GPUD_VK_DEFINE_HANDLE(GpudVkQueue)
GPUD_VK_DEFINE_HANDLE(GpudVkCommandBuffer)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkSemaphore)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkFence)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkDeviceMemory)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkBuffer)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkImage)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkEvent)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkQueryPool)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkBufferView)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkImageView)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkShaderModule)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkPipelineCache)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkPipelineLayout)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkRenderPass)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkPipeline)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkDescriptorSetLayout)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkSampler)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkDescriptorPool)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkDescriptorSet)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkFramebuffer)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkCommandPool)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkSamplerYcbcrConversion)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkDescriptorUpdateTemplate)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkSurfaceKHR)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkSwapchainKHR)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkDisplayKHR)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkDisplayModeKHR)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkDebugReportCallbackEXT)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkObjectTableNVX)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkIndirectCommandsLayoutNVX)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkDebugUtilsMessengerEXT)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkValidationCacheEXT)
GPUD_VK_DEFINE_NON_DISPATCHABLE_HANDLE(GpudVkAccelerationStructureNV)

typedef enum GpudVkResult {
    GPUD_VK_SUCCESS = 0,
    GPUD_VK_NOT_READY = 1,
    GPUD_VK_TIMEOUT = 2,
    GPUD_VK_EVENT_SET = 3,
    GPUD_VK_EVENT_RESET = 4,
    GPUD_VK_INCOMPLETE = 5,
    GPUD_VK_ERROR_OUT_OF_HOST_MEMORY = -1,
    GPUD_VK_ERROR_OUT_OF_DEVICE_MEMORY = -2,
    GPUD_VK_ERROR_INITIALIZATION_FAILED = -3,
    GPUD_VK_ERROR_DEVICE_LOST = -4,
    GPUD_VK_ERROR_MEMORY_MAP_FAILED = -5,
    GPUD_VK_ERROR_LAYER_NOT_PRESENT = -6,
    GPUD_VK_ERROR_EXTENSION_NOT_PRESENT = -7,
    GPUD_VK_ERROR_FEATURE_NOT_PRESENT = -8,
    GPUD_VK_ERROR_INCOMPATIBLE_DRIVER = -9,
    GPUD_VK_ERROR_TOO_MANY_OBJECTS = -10,
    GPUD_VK_ERROR_FORMAT_NOT_SUPPORTED = -11,
    GPUD_VK_ERROR_FRAGMENTED_POOL = -12,
    GPUD_VK_ERROR_OUT_OF_POOL_MEMORY = -1000069000,
    GPUD_VK_ERROR_INVALID_EXTERNAL_HANDLE = -1000072003,
    GPUD_VK_ERROR_SURFACE_LOST_KHR = -1000000000,
    GPUD_VK_ERROR_NATIVE_WINDOW_IN_USE_KHR = -1000000001,
    GPUD_VK_SUBOPTIMAL_KHR = 1000001003,
    GPUD_VK_ERROR_OUT_OF_DATE_KHR = -1000001004,
    GPUD_VK_ERROR_INCOMPATIBLE_DISPLAY_KHR = -1000003001,
    GPUD_VK_ERROR_VALIDATION_FAILED_EXT = -1000011001,
    GPUD_VK_ERROR_INVALID_SHADER_NV = -1000012000,
    GPUD_VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT = -1000158000,
    GPUD_VK_ERROR_FRAGMENTATION_EXT = -1000161000,
    GPUD_VK_ERROR_NOT_PERMITTED_EXT = -1000174001,
    GPUD_VK_ERROR_INVALID_DEVICE_ADDRESS_EXT = -1000244000,
    GPUD_VK_ERROR_OUT_OF_POOL_MEMORY_KHR = GPUD_VK_ERROR_OUT_OF_POOL_MEMORY,
    GPUD_VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR = GPUD_VK_ERROR_INVALID_EXTERNAL_HANDLE,
    GPUD_VK_RESULT_BEGIN_RANGE = GPUD_VK_ERROR_FRAGMENTED_POOL,
    GPUD_VK_RESULT_END_RANGE = GPUD_VK_INCOMPLETE,
    GPUD_VK_RESULT_RANGE_SIZE = (GPUD_VK_INCOMPLETE - GPUD_VK_ERROR_FRAGMENTED_POOL + 1),
    GPUD_VK_RESULT_MAX_ENUM = 0x7FFFFFFF
} GpudVkResult;

typedef struct GpudVkExtent3D {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} GpudVkExtent3D;

typedef struct GpudVkPhysicalDeviceLimits {
    uint32_t maxImageDimension1D;
    uint32_t maxImageDimension2D;
    uint32_t maxImageDimension3D;
    uint32_t maxImageDimensionCube;
    uint32_t maxImageArrayLayers;
    uint32_t maxTexelBufferElements;
    uint32_t maxUniformBufferRange;
    uint32_t maxStorageBufferRange;
    uint32_t maxPushConstantsSize;
    uint32_t maxMemoryAllocationCount;
    uint32_t maxSamplerAllocationCount;
    uint64_t bufferImageGranularity;
    uint64_t sparseAddressSpaceSize;
    uint32_t maxBoundDescriptorSets;
    uint32_t maxPerStageDescriptorSamplers;
    uint32_t maxPerStageDescriptorUniformBuffers;
    uint32_t maxPerStageDescriptorStorageBuffers;
    uint32_t maxPerStageDescriptorSampledImages;
    uint32_t maxPerStageDescriptorStorageImages;
    uint32_t maxPerStageDescriptorInputAttachments;
    uint32_t maxPerStageResources;
    uint32_t maxDescriptorSetSamplers;
    uint32_t maxDescriptorSetUniformBuffers;
    uint32_t maxDescriptorSetUniformBuffersDynamic;
    uint32_t maxDescriptorSetStorageBuffers;
    uint32_t maxDescriptorSetStorageBuffersDynamic;
    uint32_t maxDescriptorSetSampledImages;
    uint32_t maxDescriptorSetStorageImages;
    uint32_t maxDescriptorSetInputAttachments;
    uint32_t maxVertexInputAttributes;
    uint32_t maxVertexInputBindings;
    uint32_t maxVertexInputAttributeOffset;
    uint32_t maxVertexInputBindingStride;
    uint32_t maxVertexOutputComponents;
    uint32_t maxTessellationGenerationLevel;
    uint32_t maxTessellationPatchSize;
    uint32_t maxTessellationControlPerVertexInputComponents;
    uint32_t maxTessellationControlPerVertexOutputComponents;
    uint32_t maxTessellationControlPerPatchOutputComponents;
    uint32_t maxTessellationControlTotalOutputComponents;
    uint32_t maxTessellationEvaluationInputComponents;
    uint32_t maxTessellationEvaluationOutputComponents;
    uint32_t maxGeometryShaderInvocations;
    uint32_t maxGeometryInputComponents;
    uint32_t maxGeometryOutputComponents;
    uint32_t maxGeometryOutputVertices;
    uint32_t maxGeometryTotalOutputComponents;
    uint32_t maxFragmentInputComponents;
    uint32_t maxFragmentOutputAttachments;
    uint32_t maxFragmentDualSrcAttachments;
    uint32_t maxFragmentCombinedOutputResources;
    uint32_t maxComputeSharedMemorySize;
    uint32_t maxComputeWorkGroupCount[3];
    uint32_t maxComputeWorkGroupInvocations;
    uint32_t maxComputeWorkGroupSize[3];
    uint32_t subPixelPrecisionBits;
    uint32_t subTexelPrecisionBits;
    uint32_t mipmapPrecisionBits;
    uint32_t maxDrawIndexedIndexValue;
    uint32_t maxDrawIndirectCount;
    float    maxSamplerLodBias;
    float    maxSamplerAnisotropy;
    uint32_t maxViewports;
    uint32_t maxViewportDimensions[2];
    float    viewportBoundsRange[2];
    uint32_t viewportSubPixelBits;
    size_t   minMemoryMapAlignment;
    uint64_t minTexelBufferOffsetAlignment;
    uint64_t minUniformBufferOffsetAlignment;
    uint64_t minStorageBufferOffsetAlignment;
    int32_t  minTexelOffset;
    uint32_t maxTexelOffset;
    int32_t  minTexelGatherOffset;
    uint32_t maxTexelGatherOffset;
    float    minInterpolationOffset;
    float    maxInterpolationOffset;
    uint32_t subPixelInterpolationOffsetBits;
    uint32_t maxFramebufferWidth;
    uint32_t maxFramebufferHeight;
    uint32_t maxFramebufferLayers;
    uint32_t framebufferColorSampleCounts;
    uint32_t framebufferDepthSampleCounts;
    uint32_t framebufferStencilSampleCounts;
    uint32_t framebufferNoAttachmentsSampleCounts;
    uint32_t maxColorAttachments;
    uint32_t sampledImageColorSampleCounts;
    uint32_t sampledImageIntegerSampleCounts;
    uint32_t sampledImageDepthSampleCounts;
    uint32_t sampledImageStencilSampleCounts;
    uint32_t storageImageSampleCounts;
    uint32_t maxSampleMaskWords;
    uint32_t timestampComputeAndGraphics;
    float    timestampPeriod;
    uint32_t maxClipDistances;
    uint32_t maxCullDistances;
    uint32_t maxCombinedClipAndCullDistances;
    uint32_t discreteQueuePriorities;
    float    pointSizeRange[2];
    float    lineWidthRange[2];
    float    pointSizeGranularity;
    float    lineWidthGranularity;
    uint32_t strictLines;
    uint32_t standardSampleLocations;
    uint64_t optimalBufferCopyOffsetAlignment;
    uint64_t optimalBufferCopyRowPitchAlignment;
    uint64_t nonCoherentAtomSize;
} GpudVkPhysicalDeviceLimits;

typedef struct GpudVkPhysicalDeviceSparseProperties {
    uint32_t residencyStandard2DBlockShape;
    uint32_t residencyStandard2DMultisampleBlockShape;
    uint32_t residencyStandard3DBlockShape;
    uint32_t residencyAlignedMipSize;
    uint32_t residencyNonResidentStrict;
} GpudVkPhysicalDeviceSparseProperties;

typedef struct GpudVkPhysicalDeviceProperties {
    uint32_t                             apiVersion;
    uint32_t                             driverVersion;
    uint32_t                             vendorID;
    uint32_t                             deviceID;
    uint32_t                             deviceType;
    char                                 deviceName[GPUD_VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    uint8_t                              pipelineCacheUUID[GPUD_VK_UUID_SIZE];
    GpudVkPhysicalDeviceLimits           limits;
    GpudVkPhysicalDeviceSparseProperties sparseProperties;
} GpudVkPhysicalDeviceProperties;

typedef struct GpudVkPhysicalDeviceFeatures {
    uint32_t robustBufferAccess;
    uint32_t fullDrawIndexUint32;
    uint32_t imageCubeArray;
    uint32_t independentBlend;
    uint32_t geometryShader;
    uint32_t tessellationShader;
    uint32_t sampleRateShading;
    uint32_t dualSrcBlend;
    uint32_t logicOp;
    uint32_t multiDrawIndirect;
    uint32_t drawIndirectFirstInstance;
    uint32_t depthClamp;
    uint32_t depthBiasClamp;
    uint32_t fillModeNonSolid;
    uint32_t depthBounds;
    uint32_t wideLines;
    uint32_t largePoints;
    uint32_t alphaToOne;
    uint32_t multiViewport;
    uint32_t samplerAnisotropy;
    uint32_t textureCompressionETC2;
    uint32_t textureCompressionASTC_LDR;
    uint32_t textureCompressionBC;
    uint32_t occlusionQueryPrecise;
    uint32_t pipelineStatisticsQuery;
    uint32_t vertexPipelineStoresAndAtomics;
    uint32_t fragmentStoresAndAtomics;
    uint32_t shaderTessellationAndGeometryPointSize;
    uint32_t shaderImageGatherExtended;
    uint32_t shaderStorageImageExtendedFormats;
    uint32_t shaderStorageImageMultisample;
    uint32_t shaderStorageImageReadWithoutFormat;
    uint32_t shaderStorageImageWriteWithoutFormat;
    uint32_t shaderUniformBufferArrayDynamicIndexing;
    uint32_t shaderSampledImageArrayDynamicIndexing;
    uint32_t shaderStorageBufferArrayDynamicIndexing;
    uint32_t shaderStorageImageArrayDynamicIndexing;
    uint32_t shaderClipDistance;
    uint32_t shaderCullDistance;
    uint32_t shaderFloat64;
    uint32_t shaderInt64;
    uint32_t shaderInt16;
    uint32_t shaderResourceResidency;
    uint32_t shaderResourceMinLod;
    uint32_t sparseBinding;
    uint32_t sparseResidencyBuffer;
    uint32_t sparseResidencyImage2D;
    uint32_t sparseResidencyImage3D;
    uint32_t sparseResidency2Samples;
    uint32_t sparseResidency4Samples;
    uint32_t sparseResidency8Samples;
    uint32_t sparseResidency16Samples;
    uint32_t sparseResidencyAliased;
    uint32_t variableMultisampleRate;
    uint32_t inheritedQueries;
} GpudVkPhysicalDeviceFeatures;

typedef struct GpudVkQueueFamilyProperties {
    uint32_t       queueFlags;
    uint32_t       queueCount;
    uint32_t       timestampValidBits;
    GpudVkExtent3D minImageTransferGranularity;
} GpudVkQueueFamilyProperties;

typedef struct GpudVkDeviceQueueCreateInfo {
    uint32_t     sType;
    const void*  pNext;
    uint32_t     flags;
    uint32_t     queueFamilyIndex;
    uint32_t     queueCount;
    const float* pQueuePriorities;
} GpudVkDeviceQueueCreateInfo;

typedef struct GpudVkDeviceCreateInfo {
    uint32_t                            sType;
    const void*                         pNext;
    uint32_t                            flags;
    uint32_t                            queueCreateInfoCount;
    const GpudVkDeviceQueueCreateInfo*  pQueueCreateInfos;
    uint32_t                            enabledLayerCount;
    const char* const*                  ppEnabledLayerNames;
    uint32_t                            enabledExtensionCount;
    const char* const*                  ppEnabledExtensionNames;
    const GpudVkPhysicalDeviceFeatures* pEnabledFeatures;
} GpudVkDeviceCreateInfo;

typedef struct GpudVkApplicationInfo {
    uint32_t    sType;
    const void* pNext;
    const char* pApplicationName;
    uint32_t    applicationVersion;
    const char* pEngineName;
    uint32_t    engineVersion;
    uint32_t    apiVersion;
} GpudVkApplicationInfo;

typedef struct GpudVkInstanceCreateInfo {
    uint32_t                     sType;
    const void*                  pNext;
    uint32_t                     flags;
    const GpudVkApplicationInfo* pApplicationInfo;
    uint32_t                     enabledLayerCount;
    const char* const*           ppEnabledLayerNames;
    uint32_t                     enabledExtensionCount;
    const char* const*           ppEnabledExtensionNames;
} GpudVkInstanceCreateInfo;

__END_DECLS

#endif  // GPUD_INCLUDE_GPUD_GPUD_VK_H_
