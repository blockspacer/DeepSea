/*
 * Copyright 2018-2019 Aaron Barany
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

dsFramebuffer* dsVkFramebuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	const char* name, const dsFramebufferSurface* surfaces, uint32_t surfaceCount, uint32_t width,
	uint32_t height, uint32_t layers);
bool dsVkFramebuffer_destroy(dsResourceManager* resourceManager, dsFramebuffer* framebuffer);

dsVkRealFramebuffer* dsVkFramebuffer_getRealFramebuffer(dsFramebuffer* framebuffer,
	dsCommandBuffer* commandBuffer, const dsVkRenderPassData* renderPassData);
void dsVkFramebuffer_removeRenderPass(dsFramebuffer* framebuffer,
	const dsVkRenderPassData* renderPass);
