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
#include <vulkan/vulkan_core.h>

#define DS_NOT_SUBMITTED (uint64_t)-1
#define DS_DELAY_FRAMES 3
#define DS_EXPECTED_FRAME_FLUSHES 10
#define DS_MAX_SUBMITS (DS_DELAY_FRAMES*DS_EXPECTED_FRAME_FLUSHES)
#define DS_PENDING_RESOURCES_ARRAY 2
#define DS_DELETE_RESOURCES_ARRAY 2

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
	PFN_vkCreateImageView vkCreateImageView;
	PFN_vkDestroyImageView vkDestroyImageView;

	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue queue;
	uint32_t queueFamilyIndex;

	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceProperties properties;
	bool hasPVRTC;

	VkPhysicalDeviceMemoryProperties memoryProperties;
} dsVkDevice;

typedef struct dsVkFormatInfo
{
	VkFormat vkFormat;
	VkFormatProperties properties;
} dsVkFormatInfo;

typedef struct dsVkDirtyRange
{
	size_t start;
	size_t size;
} dsVkDirtyRange;

typedef struct dsVkGfxBufferData
{
	dsAllocator* allocator;
	dsAllocator* scratchAllocator;
	dsSpinlock lock;

	VkDeviceMemory deviceMemory;
	VkBuffer deviceBuffer;
	uint64_t lastUsedSubmit;

	VkDeviceMemory hostMemory;
	VkBuffer hostBuffer;
	uint64_t uploadedSubmit;
	void* submitQueue;

	dsGfxBufferUsage usage;
	size_t size;

	dsVkDirtyRange* dirtyRanges;
	uint32_t dirtyRangeCount;
	uint32_t maxDirtyRanges;

	size_t mappedStart;
	size_t mappedSize;
	bool mappedWrite;

	bool keepHost;
	bool used;
	bool needsInitialCopy;

	uint32_t commandBufferCount;
} dsVkGfxBufferData;

typedef struct dsVkGfxBuffer
{
	dsGfxBuffer buffer;
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

typedef struct dsVkTexture
{
	dsTexture texture;
	dsSpinlock lock;

	VkDeviceMemory deviceMemory;
	VkImage deviceImage;
	VkImageView deviceImageView;
	uint64_t lastUsedSubmit;

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

	bool needsInitialCopy;

	uint32_t commandBufferCount;
} dsVkTexture;

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
} dsVkResourceList;

typedef struct dsVkBarrierList
{
	dsAllocator* allocator;
	dsVkDevice* device;

	VkBufferMemoryBarrier* bufferBarriers;
	uint32_t bufferBarrierCount;
	uint32_t maxBufferBarriers;
} dsVkBarrierList;

typedef struct dsVkCommandBuffer
{
	dsCommandBuffer commandBuffer;
	VkCommandBuffer vkCommandBuffer;
	dsVkResourceList usedResources;
	dsVkBarrierList barriers;
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

	dsVkBarrierList resourceBarriers;
	dsVkResourceList pendingResources[DS_PENDING_RESOURCES_ARRAY];
	dsVkResourceList deleteResources[DS_DELETE_RESOURCES_ARRAY];
	uint32_t curPendingResources;
	uint32_t curDeleteResources;

	VkBufferCopy* bufferCopies;
	uint32_t bufferCopiesCount;
	uint32_t maxBufferCopies;
} dsVkRenderer;

typedef struct dsVkResourceManager
{
	dsResourceManager resourceManager;
	dsVkDevice* device;

	dsVkFormatInfo standardFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	dsVkFormatInfo specialFormats[dsGfxFormat_SpecialCount];
	dsVkFormatInfo compressedFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];
} dsVkResourceManager;
