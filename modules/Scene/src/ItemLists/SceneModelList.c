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

#include <DeepSea/Scene/ItemLists/SceneModelList.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <DeepSea/Scene/Nodes/SceneModelNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneNodeItemData.h>

#include <stdlib.h>
#include <string.h>

typedef struct Entry
{
	const dsSceneModelNode* node;
	const dsMatrix44f* transform;
	dsSceneNodeItemData* itemData;
	uint64_t nodeID;
} Entry;

typedef struct DrawItem
{
	dsShader* shader;
	dsMaterial* material;
	uint32_t instance;
	float flatDistance;

	dsDrawGeometry* geometry;
	union
	{
		dsDrawRange drawRange;
		dsDrawIndexedRange drawIndexedRange;
	};
	dsPrimitiveType primitiveType;
} DrawItem;

struct dsSceneModelList
{
	dsSceneItemList itemList;

	dsDynamicRenderStates renderStates;
	bool hasRenderStates;
	dsModelSortType sortType;
	uint32_t cullNameID;

	dsSharedMaterialValues* instanceValues;
	dsSceneInstanceData** instanceData;
	uint32_t instanceDataCount;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;

	dsSceneInstanceInfo* instances;
	DrawItem* drawItems;
	uint32_t maxInstances;
	uint32_t maxDrawItems;
};

static void addInstances(dsSceneItemList* itemList, const dsView* view, uint32_t* instanceCount,
	uint32_t* drawItemCount)
{
	DS_PROFILE_FUNC_START();

	dsSceneModelList* modelList = (dsSceneModelList*)itemList;

	for (uint32_t i = 0; i < modelList->entryCount; ++i)
	{
		const Entry* entry = modelList->entries + i;
		const dsSceneModelNode* modelNode = entry->node;
		// Non-zero cull result means out of view.
		if (modelList->cullNameID &&
			dsSceneNodeItemData_findID(entry->itemData, modelList->cullNameID))
		{
			continue;
		}

		float distance2 = dsVector3_dist2(entry->transform->columns[3],
			view->cameraMatrix.columns[3]);

		dsVector3f direction;
		dsVector3_sub(direction, entry->transform->columns[3], view->cameraMatrix.columns[3]);
		float flatDistance = -dsVector3_dot(direction, view->cameraMatrix.columns[2]);

		bool hasAny = false;
		uint32_t instanceIndex = *instanceCount;
		for (uint32_t j = 0; j < modelNode->modelCount; ++j)
		{
			dsSceneModelInfo* model = modelNode->models + j;
			if (model->listNameID != itemList->nameID)
				continue;

			if (model->distanceRange.x <= model->distanceRange.y &&
				(distance2 < dsPow2(model->distanceRange.x) ||
					distance2 >= dsPow2(model->distanceRange.y)))
			{
				continue;
			}

			uint32_t itemIndex = *drawItemCount;
			if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, modelList->drawItems, *drawItemCount,
					modelList->maxDrawItems, 1))
			{
				continue;
			}

			hasAny = true;
			DrawItem* item = modelList->drawItems + itemIndex;
			item->shader = model->shader;
			item->material = model->material;
			item->flatDistance = flatDistance;
			item->instance = instanceIndex;

			item->geometry = model->geometry;
			if (item->geometry->indexBuffer.buffer)
				item->drawIndexedRange = model->drawIndexedRange;
			else
				item->drawRange = model->drawRange;
			item->primitiveType = model->primitiveType;
		}

		if (!hasAny)
			continue;

		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, modelList->instances, *instanceCount,
				modelList->maxInstances, 1))
		{
			continue;
		}

		dsSceneInstanceInfo* instance = modelList->instances + instanceIndex;
		instance->node = (dsSceneNode*)modelNode;
		instance->transform = *entry->transform;
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

