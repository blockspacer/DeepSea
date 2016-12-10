/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Render/Resources/GfxBuffer.h>

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>
#include <errno.h>

dsGfxBuffer* dsGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	int usage, int memoryHints, size_t size, const void* data)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createBufferFunc || !resourceManager->destroyBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (!usage || (resourceManager->supportedBuffers & usage) != usage)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Requested buffer usage type isn't supported.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (memoryHints == 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"At least one memory hint flag must be set when creating a buffer.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsGfxBuffer* buffer = resourceManager->createBufferFunc(resourceManager, allocator, usage,
		memoryHints, size, data);
	if (buffer)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->bufferCount, 1);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->bufferMemorySize, buffer->size);
	}
	DS_PROFILE_FUNC_RETURN(buffer);
}

void* dsGfxBuffer_map(dsGfxBuffer* buffer, int flags, size_t offset, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->mapBufferFunc ||
		!buffer->resourceManager->unmapBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	dsGfxBufferMapSupport support = resourceManager->bufferMapSupport;
	if (support == dsGfxBufferMapSupport_None)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Buffer mapping not supported on the current device.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}
	else if ((flags & dsGfxBufferMap_Persistent) && support != dsGfxBufferMapSupport_Persistent)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Persistent buffer mapping not supported on the current device.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if ((flags & dsGfxBufferMap_Read) && !(buffer->memoryHints & dsGfxMemory_Read))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to read from a buffer without the read memory flag set.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if ((buffer->memoryHints & dsGfxMemory_GpuOnly))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to map a buffer set with GPU only memory flag set.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if ((size == DS_MAP_FULL_BUFFER && offset > size) ||
		(size != DS_MAP_FULL_BUFFER && offset + size > buffer->size))
	{
		errno = ERANGE;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (flags == 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"At least one buffer map flag must set when mapping a buffer.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	void* ptr;
	if (support == dsGfxBufferMapSupport_Full)
	{
		void* mappedMem = resourceManager->mapBufferFunc(resourceManager, buffer, flags, 0,
			DS_MAP_FULL_BUFFER);
		ptr = ((uint8_t*)mappedMem + offset);
	}
	else
	{
		size_t rem = 0;
		if (resourceManager->minMappingAlignment > 0)
			rem = offset % resourceManager->minMappingAlignment;
		ptr = ((uint8_t*)buffer->resourceManager->mapBufferFunc(resourceManager, buffer, flags,
			offset - rem, size + rem) + rem);
	}

	DS_PROFILE_FUNC_RETURN(ptr);
}

bool dsGfxBuffer_unmap(dsGfxBuffer* buffer)
{
	DS_PROFILE_FUNC_START();

	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->unmapBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->unmapBufferFunc(resourceManager, buffer);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxBuffer_flush(dsGfxBuffer* buffer, size_t offset, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->flushBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (resourceManager->bufferMapSupport != dsGfxBufferMapSupport_Persistent)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Persistent buffer mapping not supported on the current device.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (buffer->memoryHints & dsGfxMemory_Coherent)
	{
		DS_PROFILE_FUNC_RETURN(true);
	}

	bool success = resourceManager->flushBufferFunc(resourceManager, buffer, offset, size);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxBuffer_invalidate(dsGfxBuffer* buffer, size_t offset, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->invalidateBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (resourceManager->bufferMapSupport != dsGfxBufferMapSupport_Persistent)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Persistent buffer mapping not supported on the current device.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (buffer->memoryHints & dsGfxMemory_Coherent)
	{
		DS_PROFILE_FUNC_RETURN(true);
	}

	bool success = resourceManager->invalidateBufferFunc(resourceManager, buffer, offset, size);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxBuffer_copyData(dsGfxBuffer* buffer, size_t offset, size_t size, const void* data)
{
	DS_PROFILE_FUNC_START();

	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->copyBufferDataFunc ||
		!data)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(buffer->usage & dsGfxBufferUsage_CopyTo))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a buffer without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (offset + size > buffer->size)
	{
		errno = ERANGE;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->copyBufferDataFunc(resourceManager, buffer, offset, size, data);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxBuffer_copy(dsGfxBuffer* srcBuffer, size_t srcOffset, dsGfxBuffer* dstBuffer,
	size_t dstOffset, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!srcBuffer || !srcBuffer->resourceManager || !srcBuffer->resourceManager->copyBufferFunc ||
		!dstBuffer || dstBuffer->resourceManager != srcBuffer->resourceManager)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(srcBuffer->usage & dsGfxBufferUsage_CopyFrom))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data from a buffer without the copy from usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(dstBuffer->usage & dsGfxBufferUsage_CopyTo))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a buffer without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (srcOffset + size > srcBuffer->size || dstOffset + size > dstBuffer->size)
	{
		errno = ERANGE;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = srcBuffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->copyBufferFunc(resourceManager, srcBuffer, srcOffset, dstBuffer,
		dstOffset, size);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxBuffer_destroy(dsGfxBuffer* buffer)
{
	DS_PROFILE_FUNC_START();

	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->destroyBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resources can only be manipulated from the main thread or "
			"threads that have created a resource context.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	size_t size = buffer->size;
	bool success = resourceManager->destroyBufferFunc(resourceManager, buffer);
	if (success)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->bufferCount, -1);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->bufferMemorySize, -size);
	}
	DS_PROFILE_FUNC_RETURN(success);
}