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
#include "VkTypes.h"

dsGfxQueryPool* dsVkGfxQueryPool_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsGfxQueryType type, uint32_t count);
bool dsVkGfxQueryPool_reset(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t first, uint32_t count);
bool dsVkGfxQueryPool_beginQuery(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t query);
bool dsVkGfxQueryPool_endQuery(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t query);
bool dsVkGfxQueryPool_queryTimestamp(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries, uint32_t query);
bool dsVkGfxQueryPool_getValues(dsResourceManager* resourceManager, dsGfxQueryPool* queries,
	uint32_t first, uint32_t count, void* data, size_t dataSize, size_t stride, size_t elementSize,
	bool checkAvailability);
bool dsVkGfxQueryPool_copyValues(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxQueryPool* queries, uint32_t first, uint32_t count, dsGfxBuffer* buffer, size_t offset,
	size_t stride, size_t elementSize, bool checkAvailability);
bool dsVkGfxQueryPool_destroy(dsResourceManager* resourceManager, dsGfxQueryPool* queries);

void dsVkGfxQueryPool_destroyImpl(dsGfxQueryPool* queries);
