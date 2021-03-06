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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene thread managers.
 * @see dsSceneThreadManager
 */

/**
 * @brief Creates a thread manager.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the thread manager with. This must support freeing
 *     memory.
 * @param renderer The renderer.
 * @param threadCount The number of additional threads to create. This must be at least one, and
 *     each thread will request a resource context from the dsResourceManager held by renderer.
 * @return The
 */
DS_SCENE_EXPORT dsSceneThreadManager* dsSceneThreadManager_create(dsAllocator* allocator,
	dsRenderer* renderer, uint32_t threadCount);

/**
 * @brief Destroys a thread manager.
 * @remark errno will be set on failure.
 * @param threadManager The thread manager to destroy.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneThreadManager_destroy(dsSceneThreadManager* threadManager);

#ifdef __cplusplus
}
#endif
