/*
 * Copyright 2017-2018 Aaron Barany
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

#include "AnyGL/gl.h"
#include <DeepSea/Render/Resources/Types.h>

dsShaderModule* dsGLShaderModule_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	mslModule* module, const char* name);
bool dsGLShaderModule_destroy(dsResourceManager* resourceManager, dsShaderModule* module);

bool dsGLShaderModule_compileShader(GLuint* outShader, dsShaderModule* module, uint32_t shaderIndex,
	GLenum stage, const char* pipelineName);
