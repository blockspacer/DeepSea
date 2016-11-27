/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for interacting with a resource manager.
 *
 * These functions are for dealing with the resource manager in general. Functions for creating and
 * manipulating specific resource types are found in the .h files for that resource type.
 */

/**
 * @brief Create a resource context for the current thread.
 *
 * This will allow resources to be created and manipulated from the current thread. It will remain
 * valid until dsResourceManager_destroyResourceContext() is called.
 *
 * @param resourceManager The resource manager
 * @return False if the resource context couldn't be created. errno will be set to an appropriate
 *     value on failure.
 */
DS_RENDER_EXPORT bool dsResourceManager_createResourceContext(dsResourceManager* resourceManager);

/**
 * @brief Destroys the resource context for the current thread.
 * @param resourceManager The resource manager
 * @return False if the resource context couldn't be destroyed. errno will be set to an appropriate
 *     value on failure.
 */
DS_RENDER_EXPORT bool dsResourceManager_destroyResourceContext(dsResourceManager* resourceManager);

/**
 * @brief Initializes the private members of a resource manager.
 *
 * This is called by the render implementation.
 *
 * @return False if an error occurred.
 */
DS_RENDER_EXPORT bool dsResourceManager_initialize(dsResourceManager* resourceManager);

/**
 * @brief Destroys the private members of a resource manager.
 *
 * This is called by the render implementation.
 */
DS_RENDER_EXPORT void dsResourceManager_destroy(dsResourceManager* resourceManager);

#ifdef __cplusplus
}
#endif