static void setupInstances(dsSceneModelList* modelList, const dsView* view, uint32_t instanceCount)
{
	DS_PROFILE_FUNC_START();

	for (uint32_t i = 0; i < modelList->instanceDataCount; ++i)
	{
		dsSceneInstanceData_populateData(modelList->instanceData[i], view, modelList->instances,
			instanceCount);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

static int sortByMaterial(const void* left, const void* right)
{
	const DrawItem* leftInfo = (const DrawItem*)left;
	const DrawItem* rightInfo = (const DrawItem*)right;
	if (leftInfo->shader != rightInfo->shader)
		return leftInfo->shader < rightInfo->shader ? -1 : 1;
	if (leftInfo->material != rightInfo->material)
		return leftInfo->material < rightInfo->material ? -1 : 1;
	if (leftInfo->geometry != rightInfo->geometry)
		return leftInfo->geometry < rightInfo->geometry ? -1 : 1;
	return leftInfo->instance - rightInfo->instance;
}

static int sortBackToFront(const void* left, const void* right)
{
	const DrawItem* leftInfo = (const DrawItem*)left;
	const DrawItem* rightInfo = (const DrawItem*)right;
	if (leftInfo->flatDistance != rightInfo->flatDistance)
		return leftInfo->flatDistance > rightInfo->flatDistance ? -1 : 1;
	return sortByMaterial(left, right);
}

static int sortFrontToBack(const void* left, const void* right)
{
	const DrawItem* leftInfo = (const DrawItem*)left;
	const DrawItem* rightInfo = (const DrawItem*)right;
	if (leftInfo->flatDistance != rightInfo->flatDistance)
		return leftInfo->flatDistance < rightInfo->flatDistance ? -1 : 1;
	return sortByMaterial(left, right);
}

static void sortGeometry(dsSceneModelList* modelList, uint32_t drawItemCount)
{
	DS_PROFILE_FUNC_START();

	switch (modelList->sortType)
	{
		case dsModelSortType_Material:
			qsort(modelList->drawItems, drawItemCount, sizeof(DrawItem), &sortByMaterial);
			break;
		case dsModelSortType_BackToFront:
			qsort(modelList->drawItems, drawItemCount, sizeof(DrawItem), &sortBackToFront);
			break;
		case dsModelSortType_FrontToBack:
			qsort(modelList->drawItems, drawItemCount, sizeof(DrawItem), &sortFrontToBack);
			break;
		default:
			break;
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

static void drawGeometry(dsSceneModelList* modelList, uint32_t drawItemCount, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	dsRenderer* renderer = commandBuffer->renderer;
	dsShader* lastShader = NULL;
	dsMaterial* lastMaterial = NULL;
	uint32_t lastInstance = (uint32_t)-1;
	dsDynamicRenderStates* renderStates =
		modelList->hasRenderStates ? &modelList->renderStates : NULL;
	bool hasInstances = modelList->instanceDataCount > 0;
	for (uint32_t i = 0; i < drawItemCount; ++i)
	{
		const DrawItem* drawItem = modelList->drawItems + i;
		bool updateInstances = false;
		if (drawItem->shader != lastShader || drawItem->material != lastMaterial)
		{
			if (lastShader)
				dsShader_unbind(lastShader, commandBuffer);

			if (!DS_CHECK(DS_SCENE_LOG_TAG, dsShader_bind(drawItem->shader, commandBuffer,
					drawItem->material, view->globalValues, renderStates)))
			{
				continue;
			}

			lastShader = drawItem->shader;
			lastMaterial = drawItem->material;
			updateInstances = hasInstances;
		}

		if (drawItem->instance != lastInstance && hasInstances)
		{
			for (uint32_t j = 0; j < modelList->instanceDataCount; ++j)
			{
				DS_CHECK(DS_SCENE_LOG_TAG, dsSceneInstanceData_bindInstance(
					modelList->instanceData[j], drawItem->instance, modelList->instanceValues));
			}

			lastInstance = drawItem->instance;
			updateInstances = true;
		}

		if (updateInstances)
		{
			DS_CHECK(DS_SCENE_LOG_TAG, dsShader_updateInstanceValues(drawItem->shader,
				commandBuffer, modelList->instanceValues));
		}

		if (drawItem->geometry->indexBuffer.buffer)
		{
			DS_CHECK(DS_SCENE_LOG_TAG, dsRenderer_drawIndexed(renderer, commandBuffer,
				drawItem->geometry, &drawItem->drawIndexedRange, drawItem->primitiveType));
		}
		else
		{
			DS_CHECK(DS_SCENE_LOG_TAG, dsRenderer_draw(renderer, commandBuffer,
				drawItem->geometry, &drawItem->drawRange, drawItem->primitiveType));
		}
	}

	if (lastShader)
		dsShader_unbind(lastShader, commandBuffer);

	DS_PROFILE_FUNC_RETURN_VOID();
}

static void cleanup(dsSceneModelList* modelList)
{
	for (uint32_t i = 0; i < modelList->instanceDataCount; ++i)
		dsSceneInstanceData_finish(modelList->instanceData[i]);
}

static void destroyInstanceData(dsSceneInstanceData* const* instanceData,
	uint32_t instanceDataCount)
{
	for (uint32_t i = 0; i < instanceDataCount; ++i)
		dsSceneInstanceData_destroy(instanceData[i]);
}

const char* const dsSceneModelList_typeName = "ModelList";

uint64_t dsSceneModelList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	const dsMatrix44f* transform, dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(thisItemData);
	if (!dsSceneNode_isOfType(node, dsSceneModelNode_type()))
		return DS_NO_SCENE_NODE;

	dsSceneModelList* modelList = (dsSceneModelList*)itemList;

	uint32_t index = modelList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, modelList->entries, modelList->entryCount,
			modelList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = modelList->entries + index;
	entry->node = (dsSceneModelNode*)node;
	entry->transform = transform;
	entry->itemData = itemData;
	entry->nodeID = modelList->nextNodeID++;
	return entry->nodeID;
}

void dsSceneModelList_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsSceneModelList* modelList = (dsSceneModelList*)itemList;
	for (uint32_t i = 0; i < modelList->entryCount; ++i)
	{
		if (modelList->entries[i].nodeID != nodeID)
			continue;

		// Order shouldn't matter, so use constant-time removal.
		modelList->entries[i] = modelList->entries[modelList->entryCount - 1];
		--modelList->entryCount;
		break;
	}
}

void dsSceneModelList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);
	dsRenderer_pushDebugGroup(commandBuffer->renderer, commandBuffer, itemList->name);

	dsSceneModelList* modelList = (dsSceneModelList*)itemList;
	uint32_t instanceCount = 0;
	uint32_t drawItemCount = 0;
	addInstances(itemList, view, &instanceCount, &drawItemCount);
	setupInstances(modelList, view, instanceCount);
	sortGeometry(modelList, drawItemCount);
	drawGeometry(modelList, drawItemCount, view, commandBuffer);
	cleanup(modelList);

	dsRenderer_popDebugGroup(commandBuffer->renderer, commandBuffer);
	DS_PROFILE_SCOPE_END();
}

