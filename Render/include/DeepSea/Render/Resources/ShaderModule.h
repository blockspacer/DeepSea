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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating shader modules.
 *
 * Shader modules contain a list of shaders, which can then be used for rendering objects on screen.
 */

/**
 * @brief Loads a shader module from a file.
 *
 * The shader module is expected to be an mslb file created with ModuleShaderLanguage.
 *
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param filePath The file path for the shader module to load.
 * @return The created shader module, or NULL if it couldn't be created. errno will be set to an
 *     appropriate value on failure.
 */
DS_RENDER_EXPORT dsShaderModule* dsShaderModule_loadFile(dsResourceManager* resourceManager,
	dsAllocator* allocator, const char* filePath);

/**
 * @brief Loads a shader module from a stream.
 *
 * The shader module is expected to have been created with ModuleShaderLanguage.
 *
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param stream The stream to load the shader module from. This stream will be read from the
 *     current position until the end, and must be seekable.
 * @return The created shader module, or NULL if it couldn't be created. errno will be set to an
 *     appropriate value on failure.
 */
DS_RENDER_EXPORT dsShaderModule* dsShaderModule_loadStream(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsStream* stream);

/**
 * @brief Loads a shader module from a data buffer.
 *
 * The shader module is expected to have been created with ModuleShaderLanguage.
 *
 * @param resourceManager The resource manager to create the shader module from.
 * @param allocator The allocator to create the shader module with. If NULL, it will use the same
 *     allocator as the resource manager.
 * @param data The data for the shader module. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @return The created shader module, or NULL if it couldn't be created. errno will be set to an
 *     appropriate value on failure.
 */
DS_RENDER_EXPORT dsShaderModule* dsShaderModule_loadData(dsResourceManager* resourceManager,
	dsAllocator* allocator, const void* data, size_t size);


/**
 * @brief Gets the number of shaders within a module.
 * @param shaderModule The shader module.
 * @return The number of shader pipelines.
 */
DS_RENDER_EXPORT uint32_t dsShaderModule_shaderCount(const dsShaderModule* shaderModule);

/**
 * @brief Gets the name of a shader within a module.
 * @param shaderModule The shader module.
 * @param shader The index of the shader.
 * @return The name of the shader, or NULL if the parameters are invalid.
 */
DS_RENDER_EXPORT const char* dsShaderModule_shaderName(const dsShaderModule* shaderModule,
	uint32_t shader);

/**
 * @brief Destroys a shader module.
 * @param shaderModule The shader module to destroy.
 * @return False if the shader module couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsShaderModule_destroy(dsShaderModule* shaderModule);

#ifdef __cplusplus
}
#endif