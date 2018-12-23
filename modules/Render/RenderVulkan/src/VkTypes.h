/*
 * Copyright 2018 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Types.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/RenderVulkan/RendererIDs.h>

#include <MSL/Client/ModuleC.h>
#include <vulkan/vulkan_core.h>

#define DS_NOT_SUBMITTED (uint64_t)-1
#define DS_DELAY_FRAMES 3
#define DS_EXPECTED_FRAME_FLUSHES 10
#define DS_MAX_SUBMITS (DS_DELAY_FRAMES*DS_EXPECTED_FRAME_FLUSHES)
#define DS_PENDING_RESOURCES_ARRAY 2
#define DS_DELETE_RESOURCES_ARRAY 2
// 10 seconds in nanoseconds
#define DS_DEFAULT_WAIT_TIMEOUT 10000000000
#define DS_MAX_DYNAMIC_STATES VK_DYNAMIC_STATE_STENCIL_REFERENCE + 1

typedef struct dsVkInstance
{
	dsDynamicLib library;

	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
	PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
	PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
	PFN_vkCreateInstance vkCreateInstance;
	PFN_vkDestroyInstance vkDestroyInstance;
	PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
	PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
	PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
	PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR;
	PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
	PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
	PFN_vkCreateDevice vkCreateDevice;
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
	PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
	PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
	PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;

	VkDebugUtilsMessengerEXT debugCallback;

	VkAllocationCallbacks allocCallbacks;
	const VkAllocationCallbacks* allocCallbacksPtr;
	VkInstance instance;
} dsVkInstance;

typedef struct dsVkDevice
{
	dsVkInstance instance;

	PFN_vkDestroyDevice vkDestroyDevice;
	PFN_vkGetDeviceQueue vkGetDeviceQueue;

	PFN_vkCreateCommandPool vkCreateCommandPool;
	PFN_vkResetCommandPool vkResetCommandPool;
	PFN_vkDestroyCommandPool vkDestroyCommandPool;
	PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
	PFN_vkResetCommandBuffer vkResetCommandBuffer;
	PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
	PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
	PFN_vkEndCommandBuffer vkEndCommandBuffer;
	PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
	PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
	PFN_vkQueueSubmit vkQueueSubmit;
	PFN_vkQueueWaitIdle vkQueueWaitIdle;

	PFN_vkCreateFence vkCreateFence;
	PFN_vkDestroyFence vkDestroyFence;
	PFN_vkResetFences vkResetFences;

	PFN_vkWaitForFences vkWaitForFences;
	PFN_vkAllocateMemory vkAllocateMemory;
	PFN_vkFreeMemory vkFreeMemory;
	PFN_vkMapMemory vkMapMemory;
	PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
	PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
	PFN_vkUnmapMemory vkUnmapMemory;

	PFN_vkCreateBuffer vkCreateBuffer;
	PFN_vkDestroyBuffer vkDestroyBuffer;
	PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
	PFN_vkBindBufferMemory vkBindBufferMemory;
	PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
	PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
	PFN_vkCreateBufferView vkCreateBufferView;
	PFN_vkDestroyBufferView vkDestroyBufferView;

	PFN_vkCreateImage vkCreateImage;
	PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
	PFN_vkDestroyImage vkDestroyImage;
	PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
	PFN_vkBindImageMemory vkBindImageMemory;
	PFN_vkCmdCopyImage vkCmdCopyImage;
	PFN_vkCmdBlitImage vkCmdBlitImage;
	PFN_vkCreateImageView vkCreateImageView;
	PFN_vkDestroyImageView vkDestroyImageView;

	PFN_vkCreateFramebuffer vkCreateFramebuffer;
	PFN_vkDestroyFramebuffer vkDestroyFramebuffer;

	PFN_vkCreateRenderPass vkCreateRenderPass;
	PFN_vkDestroyRenderPass vkDestroyRenderPass;

	PFN_vkCreateQueryPool vkCreateQueryPool;
	PFN_vkDestroyQueryPool vkDestroyQueryPool;
	PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
	PFN_vkCmdBeginQuery vkCmdBeginQuery;
	PFN_vkCmdEndQuery vkCmdEndQuery;
	PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
	PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
	PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;

	PFN_vkCreateShaderModule vkCreateShaderModule;
	PFN_vkDestroyShaderModule vkDestroyShaderModule;

	PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
	PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;

	PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
	PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
	PFN_vkResetDescriptorPool vkResetDescriptorPool;
	PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
	PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
	PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;

	PFN_vkCreateSampler vkCreateSampler;
	PFN_vkDestroySampler vkDestroySampler;

	PFN_vkCreatePipelineCache vkCreatePipelineCache;
	PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
	PFN_vkGetPipelineCacheData vkGetPipelineCacheData;

	PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
	PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;

	PFN_vkCreateComputePipelines vkCreateComputePipelines;
	PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
	PFN_vkDestroyPipeline vkDestroyPipeline;

	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue queue;
	uint32_t queueFamilyIndex;

	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceProperties properties;
	bool hasPVRTC;
	bool hasLazyAllocation;

	VkPhysicalDeviceMemoryProperties memoryProperties;
} dsVkDevice;

typedef struct dsVkFormatInfo
{
	VkFormat vkFormat;
	VkFormatProperties properties;
} dsVkFormatInfo;

typedef struct dsVkResource
{
	dsSpinlock lock;
	uint64_t lastUsedSubmit;
	uint32_t commandBufferCount;
} dsVkResource;

typedef struct dsVkDirtyRange
{
	size_t start;
	size_t size;
} dsVkDirtyRange;

typedef struct dsVkBufferView
{
	VkBufferView bufferView;
	dsGfxFormat format;
	size_t offset;
	size_t count;
} dsVkBufferView;

typedef struct dsVkGfxBufferData
{
	dsResourceManager* resourceManager;
	dsAllocator* allocator;
	dsAllocator* scratchAllocator;

	dsVkResource resource;

	VkDeviceMemory deviceMemory;
	VkBuffer deviceBuffer;

	VkDeviceMemory hostMemory;
	VkBuffer hostBuffer;
	uint64_t uploadedSubmit;
	void* submitQueue;

	dsGfxBufferUsage usage;
	dsGfxMemory memoryHints;
	size_t size;

	dsVkDirtyRange* dirtyRanges;
	uint32_t dirtyRangeCount;
	uint32_t maxDirtyRanges;

	dsSpinlock bufferViewLock;
	dsVkBufferView* bufferViews;
	uint32_t bufferViewCount;
	uint32_t maxBufferViews;

	size_t mappedStart;
	size_t mappedSize;
	bool mappedWrite;

	bool keepHost;
	bool used;
	bool needsInitialCopy;
} dsVkGfxBufferData;

typedef struct dsVkGfxBuffer
{
	dsGfxBuffer buffer;
	dsSpinlock lock;
	dsVkGfxBufferData* bufferData;
} dsVkGfxBuffer;

typedef struct dsVkDrawGeometry
{
	dsDrawGeometry drawGeometry;
	uint32_t vertexHash;
} dsVkDrawGeometry;

typedef struct dsVkHostImage
{
	VkImage image;
	size_t offset;
	VkSubresourceLayout layout;
} dsVkHostImage;

typedef struct dsVkCopyImage
{
	dsVkResource resource;
	dsAllocator* allocator;
	dsVkDevice* device;
	dsTexture* texture;
	VkImage* images;
	VkImageMemoryBarrier* imageBarriers;
	VkImageCopy* imageCopies;
	uint32_t imageCount;
	uint32_t imageCopyCount;
	VkDeviceMemory memory;
} dsVkCopyImage;

typedef struct dsVkTexture
{
	dsTexture texture;
	dsVkResource resource;

	VkDeviceMemory deviceMemory;
	VkImage deviceImage;
	VkImageView deviceImageView;

	VkDeviceMemory hostMemory;
	VkImage hostImage;
	uint32_t hostImageCount;
	dsVkHostImage* hostImages;
	uint64_t uploadedSubmit;
	void* submitQueue;

	VkDeviceMemory surfaceMemory;
	VkImage surfaceImage;
	VkImageView surfaceImageView;
	uint64_t lastDrawSubmit;

	VkImageAspectFlags aspectMask;
	bool needsInitialCopy;
} dsVkTexture;

typedef struct dsVkRenderbuffer
{
	dsRenderbuffer renderbuffer;
	dsVkResource resource;

	VkDeviceMemory memory;
	VkImage image;
	VkImageView imageView;
} dsVkRenderbuffer;

typedef struct dsVkRealFramebuffer
{
	dsAllocator* allocator;
	dsVkDevice* device;
	dsVkResource resource;
	VkFramebuffer framebuffer;
	VkRenderPass renderPass;

	VkImageView* imageViews;
	bool* imageViewsTemp;
	uint32_t surfaceCount;
} dsVkRealFramebuffer;

typedef struct dsVkFramebuffer
{
	dsFramebuffer framebuffer;
	dsAllocator* scratchAllocator;
	dsSpinlock lock;

	dsVkRealFramebuffer** realFramebuffers;
	uint32_t framebufferCount;
	uint32_t maxFramebuffers;
} dsVkFramebuffer;

typedef struct dsVkGfxFence
{
	dsGfxFence fence;
	dsVkResource resource;
} dsVkGfxFence;

typedef struct dsVkGfxQueryPool
{
	dsGfxQueryPool queries;
	dsVkResource resource;
	VkQueryPool vkQueries;
} dsVkGfxQueryPool;

typedef struct dsVkMaterialDesc
{
	dsMaterialDesc materialDesc;
	uint32_t* elementMappings;
	VkDescriptorSetLayoutBinding* bindings[2];
	VkDescriptorSetLayout descriptorSets[2];
} dsVkMaterialDesc;

typedef struct dsVkSamplerList
{
	dsResourceManager* resourceManager;
	dsAllocator* allocator;
	dsVkResource resource;
	VkSampler* samplers;
	uint32_t samplerCount;
	float defaultAnisotropy;
} dsVkSamplerList;

typedef struct dsVkTexelBufferBinding
{
	dsVkGfxBufferData* buffer;
	dsGfxFormat format;
	size_t offset;
	size_t count;
} dsVkTexelBufferBinding;

typedef struct dsVkGfxBufferBinding
{
	dsVkGfxBufferData* buffer;
	dsGfxFormat format;
	size_t offset;
	size_t size;
} dsVkGfxBufferBinding;

typedef struct dsVkMaterialDescriptor
{
	dsRenderer* renderer;
	dsAllocator* allocator;
	dsVkResource resource;
	dsLifetime* shader;
	const dsVkSamplerList* samplers; // Only used for comparison

	dsTexture** textures;
	dsVkGfxBufferBinding* buffers;
	dsVkTexelBufferBinding* texelBuffers;

	VkDescriptorPool pool;
	VkDescriptorSet set;
} dsVkMaterialDescriptor;

struct dsDeviceMaterial
{
	dsResourceManager* resourceManager;
	dsAllocator* allocator;
	dsAllocator* scratchAllocator;
	dsMaterial* material;
	dsLifetime* lifetime;

	dsVkMaterialDescriptor** descriptors;
	uint32_t descriptorCount;
	uint32_t maxDescriptors;

	VkWriteDescriptorSet* bindings;
	VkDescriptorImageInfo* imageInfos;
	VkDescriptorBufferInfo* bufferInfos;
	VkBufferView* bufferViews;

	dsTexture** textures;
	dsVkGfxBufferBinding* buffers;
	dsVkTexelBufferBinding* texelBuffers;

	uint32_t bindingCount;
	uint32_t imageInfoCount;
	uint32_t bufferInfoCount;
	uint32_t bufferViewCount;

	dsSpinlock lock;
};

typedef struct dsVkComputePipeline
{
	dsAllocator* allocator;
	dsVkResource resource;
	dsVkDevice* device;
	VkPipeline pipeline;
} dsVkComputePipeline;

typedef struct dsVkPipeline
{
	dsAllocator* allocator;
	dsVkResource resource;
	dsVkDevice* device;

	VkPipeline pipeline;

	uint32_t hash;
	uint32_t samples;
	float defaultAnisotropy;
	uint32_t subpass;
	dsPrimitiveType primitiveType;
	dsVertexFormat formats[DS_MAX_GEOMETRY_VERTEX_BUFFERS];
	dsLifetime* renderPass;
} dsVkPipeline;

typedef struct dsVkSamplerMapping
{
	uint32_t uniformIndex;
	uint32_t samplerIndex;
} dsVkSamplerMapping;

typedef struct dsVkShader
{
	dsShader shader;
	dsAllocator* scratchAllocator;
	dsLifetime* lifetime;
	mslPipeline pipeline;

	VkShaderStageFlags stages;
	mslSizedData spirv[mslStage_Count];
	VkShaderModule shaders[mslStage_Count];
	VkPipelineLayout layout;

	VkPipelineTessellationStateCreateInfo tessellationInfo;
	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkSampleMask sampleMask;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	VkPipelineColorBlendStateCreateInfo blendInfo;
	VkPipelineColorBlendAttachmentState attachments[DS_MAX_ATTACHMENTS];
	VkPipelineDynamicStateCreateInfo dynamicInfo;
	VkDynamicState dynamicStates[DS_MAX_DYNAMIC_STATES];

	bool dynamicLineWidth;
	bool dynamicDepthBias;
	bool dynamicBlendConstants;
	bool dynamicDepthBounds;
	bool dynamicStencilCompareMask;
	bool dynamicStencilWriteMask;
	bool dynamicStencilReference;

	dsVkComputePipeline* computePipeline;

	dsLifetime** usedMaterials;
	uint32_t usedMaterialCount;
	uint32_t maxUsedMaterials;

	dsLifetime** usedRenderPasses;
	uint32_t usedRenderPassCount;
	uint32_t maxUsedRenderPasses;

	dsVkPipeline** pipelines;
	uint32_t pipelineCount;
	uint32_t maxPipelines;

	dsVkSamplerList* samplers;
	dsVkSamplerMapping* samplerMapping;
	uint32_t samplerCount;
	bool samplersHaveDefaultAnisotropy;

	dsSpinlock materialLock;
	dsSpinlock pipelineLock;
	dsSpinlock samplerLock;
} dsVkShader;

typedef struct dsVkRenderPass
{
	dsRenderPass renderPass;
	dsVkResource resource;
	dsAllocator* scratchAllocator;
	dsLifetime* lifetime;

	VkRenderPass vkRenderPass;

	dsLifetime** usedShaders;
	uint32_t usedShaderCount;
	uint32_t maxUsedShaders;

	dsSpinlock shaderLock;
} dsVkRenderPass;

typedef struct dsVkSubmitInfo
{
	uint64_t submitIndex;
	VkCommandBuffer resourceCommands;
	VkCommandBuffer renderCommands;
	VkFence fence;
} dsVkSubmitInfo;

typedef struct dsVkResourceList
{
	dsAllocator* allocator;

	dsVkGfxBufferData** buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;

	dsTexture** textures;
	uint32_t textureCount;
	uint32_t maxTextures;

	dsVkCopyImage** copyImages;
	uint32_t copyImageCount;
	uint32_t maxCopyImages;

	dsRenderbuffer** renderbuffers;
	uint32_t renderbufferCount;
	uint32_t maxRenderbuffers;

	dsVkRealFramebuffer** framebuffers;
	uint32_t framebufferCount;
	uint32_t maxFramebuffers;

	dsGfxFence** fences;
	uint32_t fenceCount;
	uint32_t maxFences;

	dsGfxQueryPool** queries;
	uint32_t queryCount;
	uint32_t maxQueries;

	dsVkMaterialDescriptor** descriptors;
	uint32_t descriptorCount;
	uint32_t maxDescriptors;

	dsVkSamplerList** samplers;
	uint32_t samplerCount;
	uint32_t maxSamplers;

	dsVkComputePipeline** computePipelines;
	uint32_t computePipelineCount;
	uint32_t maxComputePipelines;

	dsVkPipeline** pipelines;
	uint32_t pipelineCount;
	uint32_t maxPipelines;
} dsVkResourceList;

typedef struct dsVkBarrierList
{
	dsAllocator* allocator;
	dsVkDevice* device;

	VkBufferMemoryBarrier* bufferBarriers;
	uint32_t bufferBarrierCount;
	uint32_t maxBufferBarriers;

	VkImageMemoryBarrier* imageBarriers;
	uint32_t imageBarrierCount;
	uint32_t maxImageBarriers;
} dsVkBarrierList;

typedef struct dsVkBufferCopyInfo
{
	VkBuffer srcBuffer;
	VkBuffer dstBuffer;
	uint32_t firstRange;
	uint32_t rangeCount;
} dsVkBufferCopyInfo;

typedef struct dsVkImageCopyInfo
{
	VkImage srcImage;
	VkImage dstImage;
	VkImageLayout srcLayout;
	VkImageLayout dstLayout;
	uint32_t firstRange;
	uint32_t rangeCount;
} dsVkImageCopyInfo;

typedef struct dsVkCommandBuffer
{
	dsCommandBuffer commandBuffer;
	VkCommandBuffer vkCommandBuffer;
	dsVkBarrierList barriers;

	dsVkResource** usedResources;
	uint32_t usedResourceCount;
	uint32_t maxUsedResources;

	dsOffscreen** readbackOffscreens;
	uint32_t readbackOffscreenCount;
	uint32_t maxReadbackOffscreens;

	VkImageMemoryBarrier* imageBarriers;
	VkImageCopy* imageCopies;
	uint32_t maxImageBarriers;
	uint32_t maxImageCopies;

	bool fenceSet;
	bool fenceReadback;
} dsVkCommandBuffer;

typedef struct dsVkRenderer
{
	dsRenderer renderer;
	dsVkDevice device;

	dsSpinlock resourceLock;
	dsSpinlock deleteLock;
	dsMutex* submitLock;
	dsConditionVariable* waitCondition;

	uint64_t submitCount;
	uint64_t finishedSubmitCount;
	VkCommandPool commandPool;
	dsVkSubmitInfo submits[DS_MAX_SUBMITS];
	uint32_t curSubmit;
	uint32_t waitCount;

	dsVkCommandBuffer mainCommandBuffer;

	dsVkBarrierList preResourceBarriers;
	dsVkBarrierList postResourceBarriers;
	dsVkResourceList pendingResources[DS_PENDING_RESOURCES_ARRAY];
	dsVkResourceList deleteResources[DS_DELETE_RESOURCES_ARRAY];
	uint32_t curPendingResources;
	uint32_t curDeleteResources;

	VkBufferCopy* bufferCopies;
	uint32_t bufferCopiesCount;
	uint32_t maxBufferCopies;

	dsVkBufferCopyInfo* bufferCopyInfos;
	uint32_t bufferCopyInfoCount;
	uint32_t maxBufferCopyInfos;

	VkImageCopy* imageCopies;
	uint32_t imageCopyCount;
	uint32_t maxImageCopies;

	dsVkImageCopyInfo* imageCopyInfos;
	uint32_t imageCopyInfoCount;
	uint32_t maxImageCopyInfos;
} dsVkRenderer;

typedef struct dsVkResourceManager
{
	dsResourceManager resourceManager;
	dsVkDevice* device;

	dsVkFormatInfo standardFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	dsVkFormatInfo specialFormats[dsGfxFormat_SpecialCount];
	dsVkFormatInfo compressedFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	const char* shaderCacheDir;
	VkPipelineCache pipelineCache;
} dsVkResourceManager;
