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

#include <DeepSea/Render/Resources/Renderbuffer.h>

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Types.h>

extern const char* dsResourceManager_noContextError;

dsRenderbuffer* dsRenderbuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsGfxFormat format, uint32_t width, uint32_t height, uint16_t samples)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createRenderbufferFunc || !resourceManager->destroyRenderbufferFunc ||
		width == 0 || height == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (!dsGfxFormat_offscreenSupported(resourceManager, format))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Format not supported for renderbuffers.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	samples = dsMax(1U, samples);
	dsRenderbuffer* renderbuffer = resourceManager->createRenderbufferFunc(resourceManager,
		allocator, format, width, height, samples);
	if (renderbuffer)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->renderbufferCount, 1);
		size_t size = dsTexture_size(format, dsTextureDim_2D, width, height, 0, 1, samples);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->renderbufferMemorySize, size);
	}
	DS_PROFILE_FUNC_RETURN(renderbuffer);
}

bool dsRenderbuffer_destroy(dsRenderbuffer* renderbuffer)
{
	DS_PROFILE_FUNC_START();

	if (!renderbuffer || !renderbuffer->resourceManager ||
		!renderbuffer->resourceManager->destroyRenderbufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = renderbuffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	size_t size = dsTexture_size(renderbuffer->format, dsTextureDim_2D, renderbuffer->width,
		renderbuffer->height, 0, 1, renderbuffer->samples);
	bool success = resourceManager->destroyRenderbufferFunc(resourceManager, renderbuffer);
	if (success)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->renderbufferCount, -1);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->renderbufferMemorySize, -size);
	}
	DS_PROFILE_FUNC_RETURN(success);
}