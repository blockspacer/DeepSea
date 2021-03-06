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

#include <DeepSea/Scene/SceneRenderPass.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Scene/ItemLists/SceneItemList.h>
#include <string.h>

static void destroyObjects(dsRenderPass* renderPass, const dsSceneItemLists* subpassDrawLists,
	uint32_t subpassDrawListCount)
{
	dsRenderPass_destroy(renderPass);
	if (!subpassDrawLists)
		return;

	for (uint32_t i = 0; i < subpassDrawListCount; ++i)
	{
		const dsSceneItemLists* drawLists = subpassDrawLists + i;
		if (!drawLists->itemLists)
			continue;

		for (uint32_t j = 0; j < drawLists->count; ++j)
			dsSceneItemList_destroy(drawLists->itemLists[i]);
	}
}

size_t dsSceneRenderPass_fullAllocSize(const char* framebuffer, uint32_t clearValueCount,
	const dsSceneItemLists* subpassDrawLists, uint32_t subpassDrawListCount)
{
	if (!framebuffer || !subpassDrawLists)
		return 0;

	size_t fullSize = DS_ALIGNED_SIZE(strlen(framebuffer) + 1) +
		DS_ALIGNED_SIZE(sizeof(dsSurfaceClearValue)*clearValueCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneRenderPass)) +
		DS_ALIGNED_SIZE(sizeof(dsSceneItemLists)*subpassDrawListCount);
	for (uint32_t i = 0; i < subpassDrawListCount; ++i)
	{
		const dsSceneItemLists* drawLists = subpassDrawLists + i;
		if (drawLists->count > 0 && !drawLists->itemLists)
			return 0;

		fullSize += DS_ALIGNED_SIZE(sizeof(dsSceneItemList*)*drawLists->count);
		for (uint32_t j = 0; j < drawLists->count; ++j)
		{
			if (!drawLists->itemLists[i])
				return 0;
		}
	}

	return fullSize;
}

dsSceneRenderPass* dsSceneRenderPass_create(dsAllocator* allocator, dsRenderPass* renderPass,
	const char* framebuffer, const dsSurfaceClearValue* clearValues, uint32_t clearValueCount,
	const dsSceneItemLists* subpassDrawLists, uint32_t subpassDrawListCount)
{
	if (!allocator || !renderPass || !framebuffer || !subpassDrawLists ||
		(!clearValues && clearValueCount > 0) ||
		(clearValueCount != 0 && clearValueCount != renderPass->attachmentCount) ||
		subpassDrawListCount != renderPass->subpassCount)
	{
		errno = EINVAL;
		destroyObjects(renderPass, subpassDrawLists, subpassDrawListCount);
		return NULL;
	}

	size_t fullSize = dsSceneRenderPass_fullAllocSize(framebuffer, clearValueCount,
		subpassDrawLists, subpassDrawListCount);
	if (fullSize == 0)
	{
		errno = EINVAL;
		destroyObjects(renderPass, subpassDrawLists, subpassDrawListCount);
		return NULL;
	}

	bool needsClear = false;
	for (uint32_t i = 0; i < renderPass->attachmentCount; ++i)
	{
		if (renderPass->attachments[i].usage & dsAttachmentUsage_Clear)
		{
			needsClear = true;
			break;
		}
	}

	if (needsClear && clearValueCount == 0)
	{
		errno = EINVAL;
		destroyObjects(renderPass, subpassDrawLists, subpassDrawListCount);
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Scene render pass requires clear values, but none were provided.");
		return NULL;
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		errno = ENOMEM;
		destroyObjects(renderPass, subpassDrawLists, subpassDrawListCount);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneRenderPass* sceneRenderPass = DS_ALLOCATE_OBJECT(&bufferAlloc,
		dsSceneRenderPass);
	DS_ASSERT(sceneRenderPass);

	sceneRenderPass->allocator = dsAllocator_keepPointer(allocator);
	sceneRenderPass->renderPass = renderPass;
	sceneRenderPass->drawLists = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemLists,
		subpassDrawListCount);
	DS_ASSERT(sceneRenderPass);

	size_t framebufferLen = strlen(framebuffer) + 1;
	sceneRenderPass->framebuffer = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, framebufferLen);
	DS_ASSERT(sceneRenderPass->framebuffer);
	memcpy((void*)sceneRenderPass->framebuffer, framebuffer, framebufferLen);

	if (clearValueCount > 0)
	{
		sceneRenderPass->clearValues = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSurfaceClearValue,
			clearValueCount);
		DS_ASSERT(sceneRenderPass->clearValues);
		memcpy((void*)sceneRenderPass->clearValues, clearValues,
			sizeof(dsSurfaceClearValue)*clearValueCount);
	}

	for (uint32_t i = 0; i < subpassDrawListCount; ++i)
	{
		const dsSceneItemLists* srcDrawLists = subpassDrawLists + i;
		dsSceneItemLists* dstDrawLists = sceneRenderPass->drawLists + i;
		if (srcDrawLists->count == 0)
		{
			dstDrawLists->itemLists = NULL;
			dstDrawLists->count = 0;
			continue;
		}

		dstDrawLists->itemLists = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemList*,
			srcDrawLists->count);
		DS_ASSERT(dstDrawLists->itemLists);
		memcpy(dstDrawLists->itemLists, srcDrawLists->itemLists,
			sizeof(dsSceneItemList*)*srcDrawLists->count);
		dstDrawLists->count = srcDrawLists->count;
	}

	return sceneRenderPass;
}

void dsSceneRenderPass_destroy(dsSceneRenderPass* renderPass)
{
	if (!renderPass)
		return;

	destroyObjects(renderPass->renderPass, renderPass->drawLists,
		renderPass->renderPass->subpassCount);
	if (renderPass->allocator)
		DS_VERIFY(dsAllocator_free(renderPass->allocator, renderPass));
}
