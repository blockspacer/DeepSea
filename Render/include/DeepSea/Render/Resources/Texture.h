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
 * @brief Functions for workingw ith textures.
 */

/**
 * @brief Helper macro for getting the depth for mipmap purposes.
 * @param dimension The dimension of the texture.
 * @param depth The depth of the texture.
 * @return The depth used for calculating the mipmap level.
 */
#define DS_MIP_DEPTH(dimension, depth) ((dimension) == dsTextureDim_3D ? (depth) : 1)

/**
 * @brief Constant for an invalid offset.
 */
#define DS_INVALID_TEXTURE_OFFSET (size_t)-1

/**
 * @brief Gets the maximum number of mipmap levels for a texture.
 * @param width The width of the texture.
 * @param height The height of the texture.
 * @param depth The depth of a 3D texture. Set to 1 for a non-3D texture, including texture arrays.
 * @return The maximum number of mipmap levels.
 */
DS_RENDER_EXPORT uint32_t dsTexture_maxMipmapLevels(uint32_t width, uint32_t height,
	uint32_t depth);

/**
 * @brief Gets the size of a texture in bytes.
 * @param format The format of the texture.
 * @param dimension The dimension of the texture.
 * @param width The width of the texture.
 * @param height The height of the texture.
 * @param depth The depth of the texture (for 3D textures) or number of array elements. Use 0 for
 *     non-array textures.
 * @param mipLevels The number of mip-map levels. Use DS_ALL_MIP_LEVELS to use the maximum number of
 *     mip levels.
 * @param samples The number of samples to use for multisampling. Use 1 if not multisampling.
 * @return The size of the texture in bytes, or 0 if invalid.
 */
DS_RENDER_EXPORT size_t dsTexture_size(dsGfxFormat format, dsTextureDim dimension, uint32_t width,
	uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t samples);

/**
 * @brief Gets the offset for a surface within a texture.
 * @param format The format of the texture.
 * @param dimension The dimension of the texture.
 * @param width The width of the texture.
 * @param height The height of the texture.
 * @param depth The depth of the texture (for 3D textures) or number of array elements. Use 0 for
 *     non-array textures.
 * @param mipLevels The number of mip-map levels. Use DS_ALL_MIP_LEVELS to use the maximum number of
 *     mip levels.
 * @param cubeFace The cube map face when accessing a cube map.
 * @param depthIndex The index of the depth level or array element.
 * @param mipIndex The mipmap index.
 * @return The offset to the surface, or DS_INVALID_TEXTURE_OFFSET if the surface is out of range.
 */
DS_RENDER_EXPORT size_t dsTexture_surfaceOffset(dsGfxFormat format, dsTextureDim dimension,
	uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, dsCubeFace cubeFace,
	uint32_t depthIndex, uint32_t mipIndex);

/**
 * @brief Gets the offset for a layer within a texture.
 *
 * This is very similar to dsTexture_surfaceOffset(), except the cube face and depth index are
 * combined into a single layer index.
 *
 * @param format The format of the texture.
 * @param dimension The dimension of the texture.
 * @param width The width of the texture.
 * @param height The height of the texture.
 * @param depth The depth of the texture (for 3D textures) or number of array elements. Use 0 for
 *     non-array textures.
 * @param mipLevels The number of mip-map levels. Use DS_ALL_MIP_LEVELS to use the maximum number of
 *     mip levels.
 * @param layerIndex The index of the layer, which is the same as depth*faceCount + face.
 * @param mipIndex The mipmap index.
 * @return The offset to the layer, or DS_INVALID_TEXTURE_OFFSET if the layer is out of range.
 */
DS_RENDER_EXPORT size_t dsTexture_layerOffset(dsGfxFormat format, dsTextureDim dimension,
	uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t layerIndex,
	uint32_t mipIndex);

