/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/Render/Resources/Types.h>

dsGfxBuffer* dsGLGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	int usage, int memoryHints, const void* data, size_t size);
void* dsGLGfxBuffer_map(dsResourceManager* resourceManager, dsGfxBuffer* buffer, int flags,
	size_t offset, size_t size);
bool dsGLGfxBuffer_unmap(dsResourceManager* resourceManager, dsGfxBuffer* buffer);
bool dsGLGfxBuffer_flush(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size);
bool dsGLGfxBuffer_invalidate(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size);
bool dsGLGfxBuffer_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* buffer, size_t offset, const void* data, size_t size);
bool dsGLGfxBuffer_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* srcBuffer, size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset,
	size_t size);
bool dsGLGfxBuffer_destroy(dsResourceManager* resourceManager, dsGfxBuffer* buffer);
