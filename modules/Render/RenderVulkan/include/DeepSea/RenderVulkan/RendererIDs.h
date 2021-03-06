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

#pragma once

#include <DeepSea/Core/Config.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief File containing the renderer IDs for Vulkan.
 */

/**
 * @brief Log tag for the RenderVulkan library.
 */
#define DS_RENDER_VULKAN_LOG_TAG "vulkan"

/**
 * @brief Constant for the renderer type ID of the Vulkan renderer.
 */
#define DS_VK_RENDERER_ID DS_FOURCC('V', 'K', 0, 0)

/**
 * @brief Constant for the renderer platform type ID of xlib.
 */
#define DS_VK_XLIB_RENDERER_PLATFORM_ID DS_FOURCC('X', 'L', 'I', 'B')

/**
 * @brief Constant for the renderer platform type ID of Win32.
 */
#define DS_VK_WIN32_RENDERER_PLATFORM_ID DS_FOURCC('W', 'I', 'N', 0)

#ifdef __cplusplus
}
#endif
