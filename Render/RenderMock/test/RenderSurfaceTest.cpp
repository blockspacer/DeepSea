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

#include "FixtureBase.h"
#include <DeepSea/Render/RenderSurface.h>
#include <gtest/gtest.h>

class RenderSurfaceTest : public FixtureBase
{
};

extern "C" DS_RENDERMOCK_EXPORT bool dsMockRenderSurface_changeSize;

TEST_F(RenderSurfaceTest, Create)
{
	EXPECT_FALSE(dsRenderSurface_create(NULL, NULL, NULL, dsRenderSurfaceType_Direct));

	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, NULL,
		dsRenderSurfaceType_Direct);
	ASSERT_TRUE(renderSurface);

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}

TEST_F(RenderSurfaceTest, Update)
{
	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, NULL,
		dsRenderSurfaceType_Direct);
	ASSERT_TRUE(renderSurface);

	EXPECT_FALSE(dsRenderSurface_update(NULL));
	EXPECT_FALSE(dsRenderSurface_update(renderSurface));

	EXPECT_EQ(1920U, renderSurface->width);
	EXPECT_EQ(1080U, renderSurface->height);

	dsMockRenderSurface_changeSize = true;
	EXPECT_TRUE(dsRenderSurface_update(renderSurface));
	dsMockRenderSurface_changeSize = false;

	EXPECT_EQ(1910U, renderSurface->width);
	EXPECT_EQ(1070U, renderSurface->height);

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}

TEST_F(RenderSurfaceTest, BeginEnd)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, NULL,
		dsRenderSurfaceType_Direct);
	ASSERT_TRUE(renderSurface);

	EXPECT_FALSE(dsRenderSurface_beginDraw(NULL, renderSurface));
	EXPECT_FALSE(dsRenderSurface_beginDraw(commandBuffer, NULL));
	EXPECT_TRUE(dsRenderSurface_beginDraw(commandBuffer, renderSurface));

	EXPECT_FALSE(dsRenderSurface_endDraw(NULL, renderSurface));
	EXPECT_FALSE(dsRenderSurface_endDraw(commandBuffer, NULL));
	EXPECT_TRUE(dsRenderSurface_endDraw(commandBuffer, renderSurface));

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}

TEST_F(RenderSurfaceTest, SwapBuffers)
{
	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, NULL,
		dsRenderSurfaceType_Direct);
	ASSERT_TRUE(renderSurface);

	EXPECT_FALSE(dsRenderSurface_swapBuffers(NULL));
	EXPECT_TRUE(dsRenderSurface_swapBuffers(renderSurface));

	renderer->doubleBuffer = false;
	EXPECT_FALSE(dsRenderSurface_swapBuffers(renderSurface));

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}
