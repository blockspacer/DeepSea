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

#include "Resources/VkShaderModule.h"
#include "VkShared.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

dsShaderModule* dsVkShaderModule_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	mslModule* module, const char* name)
{
	dsShaderModule* shaderModule = DS_ALLOCATE_OBJECT(allocator, dsShaderModule);
	if (!shaderModule)
		return NULL;

	shaderModule->resourceManager = resourceManager;
	shaderModule->allocator = dsAllocator_keepPointer(allocator);
	shaderModule->module = module;
	shaderModule->name = name;

	return shaderModule;
}

bool dsVkShaderModule_destroy(dsResourceManager* resourceManager, dsShaderModule* module)
{
	DS_UNUSED(resourceManager);
	if (module->allocator)
		DS_VERIFY(dsAllocator_free(module->allocator, module));
	return true;
}
