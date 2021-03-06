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

#include "Fixtures/FixtureBase.h"
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Renderbuffer.h>
#include <gtest/gtest.h>

class RenderbufferTest : public FixtureBase
{
};

TEST_F(RenderbufferTest, Create)
{
	EXPECT_FALSE(dsRenderbuffer_create(NULL, NULL, dsRenderbufferUsage_Standard,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), 128, 256, 4));
	EXPECT_FALSE(dsRenderbuffer_create(resourceManager, NULL, dsRenderbufferUsage_Standard,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), 0, 256, 4));
	EXPECT_FALSE(dsRenderbuffer_create(resourceManager, NULL, dsRenderbufferUsage_Standard,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), 128, 0, 4));

	dsRenderbuffer* renderbuffer = dsRenderbuffer_create(resourceManager, NULL,
		dsRenderbufferUsage_Standard, dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		128, 256, 4);
	ASSERT_TRUE(renderbuffer);

	EXPECT_EQ(1U, resourceManager->renderbufferCount);
	EXPECT_EQ((size_t)(128*256*4*4), resourceManager->renderbufferMemorySize);

	EXPECT_TRUE(dsRenderbuffer_destroy(renderbuffer));
	EXPECT_EQ(0U, resourceManager->renderbufferCount);
	EXPECT_EQ(0U, resourceManager->renderbufferMemorySize);
}
