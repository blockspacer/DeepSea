/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <gtest/gtest.h>
#include <string.h>

namespace
{

struct TestData
{
	float f;
	int i;
};

} // namespace

class GfxBufferTest : public FixtureBase
{
};

TEST_F(GfxBufferTest, Create)
{
	EXPECT_FALSE(dsGfxBuffer_create(resourceManager, NULL, 0, 0, NULL, 100));
	EXPECT_FALSE(dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex, 0, NULL, 100));
	EXPECT_FALSE(dsGfxBuffer_create(resourceManager, NULL, 12345, dsGfxMemory_Static, NULL, 100));

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 100);
	EXPECT_TRUE(buffer);
	EXPECT_EQ(1U, resourceManager->bufferCount);
	EXPECT_EQ(100U, resourceManager->bufferMemorySize);
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
	EXPECT_EQ(0U, resourceManager->bufferCount);
	EXPECT_EQ(0U, resourceManager->bufferMemorySize);
}

TEST_F(GfxBufferTest, Map)
{
	TestData testData = {1.2f, 3};

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Draw, &testData, sizeof(testData));
	ASSERT_TRUE(buffer);
	EXPECT_FALSE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, DS_MAP_FULL_BUFFER));
	EXPECT_TRUE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER));
	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));

	buffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Read | dsGfxMemory_GpuOnly, &testData, sizeof(testData));
	ASSERT_TRUE(buffer);
	EXPECT_FALSE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, DS_MAP_FULL_BUFFER));
	EXPECT_FALSE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));

	buffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Draw | dsGfxMemory_Read | dsGfxMemory_Persistent,
		&testData, sizeof(testData));
	ASSERT_TRUE(buffer);
	EXPECT_FALSE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, sizeof(TestData) + 10));
	void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, DS_MAP_FULL_BUFFER);
	ASSERT_TRUE(data);
	EXPECT_EQ(0U, memcmp(&testData, data, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read | dsGfxBufferMap_Write |
		dsGfxBufferMap_Persistent, 4, 4);
	ASSERT_TRUE(data);
	EXPECT_EQ(3, *(int*)data);
	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	// Limit the map support to test error checking.
	resourceManager->bufferMapSupport = dsGfxBufferMapSupport_Range;
	EXPECT_FALSE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Read | dsGfxBufferMap_Write |
			dsGfxBufferMap_Persistent, 4, 4));
	EXPECT_TRUE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Read | dsGfxBufferMap_Write, 4, 4));
	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_FALSE(dsGfxBuffer_flush(buffer, 0, sizeof(testData)));
	EXPECT_FALSE(dsGfxBuffer_invalidate(buffer, 0, sizeof(testData)));

	resourceManager->bufferMapSupport = dsGfxBufferMapSupport_Full;
	data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read | dsGfxBufferMap_Write, 4, 4);
	ASSERT_TRUE(data);
	EXPECT_EQ(3, *(int*)data);
	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	resourceManager->bufferMapSupport = dsGfxBufferMapSupport_None;
	EXPECT_FALSE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Read | dsGfxBufferMap_Write, 0,
		sizeof(testData)));

	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

TEST_F(GfxBufferTest, FlushInvalidate)
{
	TestData testData = {1.2f, 3};

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Draw, &testData, sizeof(testData));
	ASSERT_TRUE(buffer);

	EXPECT_TRUE(dsGfxBuffer_flush(buffer, 0, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_invalidate(buffer, 0, sizeof(testData)));

	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));

	buffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Draw | dsGfxMemory_Coherent, &testData, sizeof(testData));
	ASSERT_TRUE(buffer);

	EXPECT_TRUE(dsGfxBuffer_flush(buffer, 0, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_invalidate(buffer, 0, sizeof(testData)));

	// Limit the map support to test error checking.
	resourceManager->bufferMapSupport = dsGfxBufferMapSupport_Range;

	EXPECT_FALSE(dsGfxBuffer_flush(buffer, 0, sizeof(testData)));
	EXPECT_FALSE(dsGfxBuffer_invalidate(buffer, 0, sizeof(testData)));

	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

TEST_F(GfxBufferTest, CopyData)
{
	TestData testData = {1.2f, 3};
	TestData copyData = {3.4f, 5};
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Read, &testData, sizeof(testData));
	ASSERT_TRUE(buffer);
	EXPECT_FALSE(dsGfxBuffer_copyData(commandBuffer, buffer, 0, &copyData, sizeof(copyData)));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));

	buffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static | dsGfxMemory_Read,
		&testData, sizeof(testData));
	ASSERT_TRUE(buffer);
	EXPECT_FALSE(dsGfxBuffer_copyData(commandBuffer, buffer, 4, &copyData, sizeof(copyData)));
	EXPECT_TRUE(dsGfxBuffer_copyData(commandBuffer, buffer, 0, &copyData, sizeof(copyData)));

	void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, sizeof(copyData));
	ASSERT_TRUE(data);
	EXPECT_EQ(0U, memcmp(&copyData, data, sizeof(copyData)));
	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

TEST_F(GfxBufferTest, Copy)
{
	TestData testData = {1.2f, 3};
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsGfxBuffer* fromBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
		dsGfxMemory_GpuOnly, &testData, sizeof(testData));
	ASSERT_TRUE(fromBuffer);
	dsGfxBuffer* toBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyTo,
		dsGfxMemory_Static | dsGfxMemory_Read, NULL, sizeof(testData));
	ASSERT_TRUE(toBuffer);
	EXPECT_FALSE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 0, toBuffer, 0, sizeof(testData)));

	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));

	fromBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyFrom,
		dsGfxMemory_GpuOnly, &testData, sizeof(testData));
	ASSERT_TRUE(fromBuffer);
	toBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Read, NULL, sizeof(testData));
	ASSERT_TRUE(toBuffer);
	EXPECT_FALSE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 0, toBuffer, 0, sizeof(testData)));

	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));

	fromBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyFrom,
		dsGfxMemory_GpuOnly, &testData, sizeof(testData));
	ASSERT_TRUE(fromBuffer);
	toBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyTo,
		dsGfxMemory_Static | dsGfxMemory_Read, NULL, sizeof(testData));
	ASSERT_TRUE(toBuffer);
	EXPECT_FALSE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 4, toBuffer, 0, sizeof(testData)));
	EXPECT_FALSE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 0, toBuffer, 4, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 0, toBuffer, 0, sizeof(testData)));

	void* data = dsGfxBuffer_map(toBuffer, dsGfxBufferMap_Read, 0, sizeof(testData));
	ASSERT_TRUE(data);
	EXPECT_EQ(0U, memcmp(&testData, data, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_unmap(toBuffer));

	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));
}
