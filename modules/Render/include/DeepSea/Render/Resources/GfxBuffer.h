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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for using graphics buffers.
 *
 * Unless a command buffer is passed as an argument, all functions must either be called on the main
 * thread or on a thread with an active resource context. A resource shouldn't be accessed
 * simultaneously across multiple threads.
 *
 * @see dsGfxBuffer
 */

/**
 * @brief Creates a graphics buffer.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the buffer from.
 * @param allocator The allocator to create the graphics buffer with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param usage How the buffer will be used. This should be a combination of dsGfxBufferUsage flags.
 * @param memoryHints Hints for how the memory for the buffer will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @param data The initial data for the buffer, or NULL to leave uninitialized. This must be the
 *     same size as the buffer.
 * @param size The size of the buffer. This must be given even if data is NULL.
 * @return The created buffer, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsGfxBuffer* dsGfxBuffer_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsGfxBufferUsage usage, dsGfxMemory memoryHints, const void* data,
	size_t size);

/**
 * @brief Maps a range of a graphics buffer to memory.
 * @remark When mapping in persistent mode and the memory type isn't coherent, this will NOT
 *     implicitly invalidate the mapped range for readback.
 * @remark errno will be set on failure.
 * @param buffer The buffer to map.
 * @param flags The flags describing how to map the memory. This should be a combination of
 *     dsGfxBufferMap flags
 * @param offset The offset into the buffer to map. This must be aligned with minMappingAlignment
 *     from dsResourceManager.
 * @param size The number of bytes to map. This may be set to DS_MAP_FULL_BUFFER to map from the
 *     offset to the end of the buffer.
 * @return A pointer to the mapped memory or NULL if the memory couldn't be mapped.
 */
DS_RENDER_EXPORT void* dsGfxBuffer_map(dsGfxBuffer* buffer, dsGfxBufferMap flags, size_t offset,
	size_t size);

/**
 * @brief Unmaps previously mapped memory from a graphics buffer.
 * @remark When mapping in persistent mode and the memory type isn't coherent, this will NOT
 *     implicitly flush the mapped range for writing.
 * @remark errno will be set on failure.
 * @param buffer The buffer to unmap.
 * @return False if the memory couldn't be unmapped.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_unmap(dsGfxBuffer* buffer);

/**
 * @brief Flushes writes to a mapped memory range for a buffer.
 *
 * This is generally used for persistently mapped memory for a non-coherent buffer. This guarantees
 * writes from the CPU will be visible from the GPU.
 *
 * @remark errno will be set on failure.
 * @param buffer The buffer to flush.
 * @param offset The offset of the range to flush.
 * @param size The size of the memory to flush.
 * @return False if the memory couldn't be flushed.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_flush(dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Invlidates reads to a mapped memory range for a buffer.
 *
 * This is generally used for persistently mapped memory for a non-coherent buffer. This guarantees
 * writes from the GPU will be visible from the CPU. This must be done even if you only write to the
 * memory range.
 *
 * @remark errno will be set on failure.
 * @param buffer The buffer to invalidate.
 * @param offset The offset of the range to invalidate.
 * @param size The size of the memory to invalidate.
 * @return False if the memory couldn't be invalidated.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_invalidate(dsGfxBuffer* buffer, size_t offset, size_t size);

/**
 * @brief Copies data to a buffer on the command queue.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param buffer The buffer to copy the data to. This must have been created with
 *     dsGfxBufferUsage_CopyTo.
 * @param commandBuffer The command buffer to process the copy on.
 * @param offset The offset into the buffer.
 * @param data The data to copy to the buffer.
 * @param size The size of the data to copy.
 * @return False if the data couldn't be copied.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_copyData(dsGfxBuffer* buffer, dsCommandBuffer* commandBuffer,
	size_t offset, const void* data, size_t size);

/**
 * @brief Copies data from one buffer to another.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to process the copy on.
 * @param srcBuffer The buffer to copy the data from. This must have been created with
 *     dsGfxBufferUsage_CopyFrom.
 * @param srcOffset The offset into the source buffer.
 * @param dstBuffer The buffer to copy to. This must have been created with dsGfxBufferUsage_CopyTo.
 * @param dstOffset The offset into the destination buffer.
 * @param size The size of the data to copy.
 * @return False if the data couldn't be copied.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_copy(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size);

/**
 * @brief Copies data from a buffer to a texture.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to process the copy on.
 * @param srcBuffer The buffer to copy the data from. This must have been created with
 *     dsGfxBufferUsage_CopyFrom.
 * @param dstTexture The texture to copy to. This must have been created with dsTextureUsage_CopyTo.
 * @param regions The regions to copy.
 * @param regionCount The number of regions to copy.
 * @return False if the data couldn't be copied.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_copyToTexture(dsCommandBuffer* commandBuffer,
	dsGfxBuffer* srcBuffer, dsTexture* dstTexture, const dsGfxBufferTextureCopyRegion* regions,
	uint32_t regionCount);

/**
 * @brief Processes a buffer to prepare it for rendering.
 *
 * This may be used to cause buffer processing to be done ealier in order to do the work before
 * it's used. This is a hint, and there's no guarantee that anything will be done.
 *
 * @param buffer The buffer to process.
 */
DS_RENDER_EXPORT void dsGfxBuffer_process(dsGfxBuffer* buffer);

/**
 * @brief Destroys a graphics buffer.
 * @remark errno will be set on failure.
 * @param buffer The buffer to destroy.
 * @return False if the buffer couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsGfxBuffer_destroy(dsGfxBuffer* buffer);

#ifdef __cplusplus
}
#endif
