/*
 * Copyright 2019 Aaron Barany
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

#include "Resources/MTLFramebuffer.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

dsFramebuffer* dsMTLFramebuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	const char* name, const dsFramebufferSurface* surfaces, uint32_t surfaceCount, uint32_t width,
	uint32_t height, uint32_t layers)
{
	DS_ASSERT(resourceManager);

	size_t nameLen = strlen(name) + 1;
	size_t bufferSize = DS_ALIGNED_SIZE(sizeof(dsFramebuffer)) +
		DS_ALIGNED_SIZE(sizeof(dsFramebufferSurface)*surfaceCount) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, bufferSize));

	dsFramebuffer* framebuffer = DS_ALLOCATE_OBJECT(&bufferAlloc, dsFramebuffer);
	DS_ASSERT(framebuffer);
	framebuffer->resourceManager = resourceManager;
	framebuffer->allocator = dsAllocator_keepPointer(allocator);
	framebuffer->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(framebuffer->name);
	memcpy((void*)framebuffer->name, name, nameLen);

	if (surfaceCount)
	{
		framebuffer->surfaces = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsFramebufferSurface,
			surfaceCount);
		DS_ASSERT(framebuffer->surfaces);
		memcpy(framebuffer->surfaces, surfaces, sizeof(dsFramebufferSurface)*surfaceCount);
	}
	else
		framebuffer->surfaces = NULL;
	framebuffer->surfaceCount = surfaceCount;
	framebuffer->width = width;
	framebuffer->height = height;
	framebuffer->layers = layers;
	return framebuffer;
}

bool dsMTLFramebuffer_destroy(dsResourceManager* resourceManager, dsFramebuffer* framebuffer)
{
	DS_UNUSED(resourceManager);
	if (framebuffer->allocator)
		DS_VERIFY(dsAllocator_free(framebuffer->allocator, framebuffer));
	return true;
}