dsSceneModelList* dsSceneModelList_create(dsAllocator* allocator, const char* name,
	dsSceneInstanceData* const* instanceData, uint32_t instanceDataCount, dsModelSortType sortType,
	const dsDynamicRenderStates* renderStates, const char* cullName)
{
	if (!allocator || !name || (!instanceData && instanceDataCount > 0))
	{
		errno = EINVAL;
		if (instanceData)
			destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene model list allocator must support freeing memory.");
		destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	uint32_t valueCount = 0;
	for (uint32_t i = 0; i < instanceDataCount; ++i)
	{
		if (!instanceData[i])
		{
			errno = EINVAL;
			destroyInstanceData(instanceData, instanceDataCount);
			return NULL;
		}
		valueCount += instanceData[i]->valueCount;
	}

	size_t nameLen = strlen(name);
	size_t globalDataSize = instanceDataCount > 0 ?
		dsSharedMaterialValues_fullAllocSize(instanceDataCount) : 0;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneModelList)) + DS_ALIGNED_SIZE(nameLen + 1) +
		DS_ALIGNED_SIZE(sizeof(dsSceneInstanceData*)*instanceDataCount) + globalDataSize;
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneModelList* modelList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneModelList);
	DS_ASSERT(modelList);

	dsSceneItemList* itemList = (dsSceneItemList*)modelList;
	itemList->allocator = allocator;
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, char, nameLen + 1);
	memcpy((void*)itemList->name, name, nameLen + 1);
	itemList->nameID = dsHashString(name);
	itemList->addNodeFunc = &dsSceneModelList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsSceneModelList_removeNode;
	itemList->commitFunc = &dsSceneModelList_commit;
	itemList->destroyFunc = (dsDestroySceneItemListFunction)&dsSceneModelList_destroy;

	if (renderStates)
	{
		modelList->renderStates = *renderStates;
		modelList->hasRenderStates = true;
	}
	else
		modelList->hasRenderStates = false;
	modelList->sortType = sortType;
	modelList->cullNameID = cullName ? dsHashString(cullName) : 0;

	if (instanceDataCount > 0)
	{
		if (valueCount > 0)
		{
			modelList->instanceValues = dsSharedMaterialValues_create((dsAllocator*)&bufferAlloc,
				instanceDataCount);
			DS_ASSERT(modelList->instanceValues);
		}
		else
			modelList->instanceValues = NULL;
		modelList->instanceData = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneInstanceData*,
			instanceDataCount);
		DS_ASSERT(modelList->instanceData);
		memcpy(modelList->instanceData, instanceData,
			sizeof(dsSceneInstanceData*)*instanceDataCount);
	}
	else
	{
		modelList->instanceValues = NULL;
		modelList->instanceData = NULL;
	}
	modelList->instanceDataCount = instanceDataCount;

	modelList->entries = NULL;
	modelList->entryCount = 0;
	modelList->maxEntries = 0;
	modelList->nextNodeID = 0;
	modelList->instances = NULL;
	modelList->drawItems = NULL;
	modelList->maxInstances = 0;
	modelList->maxDrawItems = 0;
	return modelList;
}

