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
#include "MTLTypes.h"

uint64_t dsMTLRenderer_flushImpl(dsRenderer* renderer, id<MTLCommandBuffer> extraCommands);
uint64_t dsMTLRenderer_getFinishedSubmitCount(const dsRenderer* renderer);
dsGfxFenceResult dsMTLRenderer_waitForSubmit(const dsRenderer* renderer, uint64_t submitCount,
	unsigned int milliseconds);
void dsMTLRenderer_processBuffer(dsRenderer* renderer, dsMTLGfxBufferData* buffer);
void dsMTLRenderer_processTexture(dsRenderer* renderer, dsTexture* texture);
id<MTLRenderPipelineState> dsMTLRenderer_getClearPipeline(dsRenderer* renderer,
	MTLPixelFormat colorFormats[DS_MAX_ATTACHMENTS], uint32_t colorMask, MTLPixelFormat depthFormat,
	MTLPixelFormat stencilFormat, bool layered, uint32_t samples);
