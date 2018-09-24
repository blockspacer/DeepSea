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

#include "Types.h"
#include <DeepSea/Core/Debug.h>

#if DS_DEBUG
#define DS_VK_CALL(func) (dsSetLastVkCallsite(__FILE__, __FUNCTION__,__LINE__), (func))
#else
#define DS_VK_CALL(func) (func)
#endif

bool dsHandleVkResult(VkResult result);
void dsSetLastVkCallsite(const char* file, const char* function, unsigned int line);
void dsGetLastVkCallsite(const char** file, const char** function,
	unsigned int* line);

VkDeviceMemory dsAllocateVkMemory(const dsVkDevice* device,
	const VkMemoryRequirements* requirements, dsGfxMemory memoryFlags);
