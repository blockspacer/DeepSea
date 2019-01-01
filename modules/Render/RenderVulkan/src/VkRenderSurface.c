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

#include "VkRenderSurface.h"

#include "Platform/VkPlatform.h"
#include "VkCommandBuffer.h"
#include "VkRenderSurfaceData.h"
#include "VkRendererInternal.h"
#include "VkShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>

dsRenderSurface* dsVkRenderSurface_create(dsRenderer* renderer, dsAllocator* allocator,
	const char* name, void* osHandle, dsRenderSurfaceType type)
{
	dsVkRenderer* vkRenderer = (dsVkRenderer*)renderer;
	VkSurfaceKHR surface;
	switch (type)
	{
		case dsRenderSurfaceType_Direct:
			surface = (VkSurfaceKHR)(uintptr_t)osHandle;
			break;
		default:
			surface = dsVkPlatform_createSurface(&vkRenderer->platform, osHandle);
			if (!surface)
				return NULL;
			break;
	}

	dsVkRenderSurface* renderSurface = DS_ALLOCATE_OBJECT(allocator, dsVkRenderSurface);
	if (!renderSurface)
	{
		if (type != dsRenderSurfaceType_Direct)
			dsVkPlatform_destroySurface(&vkRenderer->platform, surface);
		return NULL;
	}

	dsRenderSurface* baseRenderSurface = (dsRenderSurface*)renderSurface;
	baseRenderSurface->renderer = renderer;
	baseRenderSurface->allocator = dsAllocator_keepPointer(allocator);
	baseRenderSurface->name = name;
	baseRenderSurface->surfaceType = type;

	renderSurface->scratchAllocator = renderer->allocator;
	renderSurface->surface = surface;
	renderSurface->updatedFrame = renderer->frameNumber - 1;
	DS_VERIFY(dsSpinlock_initialize(&renderSurface->lock));

	renderSurface->surfaceData = dsVkRenderSurfaceData_create(renderSurface->scratchAllocator,
		renderer, surface, renderer->vsync, 0);
	if (!renderSurface->surfaceData)
	{
		dsVkRenderSurface_destroy(renderer, baseRenderSurface);
		return NULL;
	}

	baseRenderSurface->width = renderSurface->surfaceData->width;
	baseRenderSurface->height = renderSurface->surfaceData->height;

	return baseRenderSurface;
}

bool dsVkRenderSurface_update(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	dsVkRenderSurface* vkSurface = (dsVkRenderSurface*)renderSurface;
	DS_VERIFY(dsSpinlock_lock(&vkSurface->lock));

	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	dsVkInstance* instance = &device->instance;

	if (vkSurface->surfaceData && vkSurface->surfaceData->vsync == renderer->vsync)
	{
		VkSurfaceCapabilitiesKHR surfaceInfo;
		VkResult result = DS_VK_CALL(instance->vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(
			device->physicalDevice, vkSurface->surface, &surfaceInfo);
		if (result == VK_SUCCESS)
		{
			if (surfaceInfo.currentExtent.width == vkSurface->surfaceData->width &&
				surfaceInfo.currentExtent.height == vkSurface->surfaceData->height)
			{
				renderSurface->width = surfaceInfo.currentExtent.width;
				renderSurface->height = surfaceInfo.currentExtent.height;
				DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
				return true;
			}
		}
		else if (result != VK_ERROR_OUT_OF_DATE_KHR)
		{
			DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
			return dsHandleVkResult(result);
		}
	}

	VkSwapchainKHR prevSwapchain = vkSurface->surfaceData ? vkSurface->surfaceData->swapchain : 0;
	dsVkRenderSurfaceData* surfaceData = dsVkRenderSurfaceData_create(vkSurface->scratchAllocator,
		renderer, vkSurface->surface, renderer->vsync, prevSwapchain);
	// Swapchain is retired even if creation failed.
	if (vkSurface->surfaceData)
		dsVkRenderer_deleteRenderSurface(renderer, vkSurface->surfaceData);
	vkSurface->surfaceData = surfaceData;

	if (surfaceData)
	{
		renderSurface->width = vkSurface->surfaceData->width;
		renderSurface->height = vkSurface->surfaceData->height;
	}
	DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));

	return surfaceData != NULL;
}

