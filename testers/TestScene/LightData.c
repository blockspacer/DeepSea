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

#include "LightData.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Scene/SceneGlobalData.h>
#include <string.h>

static dsShaderVariableElement elements[] =
{
	{"direction", dsMaterialType_Vec3, 0},
	{"color", dsMaterialType_Vec3, 0},
	{"ambient", dsMaterialType_Vec3, 0}
};

typedef struct dsLightData
{
	dsSceneGlobalData globalData;
	dsShaderVariableGroup* variableGroup;
	uint32_t nameID;
} dsLightData;

dsShaderVariableGroupDesc* dsLightData_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (!resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsShaderVariableGroupDesc_create(resourceManager, allocator, elements,
		DS_ARRAY_SIZE(elements));
}

bool dsLightData_populateData(dsSceneGlobalData* globalData, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsLightData* lightData = (dsLightData*)globalData;
	if (!dsShaderVariableGroup_commit(lightData->variableGroup, commandBuffer))
		return false;

	DS_VERIFY(dsSharedMaterialValues_setVariableGroupID(view->globalValues, lightData->nameID,
		lightData->variableGroup));
	return true;
}

bool dsLightData_destroy(dsSceneGlobalData* globalData)
{
	dsLightData* lightData = (dsLightData*)globalData;
	if (!dsShaderVariableGroup_destroy(lightData->variableGroup))
		return false;

	if (globalData->allocator)
		DS_VERIFY(dsAllocator_free(globalData->allocator, globalData));
	return true;
}

dsSceneGlobalData* dsLightData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* lightDesc)
{
	DS_ASSERT(allocator);
	DS_ASSERT(resourceManager);
	DS_ASSERT(lightDesc);

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsLightData)) +
		dsShaderVariableGroup_fullAllocSize(resourceManager, lightDesc);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsLightData* lightData = DS_ALLOCATE_OBJECT(&bufferAlloc, dsLightData);
	DS_VERIFY(lightData);
	dsSceneGlobalData* globalData = (dsSceneGlobalData*)lightData;

	globalData->allocator = dsAllocator_keepPointer(allocator);
	globalData->valueCount = 1;
	globalData->populateDataFunc = &dsLightData_populateData;
	globalData->finishFunc = NULL;
	globalData->destroyFunc = &dsLightData_destroy;

	lightData->variableGroup = dsShaderVariableGroup_create(resourceManager,
		(dsAllocator*)&bufferAlloc, allocator, lightDesc);
	if (!lightData->variableGroup)
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, buffer));
		return NULL;
	}

	const char* name = "Light";
	lightData->nameID = dsHashString(name);

	return globalData;
}

void dsLightData_setDirection(dsSceneGlobalData* globalData, const dsVector3f* direction)
{
	DS_VERIFY(globalData);
	DS_VERIFY(direction);
	dsLightData* lightData = (dsLightData*)globalData;
	dsVector3f normDir;
	dsVector3f_normalize(&normDir, direction);
	DS_VERIFY(dsShaderVariableGroup_setElementData(lightData->variableGroup, 0, &normDir,
		dsMaterialType_Vec3, 0, 1));
}

void dsLightData_setColor(dsSceneGlobalData* globalData, const dsVector3f* color)
{
	DS_VERIFY(globalData);
	DS_VERIFY(color);
	dsLightData* lightData = (dsLightData*)globalData;
	DS_VERIFY(dsShaderVariableGroup_setElementData(lightData->variableGroup, 1, color,
		dsMaterialType_Vec3, 0, 1));
}

void dsLightData_setAmbientColor(dsSceneGlobalData* globalData, const dsVector3f* color)
{
	DS_VERIFY(globalData);
	DS_VERIFY(color);
	dsLightData* lightData = (dsLightData*)globalData;
	DS_VERIFY(dsShaderVariableGroup_setElementData(lightData->variableGroup, 2, color,
		dsMaterialType_Vec3, 0, 1));
}
