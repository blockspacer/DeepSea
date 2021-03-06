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

#include "VectorScratchDataImpl.h"
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/RenderMock/MockRenderer.h>
#include <DeepSea/VectorDraw/Export.h>
#include <gtest/gtest.h>

class FixtureBase : public testing::Test
{
public:
	void SetUp() override
	{
		dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT);
		renderer = dsMockRenderer_create(&allocator.allocator);
		ASSERT_TRUE(renderer);
		resourceManager = renderer->resourceManager;
		dsVectorImage_testing = true;
	}

	void TearDown() override
	{
		dsVectorImage_testing = false;
		dsRenderer_destroy(renderer);
		EXPECT_EQ(0U, allocator.allocator.size);
	}

	dsSystemAllocator allocator;
	dsRenderer* renderer;
	dsResourceManager* resourceManager;
};