bool dsVkRenderSurface_beginDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	dsVkRenderSurface* vkSurface = (dsVkRenderSurface*)renderSurface;
	DS_VERIFY(dsSpinlock_lock(&vkSurface->lock));

	if (vkSurface->updatedFrame == renderer->frameNumber)
	{
		DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
		return true;
	}

	if (vkSurface->surfaceData && vkSurface->surfaceData->vsync == renderer->vsync)
	{
		dsVkSurfaceResult result = dsVkRenderSurfaceData_acquireImage(vkSurface->surfaceData);
		if (result == dsVkSurfaceResult_Success)
		{
			bool success = dsVkCommandBuffer_addRenderSurface(commandBuffer,
				vkSurface->surfaceData);
			if (success)
				vkSurface->updatedFrame = renderer->frameNumber;
			DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
			return success;
		}
		else if (result == dsVkSurfaceResult_Error)
		{
			DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
			return false;
		}
	}

	VkSwapchainKHR prevSwapchain = vkSurface->surfaceData ? vkSurface->surfaceData->swapchain : 0;
	dsVkRenderSurfaceData* surfaceData = dsVkRenderSurfaceData_create(vkSurface->scratchAllocator,
		renderer, vkSurface->surface, renderer->vsync, prevSwapchain);
	// Swapchain is retired even if creation failed.
	if (vkSurface->surfaceData)
		dsVkRenderer_deleteRenderSurface(renderer, vkSurface->surfaceData);
	vkSurface->surfaceData = surfaceData;

	bool success = false;
	if (surfaceData)
	{
		if (dsVkRenderSurfaceData_acquireImage(vkSurface->surfaceData) == dsVkSurfaceResult_Success)
		{
			success = dsVkCommandBuffer_addResource(commandBuffer,
				&vkSurface->surfaceData->resource);
			if (success)
				vkSurface->updatedFrame = renderer->frameNumber;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));

	return success;
}

bool dsVkRenderSurface_endDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	DS_UNUSED(renderer);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(renderSurface);

	return true;
}

bool dsVkRenderSurface_swapBuffers(dsRenderer* renderer, dsRenderSurface** renderSurfaces,
	uint32_t count)
{
	VkSemaphore semaphore = dsVkRenderer_flushImpl(renderer, true);
	VkSwapchainKHR* swapchains = DS_ALLOCATE_STACK_OBJECT_ARRAY(VkSwapchainKHR, count);
	uint32_t* imageIndices = DS_ALLOCATE_STACK_OBJECT_ARRAY(uint32_t, count);
	for (uint32_t i = 0; i < count; ++i)
	{
		dsVkRenderSurface* vkSurface = (dsVkRenderSurface*)renderSurfaces[i];
		DS_VERIFY(dsSpinlock_lock(&vkSurface->lock));
		if (!vkSurface->surfaceData)
		{
			DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
			errno = EAGAIN;
			return false;
		}

		swapchains[i] = vkSurface->surfaceData->swapchain;
		imageIndices[i] = vkSurface->surfaceData->imageIndex;
		DS_VERIFY(dsSpinlock_unlock(&vkSurface->lock));
	}

	VkPresentInfoKHR presentInfo =
	{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		NULL,
		1, &semaphore,
		count, swapchains, imageIndices,
		NULL
	};

	dsVkDevice* device = &((dsVkRenderer*)renderer)->device;
	VkResult result = DS_VK_CALL(device->vkQueuePresentKHR)(device->queue, &presentInfo);
	return dsHandleVkResult(result);
}

bool dsVkRenderSurface_destroy(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	dsVkRenderSurface* vkSurface = (dsVkRenderSurface*)renderSurface;
	if (vkSurface->surfaceData)
		dsVkRenderer_deleteRenderSurface(renderer, vkSurface->surfaceData);
	dsSpinlock_shutdown(&vkSurface->lock);
	if (renderSurface->allocator)
		DS_VERIFY(dsAllocator_free(renderSurface->allocator, renderSurface));
	return true;
}