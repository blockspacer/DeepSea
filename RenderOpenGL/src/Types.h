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

#include "AnyGL/gl.h"
#include <DeepSea/RenderOpenGL/Types.h>
#include <DeepSea/Core/Types.h>

struct dsResourceContext
{
	void* context;
	void* dummySurface;
	void* dummyOsSurface;
	bool claimed;
};

typedef struct dsGLGfxBuffer
{
	dsGfxBuffer buffer;
	GLuint bufferId;
} dsGLGfxBuffer;

typedef struct dsGLResourceManager
{
	dsResourceManager resourceManager;
	dsResourceContext* resourceContexts;
	dsMutex* mutex;
} dsGLResourceManager;

typedef struct dsGLRenderer
{
	dsRenderer renderer;
	dsOpenGLOptions options;
	bool releaseDisplay;

	void* sharedConfig;
	void* sharedContext;
	void* dummySurface;
	void* dummyOsSurface;
	void* renderConfig;
	void* renderContext;
} dsGLRenderer;