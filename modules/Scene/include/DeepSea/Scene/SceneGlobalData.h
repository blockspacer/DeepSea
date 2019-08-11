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
 * @brief Functions for creating and manipulating scene global data.
 * @see dsSceneGlobalData
 */

/**
 * @brief Populates the global data.
 * @remark errno will be set on failure.
 * @param globalData The global data.
 * @param view The view being drawn.
 * @return False if the data couldn't be set.
 */
DS_SCENE_EXPORT bool dsSceneGlobalData_populateData(dsSceneGlobalData* globalData,
	const dsView* view);

/**
 * @brief Finishes the current set of global data.
 * @remark errno will be set on failure.
 * @param globalData The global data.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneGlobalData_finish(dsSceneGlobalData* globalData);

/**
 * @brief Destroys a scene global data object.
 * @remark errno will be set on failure.
 * @param globalData The global data to destroy.
 * @return False if the global data couldn't be destroyed.
 */
DS_SCENE_EXPORT bool dsSceneGlobalData_destroy(dsSceneGlobalData* globalData);

#ifdef __cplusplus
}
#endif