/**
 * @brief Creates a texture.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the texture from.
 * @param allocator The allocator to create the texture with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param usage How the texture will be used. This should be a combination of dsTextureUsage flags.
 * @param memoryHints Hints for how the memory for the texture will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @param format The format of the texture.
 * @param dimension The dimension of the texture.
 * @param width The width of the texture.
 * @param height The height of the texture.
 * @param depth The depth of the texture (for 3D textures) or number of array elements. Use 0 for
 *     non-array textures.
 * @param mipLevels The number of mip-map levels. Use DS_ALL_MIP_LEVELS to use the maximum number of
 *     mip levels.
 * @param data The initial data for the texture, or NULL to leave uninitialized. The order of the
 *     data is:
 *     - Mip levels.
 *     - Array elements/depth slices.
 *     - Cube faces in the order of dsCubeFace.
 *     - Texture rows.
 *     Data is tightly packed.
 * @param size The size of the data. This must match the size of the texture, and is used to ensure
 *     protect against incorrectly calculated buffer sizes.
 * @return The created texture, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsTexture* dsTexture_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, unsigned int usage, unsigned int memoryHints, dsGfxFormat format,
	dsTextureDim dimension, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels,
	const void* data, size_t size);

/**
 * @brief Creates an offscreen texture.
 *
 * The offscreen may be rendered to, and may also be used interchangibely with a dsTexture.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the offscreen from.
 * @param allocator The allocator to create the offscreen with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param usage How the offscreen will be used. This should be a combination of dsTextureUsage
 *     flags.
 * @param memoryHints Hints for how the memory for the offscreen will be used. This should be a
 *     combination of dsGfxMemory flags.
 * @param format The format of the offscreen.
 * @param dimension The dimension of the offscreen.
 * @param width The width of the offscreen.
 * @param height The height of the offscreen.
 * @param depth The depth of the texture (for 3D textures) or number of array elements. Use 0 for
 *     non-array textures.
 * @param mipLevels The number of mip-map levels. Use DS_ALL_MIP_LEVELS to use the maximum number of
 *     mip levels.
 * @param samples The number of samples to use for multisampling. This may be set to
 *     DS_DEFAULT_ANTIALIAS_SAMPLES to use the default set on the renderer. The renderbuffer will
 *     need to be re-created by the caller if the default changes. When multisampling isn't
 *     supported, this will silently fallback to no multisampling for resolved surfaces or fail for
 *     non-resolved surfaces.
 * @param resolve True to resolve multisampled offscreens, false to leave unresolved to sample in
 *     the shader.
 * @return The created offscreen, or NULL if it couldn't be created.
 */
DS_RENDER_EXPORT dsOffscreen* dsTexture_createOffscreen(dsResourceManager* resourceManager,
	dsAllocator* allocator, unsigned int usage, unsigned int memoryHints, dsGfxFormat format,
	dsTextureDim dimension, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels,
	uint32_t samples, bool resolve);

/**
 * @brief Copies data to a texture.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @remark errno will be set on failure.
 * @param texture The texture to copy the data to.
 * @param commandBuffer The command buffer to process the copy on.
 * @param position The position of the texture to copy to.
 * @param width The width of the texture data. This must be a multiple of the format block size or
 *     reach the end of the image.
 * @param height The height of the texture data. This must be a multiple of the format block size or
 *     reach the end of the image.
 * @param layers The number of layers to copy, which is the depth multiplied by the number of faces.
 *     This must be at least 1.
 * @param data The texture data to copy. This must be tightly packed.
 * @param size The size of the data. This is used to help catch mismatched data.
 * @return False if the data couldn't be copied.
 */
DS_RENDER_EXPORT bool dsTexture_copyData(dsTexture* texture, dsCommandBuffer* commandBuffer,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size);

/**
 * @brief Copies data from one texture to another.
 *
 * This queues the copy on a command buffer, so the thread that processes this doesn't need a
 * resource context.
 *
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to process the copy on.
 * @param srcTexture The texture to copy from.
 * @param dstTexture The texture to copy to.
 * @param regions The regions to copy.
 * @param regionCount The number of regions to copy.
 * @return False if the data couldn't be copied.
 */
DS_RENDER_EXPORT bool dsTexture_copy(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureCopyRegion* regions, size_t regionCount);

/**
 * @brief Generates mipmaps for a texture based on the first mip level.
 *
 * This will only populate data for already allocated mipmaps. Most commonly, this will be used to
 * generate mipmaps for an offscreen after rendering to the first mip level.
 *
 * @remark errno will be set on failure.
 * @param texture The texture to generate mipmaps for.
 * @param commandBuffer The command buffer to process the generation on.
 */
DS_RENDER_EXPORT bool dsTexture_generateMipmaps(dsTexture* texture, dsCommandBuffer* commandBuffer);

/**
 * @brief Grabs texture data.
 *
 * This is generally used to read data back from an offscreen to the CPU.
 *
 * @remark When reading from an offscreen, note that this may block until the GPU is finished
 *     executing drawing commands.
 * @remark errno will be set on failure.
 *
 * @param[out] result The output buffer for the result.
 * @param size The size of the result buffer. This must match the size that will be read from the
 *     texture based on the requested size.
 * @param texture The texture to read from.
 * @param position The position to read from.
 * @param width The width of the texture region.
 * @param height The height of the texture region.
 * @return False if the data couldn't be read.
 */
DS_RENDER_EXPORT bool dsTexture_getData(void* result, size_t size, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height);

/**
 * @brief Destroys a texture or offscreen.
 * @remark errno will be set on failure.
 * @param texture The texture to destroy.
 * @return False if the texture couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsTexture_destroy(dsTexture* texture);

#ifdef __cplusplus
}
#endif
