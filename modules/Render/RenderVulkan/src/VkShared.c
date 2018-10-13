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

#include "VkShared.h"
#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

typedef struct LastCallsite
{
	const char* lastFile;
	const char* lastFunction;
	unsigned int lastLine;
} LastCallsite;

static DS_THREAD_LOCAL LastCallsite lastCallsite;

bool dsHandleVkResult(VkResult result)
{
	switch (result)
	{
		case VK_SUCCESS:
			return true;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			errno = ENOMEM;
			return false;
		default:
			errno = EPERM;
			return false;
	}
}

void dsSetLastVkCallsite(const char* file, const char* function, unsigned int line)
{
	LastCallsite* curLastCallsite = &lastCallsite;
	curLastCallsite->lastFile = file;
	curLastCallsite->lastFunction = function;
	curLastCallsite->lastLine = line;
}

void dsGetLastVkCallsite(const char** file, const char** function, unsigned int* line)
{
	const LastCallsite* curLastCallsite = &lastCallsite;
	*file = curLastCallsite->lastFile;
	*function = curLastCallsite->lastFunction;
	*line = curLastCallsite->lastLine;
}

uint32_t dsVkMemoryIndex(const dsVkDevice* device, const VkMemoryRequirements* requirements,
	dsGfxMemory memoryFlags)
{
	uint32_t requiredFlags = 0;
	uint32_t optimalFlags = 0;
	if (!(memoryFlags & dsGfxMemory_GPUOnly))
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	if (memoryFlags & dsGfxMemory_Coherent)
		requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	if (memoryFlags & (dsGfxMemory_Dynamic | dsGfxMemory_Stream))
		optimalFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

	uint32_t memoryIndex = DS_INVALID_HEAP;
	VkDeviceSize memorySize = 0;

	const VkPhysicalDeviceMemoryProperties* memoryProperties = &device->memoryProperties;
	for (uint32_t curBitmask = requirements->memoryTypeBits; curBitmask;
		curBitmask = dsRemoveLastBit(curBitmask))
	{
		uint32_t i = dsBitmaskIndex(curBitmask);
		const VkMemoryType* memoryType = memoryProperties->memoryTypes + i;
		if ((memoryType->propertyFlags & requiredFlags) != requiredFlags)
			continue;

		if (memoryIndex == DS_INVALID_HEAP)
			memoryIndex = i;

		// Find the largest optimal heap.
		VkDeviceSize size = memoryProperties->memoryHeaps[memoryType->heapIndex].size;
		if ((memoryType->propertyFlags & optimalFlags) == optimalFlags && size > memorySize)
			memoryIndex = i;
	}

	if (memoryIndex == DS_INVALID_HEAP)
	{
		errno = ENOMEM;
		DS_LOG_ERROR(DS_RENDER_VULKAN_LOG_TAG, "No suitable GPU heap found.");
	}

	return memoryIndex;
}

VkDeviceMemory dsAllocateVkMemory(const dsVkDevice* device,
	const VkMemoryRequirements* requirements, uint32_t memoryIndex)
{
	if (memoryIndex == DS_INVALID_HEAP)
	{
		errno = ENOMEM;
		return 0;
	}

	VkMemoryAllocateInfo allocInfo =
	{
		VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
		NULL,
		requirements->size,
		memoryIndex
	};
	const dsVkInstance* instance = &device->instance;
	VkDeviceMemory memory = 0;
	VkResult result = DS_VK_CALL(device->vkAllocateMemory)(device->device, &allocInfo,
		instance->allocCallbacksPtr, &memory);
	if (!dsHandleVkResult(result))
		return 0;

	return memory;
}