/*
 * Copyright 2016-2019 Aaron Barany
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

#include "Resources/RenderResourceHelpers.h"
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>

extern const char* dsResourceManager_noContextError;

inline static void adjustAlignment(size_t alignment, size_t totalSize, size_t* offset, size_t* size,
	size_t* rem)
{
	if (alignment > 0)
	{
		*rem = *offset % alignment;
		*offset -= *rem;
		if (*size != DS_MAP_FULL_BUFFER)
		{
			*size += *rem;

			size_t count = (*size + alignment - 1)/alignment;
			*size = count*alignment;
			*size = dsMin(*size, totalSize - *offset);
		}
	}
	else
		rem = 0;
}

dsGfxBuffer* dsGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsGfxBufferUsage usage, dsGfxMemory memoryHints, const void* data, size_t size)
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

	if (memoryHints & dsGfxMemory_Coherent &&
		resourceManager->bufferMapSupport != dsGfxBufferMapSupport_Persistent)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Coherent buffer memory isn't supported when persistent "
			"mapping isn't natively supported.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsGfxBuffer* buffer = resourceManager->createBufferFunc(resourceManager, allocator, usage,
		memoryHints, data, size);
	if (buffer)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->bufferCount, 1);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->bufferMemorySize, buffer->size);
	}
	DS_PROFILE_FUNC_RETURN(buffer);
}

void* dsGfxBuffer_map(dsGfxBuffer* buffer, dsGfxBufferMap flags, size_t offset, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->mapBufferFunc ||
		!buffer->resourceManager->unmapBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!(flags & dsGfxBufferMap_Read) && !(flags & dsGfxBufferMap_Write))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Buffer map flags must contain at least one of "
			"dsGfxBufferMap_Read or dsGfxBufferMap_Write.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if ((flags & dsGfxBufferMap_Read) && !(buffer->memoryHints & dsGfxMemory_Read))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to read from a buffer without the read memory flag set.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if ((buffer->memoryHints & dsGfxMemory_GPUOnly))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to map a buffer with GPU only memory flag set.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if ((flags & dsGfxBufferMap_Persistent) && !(buffer->memoryHints & dsGfxMemory_Persistent))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to persistently map a buffer without the persistent memory flag set.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if ((flags & dsGfxBufferMap_Orphan) && (flags & dsGfxBufferMap_Read))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot both read from and orphan a buffer.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if ((size == DS_MAP_FULL_BUFFER && offset > size) ||
		(size != DS_MAP_FULL_BUFFER && !DS_IS_BUFFER_RANGE_VALID(offset, size, buffer->size)))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to map a buffer out of range.");
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
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (size == 0)
		DS_PROFILE_FUNC_RETURN(NULL);

	size_t rem = 0;
	uint32_t alignment = resourceManager->minNonCoherentMappingAlignment;
	if (alignment > 0 && !(buffer->memoryHints & dsGfxMemory_Coherent))
		adjustAlignment(alignment, buffer->size, &offset, &size, &rem);
	void* ptr = buffer->resourceManager->mapBufferFunc(resourceManager, buffer, flags, offset,
		size);
	if (!ptr)
		DS_PROFILE_FUNC_RETURN(NULL);
	ptr = (uint8_t*)ptr + rem;

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
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
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
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (buffer->memoryHints & dsGfxMemory_Coherent)
	{
		DS_PROFILE_FUNC_RETURN(true);
	}

	size_t rem;
	adjustAlignment(resourceManager->minNonCoherentMappingAlignment, buffer->size, &offset, &size,
		&rem);

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
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (buffer->memoryHints & dsGfxMemory_Coherent)
	{
		DS_PROFILE_FUNC_RETURN(true);
	}

	size_t rem;
	adjustAlignment(resourceManager->minNonCoherentMappingAlignment, buffer->size, &offset, &size,
		&rem);

	bool success = resourceManager->invalidateBufferFunc(resourceManager, buffer, offset, size);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxBuffer_copyData(dsGfxBuffer* buffer, dsCommandBuffer* commandBuffer, size_t offset,
	const void* data, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !buffer || !buffer->resourceManager ||
		!buffer->resourceManager->copyBufferDataFunc || !data)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(buffer->usage & dsGfxBufferUsage_CopyTo))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a buffer without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(offset, size, buffer->size))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy buffer data out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Buffer copying must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Buffer copying must be performed outside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	bool success = resourceManager->copyBufferDataFunc(resourceManager, commandBuffer, buffer,
		offset, data, size);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxBuffer_copy(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer, size_t srcOffset,
	dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !srcBuffer || !srcBuffer->resourceManager ||
		!srcBuffer->resourceManager->copyBufferFunc || !dstBuffer ||
		dstBuffer->resourceManager != srcBuffer->resourceManager)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = srcBuffer->resourceManager;
	if (!resourceManager->canCopyBuffers)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Buffers cannot be copied between each other on the current target.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(srcBuffer->usage & dsGfxBufferUsage_CopyFrom))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data from a buffer without the copy from usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(dstBuffer->usage & dsGfxBufferUsage_CopyTo))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a buffer without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(srcOffset, size, srcBuffer->size) ||
		!DS_IS_BUFFER_RANGE_VALID(dstOffset, size, dstBuffer->size))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to copy buffer data out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Buffer copying must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Buffer copying must be performed outside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->copyBufferFunc(resourceManager, commandBuffer, srcBuffer,
		srcOffset, dstBuffer, dstOffset, size);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsGfxBuffer_copyToTexture(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	dsTexture* dstTexture, const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !srcBuffer || !srcBuffer->resourceManager ||
		!srcBuffer->resourceManager->copyBufferToTextureFunc || !dstTexture ||
		dstTexture->resourceManager != srcBuffer->resourceManager|| (!regions && regionCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = srcBuffer->resourceManager;
	if (!dsGfxFormat_copyBufferToTextureSupported(resourceManager, dstTexture->info.format))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture format cannot be copied from buffer to texture on the current target.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(srcBuffer->usage & dsGfxBufferUsage_CopyFrom))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data from a buffer without the copy from usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(dstTexture->usage & dsTextureUsage_CopyTo))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Attempting to copy data to a texture without the copy to usage flag set.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, dstTexture->info.format));

	for (size_t i = 0; i < regionCount; ++i)
	{
		if (!dsIsGfxBufferTextureCopyRegionValid(regions + i, &dstTexture->info, srcBuffer->size))
			DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Buffer to texture copying must be performed inside of a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Buffer to texture copying must be performed outside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->copyBufferToTextureFunc(resourceManager, commandBuffer,
		srcBuffer, dstTexture, regions, regionCount);
	DS_PROFILE_FUNC_RETURN(success);
}

void dsGfxBuffer_process(dsGfxBuffer* buffer)
{
	DS_PROFILE_FUNC_START();

	if (!buffer || !buffer->resourceManager || !buffer->resourceManager->processBufferFunc)
		DS_PROFILE_FUNC_RETURN_VOID();

	dsResourceManager* resourceManager = buffer->resourceManager;
	resourceManager->processBufferFunc(resourceManager, buffer);
	DS_PROFILE_FUNC_RETURN_VOID();
}

bool dsGfxBuffer_destroy(dsGfxBuffer* buffer)
{
	if (!buffer)
		return true;

	DS_PROFILE_FUNC_START();

	if (!buffer->resourceManager || !buffer->resourceManager->destroyBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
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
