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

#include <DeepSea/Scene/View.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Scene/SceneCullManager.h>

uint32_t dsView_registerCullID(const dsView* view, dsSceneCullID cullID)
{
	if (!view || !cullID)
	{
		errno = EINVAL;
		return DS_NO_SCENE_CULL;
	}

	return dsSceneCullManager_registerCullID((dsSceneCullManager*)&view->cullManager, cullID);
}
