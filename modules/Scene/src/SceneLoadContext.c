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

#include <DeepSea/Scene/SceneLoadContext.h>

#include "SceneLoadContextInternal.h"
#include "SceneTypes.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/ItemLists/InstanceTransformData.h>
#include <DeepSea/Scene/ItemLists/SceneModelList.h>
#include <DeepSea/Scene/Nodes/SceneModelNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTransformNode.h>
#include <DeepSea/Scene/ViewTransformData.h>

#include <string.h>

size_t dsSceneLoadContext_sizeof(void)
{
	return sizeof(dsSceneLoadContext);
}

size_t dsSceneLoadContext_fullAllocSize(void)
{
	return DS_ALIGNED_SIZE(sizeof(dsSceneLoadContext));
}

dsSceneLoadContext* dsSceneLoadContext_create(dsAllocator* allocator, dsRenderer* renderer)
{
	if (!allocator || !renderer)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneLoadContext* context = DS_ALLOCATE_OBJECT(allocator, dsSceneLoadContext);
	if (!context)
		return NULL;

	context->allocator = dsAllocator_keepPointer(allocator);
	context->renderer = renderer;
	dsHashTable_initialize(&context->nodeTypeTable.hashTable, DS_SCENE_TYPE_TABLE_SIZE,
		dsHashString, dsHashStringEqual);
	dsHashTable_initialize(&context->itemListTypeTable.hashTable, DS_SCENE_TYPE_TABLE_SIZE,
		dsHashString, dsHashStringEqual);
	dsHashTable_initialize(&context->globalDataTypeTable.hashTable, DS_SCENE_TYPE_TABLE_SIZE,
		dsHashString, dsHashStringEqual);

	// Built-in types.
	dsSceneLoadContext_registerNodeType(context, dsSceneNodeRef_typeName, &dsSceneNodeRef_load,
		NULL, NULL);
	dsSceneLoadContext_registerNodeType(context, dsSceneModelNode_typeName, &dsSceneModelNode_load,
		NULL, NULL);
	dsSceneLoadContext_registerNodeType(context, dsSceneTransformNode_typeName,
		&dsSceneTransformNode_load, NULL, NULL);

	dsSceneLoadContext_registerItemListType(context, dsSceneModelList_typeName,
		&dsSceneModelList_load, NULL, NULL);

	dsSceneLoadContext_registerInstanceDataType(context, dsInstanceTransformData_typeName,
		&dsInstanceTransformData_load, NULL, NULL);

	dsSceneLoadContext_registerGlobalDataType(context, dsViewTransformData_typeName,
		&dsViewTransformData_load, NULL, NULL);

	return context;
}

dsRenderer* dsSceneLoadContext_getRenderer(const dsSceneLoadContext* context)
{
	if (!context)
	{
		errno = EINVAL;
		return NULL;
	}

	return context->renderer;
}

bool dsSceneLoadContext_registerNodeType(dsSceneLoadContext* context, const char* name,
	dsLoadSceneNodeFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (!context || !name || !loadFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsHashTable* hashTable = &context->nodeTypeTable.hashTable;
	size_t index = hashTable->list.length;
	if (index >= DS_MAX_SCENE_TYPES)
	{
		errno = ENOMEM;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Node type name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_NAME_LENGTH);
		return false;
	}

	dsLoadSceneNodeItem* nodeType = context->nodeTypes + index;
	memcpy(nodeType->name, name, nameLength + 1);
	nodeType->loadFunc = loadFunc;
	nodeType->userData = userData;
	nodeType->destroyUserDataFunc = destroyUserDataFunc;
	if (!dsHashTable_insert(hashTable, nodeType->name, (dsHashTableNode*)nodeType, NULL))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Node type '%s' has already been registered.", name);
		return false;
	}
	return true;
}

