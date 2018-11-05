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

#include "VkResourceList.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

void dsVkResourceList_initialize(dsVkResourceList* resources, dsAllocator* allocator)
{
	memset(resources, 0, sizeof(*resources));
	DS_ASSERT(allocator->freeFunc);
	resources->allocator = allocator;
}

bool dsVkResourceList_addBuffer(dsVkResourceList* resources, dsVkGfxBufferData* buffer)
{
	uint32_t index = resources->bufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->buffers, resources->bufferCount,
		resources->maxBuffers, 1))
	{
		return false;
	}

	resources->buffers[index] = buffer;
	return true;
}

bool dsVkResourceList_addTexture(dsVkResourceList* resources, dsTexture* texture)
{
	uint32_t index = resources->textureCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->textures, resources->textureCount,
		resources->maxTextures, 1))
	{
		return false;
	}

	resources->textures[index] = texture;
	return true;
}

bool dsVkResourceList_addCopyImage(dsVkResourceList* resources, dsVkCopyImage* image)
{
	uint32_t index = resources->copyImageCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(resources->allocator, resources->copyImages,
		resources->copyImageCount, resources->maxCopyImages, 1))
	{
		return false;
	}

	resources->copyImages[index] = image;
	return true;
}

void dsVkResourceList_clear(dsVkResourceList* resources)
{
	resources->bufferCount = 0;
	resources->textureCount = 0;
	resources->copyImageCount = 0;
}

void dsVkResourceList_shutdown(dsVkResourceList* resources)
{
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->buffers));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->textures));
	DS_VERIFY(dsAllocator_free(resources->allocator, resources->copyImages));
}
