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
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Packing.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/TextureData.h>
#include <gtest/gtest.h>

extern char assetsDir[];

class TextureDataTest : public FixtureBase
{
public:
	const char* getPath(const char* fileName)
	{
		dsPath_combine(buffer, DS_PATH_MAX, assetsDir, fileName);
		return buffer;
	}

private:
	char buffer[DS_PATH_MAX];
};

struct Color16f
{
	dsHalfFloat r;
	dsHalfFloat g;
	dsHalfFloat b;
	dsHalfFloat a;
};

bool operator==(const dsColor& color1, const dsColor& color2)
{
	return color1.r == color2.r && color1.g == color2.g && color1.b == color2.b &&
		color1.a == color2.a;
}

bool operator==(const dsColor& color1, const Color16f& color2)
{
	dsVector4f color4f = {{dsUnpackHalfFloat(color2.r), dsUnpackHalfFloat(color2.g),
		dsUnpackHalfFloat(color2.b), dsUnpackHalfFloat(color2.a)}};
	return color1.r == round(color4f.r*255) && color1.g == round(color4f.g*255) &&
		color1.b == round(color4f.b*255) && color1.a == round(color4f.a*255);
}

TEST_F(TextureDataTest, Create)
{
	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTextureData* textureData = dsTextureData_create(NULL, format, dsTextureDim_2D, 2, 4, 5, 6);
	EXPECT_FALSE(textureData);

	textureData = dsTextureData_create((dsAllocator*)&allocator, dsGfxFormat_R8G8B8A8,
		dsTextureDim_2D, 2, 4, 5, 6);
	EXPECT_FALSE(textureData);

	textureData = dsTextureData_create((dsAllocator*)&allocator, format, dsTextureDim_2D, 2, 4, 5,
		6);
	ASSERT_TRUE(textureData);
	EXPECT_EQ(format, textureData->format);
	EXPECT_EQ(dsTextureDim_2D, textureData->dimension);
	EXPECT_EQ(2U, textureData->width);
	EXPECT_EQ(4U, textureData->height);
	EXPECT_EQ(5U, textureData->depth);
	EXPECT_EQ(3U, textureData->mipLevels);
	EXPECT_EQ(dsTexture_size(format, dsTextureDim_2D, 2, 4, 5, 3, 1), textureData->dataSize);

	EXPECT_TRUE(dsTextureData_destroy(textureData));
}

TEST_F(TextureDataTest, LoadPvrFile_R8G8B8A8)
{
	dsTextureData* textureData = dsTextureData_loadPvrFile((dsAllocator*)&allocator,
		getPath("asdf"));
	EXPECT_FALSE(textureData);

	textureData = dsTextureData_loadPvrFile((dsAllocator*)&allocator, getPath("test.txt"));
	EXPECT_FALSE(textureData);

	textureData = dsTextureData_loadPvrFile((dsAllocator*)&allocator, getPath("empty.txt"));
	EXPECT_FALSE(textureData);

	textureData = dsTextureData_loadPvrFile((dsAllocator*)&allocator, getPath("test.r8g8b8a8.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), textureData->format);
	EXPECT_EQ(dsTextureDim_2D, textureData->dimension);
	EXPECT_EQ(4U, textureData->width);
	EXPECT_EQ(4U, textureData->height);
	EXPECT_EQ(0U, textureData->depth);
	EXPECT_EQ(1U, textureData->mipLevels);

	ASSERT_EQ(4*4*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_TRUE(dsTextureData_destroy(textureData));
}

TEST_F(TextureDataTest, LoadPvrFile_B8G8R8A8)
{
	dsTextureData* textureData = dsTextureData_loadPvrFile((dsAllocator*)&allocator,
		getPath("test.b8r8g8a8.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm), textureData->format);
	EXPECT_EQ(dsTextureDim_2D, textureData->dimension);
	EXPECT_EQ(4U, textureData->width);
	EXPECT_EQ(4U, textureData->height);
	EXPECT_EQ(0U, textureData->depth);
	EXPECT_EQ(3U, textureData->mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{128, 128, 255, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{255, 255, 128, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{63, 127, 127, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{191, 127, 127, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{159, 64, 159, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{127, 223, 127, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{159, 64, 159, 255}}), textureColors[20]);

	EXPECT_TRUE(dsTextureData_destroy(textureData));
}

TEST_F(TextureDataTest, LoadPvrFile_R16G16B16A16F)
{
	dsTextureData* textureData = dsTextureData_loadPvrFile((dsAllocator*)&allocator,
		getPath("test.r16g16b16a16f.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float), textureData->format);
	EXPECT_EQ(dsTextureDim_2D, textureData->dimension);
	EXPECT_EQ(4U, textureData->width);
	EXPECT_EQ(4U, textureData->height);
	EXPECT_EQ(0U, textureData->depth);
	EXPECT_EQ(3U, textureData->mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(uint16_t)*4, textureData->dataSize);
	const Color16f* textureColors = (const Color16f*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{127, 127, 63, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{127, 127, 191, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{159, 64, 159, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{127, 223, 127, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{159, 64, 159, 255}}), textureColors[20]);

	EXPECT_TRUE(dsTextureData_destroy(textureData));
}

TEST_F(TextureDataTest, LoadPvrFile_ETC2SRGB)
{
	dsTextureData* textureData = dsTextureData_loadPvrFile((dsAllocator*)&allocator,
		getPath("test.etc2srgb.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_ETC2_R8G8B8, dsGfxFormat_SRGB),
		textureData->format);
	EXPECT_EQ(dsTextureDim_2D, textureData->dimension);
	EXPECT_EQ(4U, textureData->width);
	EXPECT_EQ(4U, textureData->height);
	EXPECT_EQ(0U, textureData->depth);
	EXPECT_EQ(3U, textureData->mipLevels);

	EXPECT_TRUE(dsTextureData_destroy(textureData));
}