bool dsSceneLoadContext_registerItemListType(dsSceneLoadContext* context, const char* name,
	dsLoadSceneItemListFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (!context || !name || !loadFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsHashTable* hashTable = &context->itemListTypeTable.hashTable;
	size_t index = hashTable->list.length;
	if (index >= DS_MAX_SCENE_TYPES)
	{
		errno = ENOMEM;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Item list type name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_NAME_LENGTH);
		return false;
	}

	dsLoadSceneItemListItem* itemListType = context->itemListTypes + index;
	memcpy(itemListType->name, name, nameLength + 1);
	itemListType->loadFunc = loadFunc;
	itemListType->userData = userData;
	itemListType->destroyUserDataFunc = destroyUserDataFunc;
	if (!dsHashTable_insert(hashTable, itemListType->name, (dsHashTableNode*)itemListType, NULL))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Item list type '%s' has already been registered.", name);
		return false;
	}
	return true;
}

bool dsSceneLoadContext_registerInstanceDataType(dsSceneLoadContext* context, const char* name,
	dsLoadSceneInstanceDataFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (!context || !name || !loadFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsHashTable* hashTable = &context->instanceDataTypeTable.hashTable;
	size_t index = hashTable->list.length;
	if (index >= DS_MAX_SCENE_TYPES)
	{
		errno = ENOMEM;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Instance data type name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_NAME_LENGTH);
		return false;
	}

	dsLoadSceneInstanceDataItem* instanceDataType = context->instanceDataTypes + index;
	memcpy(instanceDataType->name, name, nameLength + 1);
	instanceDataType->loadFunc = loadFunc;
	instanceDataType->userData = userData;
	instanceDataType->destroyUserDataFunc = destroyUserDataFunc;
	if (!dsHashTable_insert(hashTable, instanceDataType->name, (dsHashTableNode*)instanceDataType,
			NULL))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Instance data type '%s' has already been registered.",
			name);
		return false;
	}
	return true;
}

bool dsSceneLoadContext_registerGlobalDataType(dsSceneLoadContext* context, const char* name,
	dsLoadSceneGlobalDataFunction loadFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (!context || !name || !loadFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsHashTable* hashTable = &context->globalDataTypeTable.hashTable;
	size_t index = hashTable->list.length;
	if (index >= DS_MAX_SCENE_TYPES)
	{
		errno = ENOMEM;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Global data type name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_NAME_LENGTH);
		return false;
	}

	dsLoadSceneGlobalDataItem* globalDataType = context->globalDataTypes + index;
	memcpy(globalDataType->name, name, nameLength + 1);
	globalDataType->loadFunc = loadFunc;
	globalDataType->userData = userData;
	globalDataType->destroyUserDataFunc = destroyUserDataFunc;
	if (!dsHashTable_insert(hashTable, globalDataType->name, (dsHashTableNode*)globalDataType,
			NULL))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Global data type '%s' has already been registered.",
			name);
		return false;
	}
	return true;
}

void dsSceneLoadContext_destroy(dsSceneLoadContext* context)
{
	if (!context)
		return;

	dsHashTable* hashTable = &context->nodeTypeTable.hashTable;
	for (dsListNode* node = hashTable->list.head; node; node = node->next)
	{
		dsLoadSceneNodeItem* nodeType = (dsLoadSceneNodeItem*)node;
		if (nodeType->destroyUserDataFunc)
			nodeType->destroyUserDataFunc(nodeType->userData);
	}

	hashTable = &context->itemListTypeTable.hashTable;
	for (dsListNode* node = hashTable->list.head; node; node = node->next)
	{
		dsLoadSceneItemListItem* itemListType = (dsLoadSceneItemListItem*)node;
		if (itemListType->destroyUserDataFunc)
			itemListType->destroyUserDataFunc(itemListType->userData);
	}

	hashTable = &context->instanceDataTypeTable.hashTable;
	for (dsListNode* node = hashTable->list.head; node; node = node->next)
	{
		dsLoadSceneInstanceDataItem* instanceDataType = (dsLoadSceneInstanceDataItem*)node;
		if (instanceDataType->destroyUserDataFunc)
			instanceDataType->destroyUserDataFunc(instanceDataType->userData);
	}

	hashTable = &context->globalDataTypeTable.hashTable;
	for (dsListNode* node = hashTable->list.head; node; node = node->next)
	{
		dsLoadSceneGlobalDataItem* globalDataType = (dsLoadSceneGlobalDataItem*)node;
		if (globalDataType->destroyUserDataFunc)
			globalDataType->destroyUserDataFunc(globalDataType->userData);
	}

	if (context->allocator)
		DS_VERIFY(dsAllocator_free(context->allocator, context));
}