dsModelSortType dsSceneModelList_getSortType(const dsSceneModelList* modelList)
{
	return modelList ? modelList->sortType : dsModelSortType_None;
}

void dsSceneModelList_setSortType(dsSceneModelList* modelList, dsModelSortType sortType)
{
	if (modelList)
		modelList->sortType = sortType;
}

const dsDynamicRenderStates* dsSceneModelList_getRenderStates(const dsSceneModelList* modelList)
{
	return modelList && modelList->hasRenderStates ? &modelList->renderStates : NULL;
}

void dsSceneModelList_setRenderStates(dsSceneModelList* modelList,
	const dsDynamicRenderStates* renderStates)
{
	if (!modelList)
		return;

	if (renderStates)
	{
		modelList->hasRenderStates = true;
		modelList->renderStates = *renderStates;
	}
	else
		modelList->hasRenderStates = false;
}

void dsSceneModelList_destroy(dsSceneModelList* modelList)
{
	if (!modelList)
		return;

	dsSceneItemList* itemList = (dsSceneItemList*)modelList;

	destroyInstanceData(modelList->instanceData, modelList->instanceDataCount);
	dsSharedMaterialValues_destroy(modelList->instanceValues);
	DS_VERIFY(dsAllocator_free(itemList->allocator, modelList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, modelList->instances));
	DS_VERIFY(dsAllocator_free(itemList->allocator, modelList->drawItems));
	DS_VERIFY(dsAllocator_free(itemList->allocator, modelList));
}
