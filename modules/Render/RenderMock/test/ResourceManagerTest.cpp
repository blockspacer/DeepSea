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

#include "Fixtures/FixtureBase.h"
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <gtest/gtest.h>

namespace
{

struct ThreadData
{
	dsResourceManager* resourceManager;
	dsMutex* mutex;
	dsConditionVariable* condition;
	uint32_t created;
};

dsThreadReturnType createResourceContextThread(void* data)
{
	ThreadData* threadData = (ThreadData*)data;
	if (!dsResourceManager_createResourceContext(threadData->resourceManager))
	{
		EXPECT_FALSE(dsResourceManager_canUseResources(threadData->resourceManager));
		return false;
	}

	EXPECT_TRUE(dsResourceManager_canUseResources(threadData->resourceManager));
	EXPECT_GE(threadData->resourceManager->maxResourceContexts,
		threadData->resourceManager->resourceContextCount);
	if (threadData->condition)
	{
		EXPECT_TRUE(dsMutex_lock(threadData->mutex));
		uint32_t created = true;
		DS_ATOMIC_STORE32(&threadData->created, &created);
		EXPECT_EQ(dsConditionVariableResult_Success,
			dsConditionVariable_wait(threadData->condition, threadData->mutex));
		EXPECT_TRUE(dsMutex_unlock(threadData->mutex));
	}

	EXPECT_TRUE(dsResourceManager_destroyResourceContext(threadData->resourceManager));
	EXPECT_TRUE(dsResourceManager_destroyResourceContext(threadData->resourceManager));
	return true;
}

} // namespace

class ResourceManagerTest : public FixtureBase
{
};

TEST_F(ResourceManagerTest, CreateResourceContext)
{
	EXPECT_TRUE(dsResourceManager_canUseResources(resourceManager));
	EXPECT_FALSE(dsResourceManager_createResourceContext(resourceManager));

	ThreadData firstThreadData =
	{
		resourceManager,
		dsMutex_create(&allocator.allocator, NULL),
		dsConditionVariable_create(&allocator.allocator, NULL),
		false
	};
	dsThread firstThread;
	EXPECT_TRUE(dsThread_create(&firstThread, &createResourceContextThread, &firstThreadData, 0,
		NULL));

	do
	{
		uint32_t created;
		DS_ATOMIC_LOAD32(&firstThreadData.created, &created);
		if (created)
			break;
		dsThread_yield();
	} while (true);

	EXPECT_TRUE(dsMutex_lock(firstThreadData.mutex));

	ThreadData secondThreadData = {resourceManager, NULL, NULL, false};
	dsThread secondThread;
	EXPECT_TRUE(dsThread_create(&secondThread, &createResourceContextThread, &secondThreadData, 0,
		NULL));
	dsThreadReturnType secondThreadReturn;
	EXPECT_TRUE(dsThread_join(&secondThread, &secondThreadReturn));
	EXPECT_FALSE(secondThreadReturn);

	EXPECT_TRUE(dsConditionVariable_notifyAll(firstThreadData.condition));
	EXPECT_TRUE(dsMutex_unlock(firstThreadData.mutex));

	dsThreadReturnType firstThreadReturn;
	EXPECT_TRUE(dsThread_join(&firstThread, &firstThreadReturn));
	EXPECT_TRUE(firstThreadReturn);

	EXPECT_EQ(0U, resourceManager->resourceContextCount);

	dsConditionVariable_destroy(firstThreadData.condition);
	dsMutex_destroy(firstThreadData.mutex);
}

TEST_F(ResourceManagerTest, CreateResourceContextContention)
{
	const unsigned int threadCount = 100;
	ThreadData threadData[threadCount] = {};
	dsThread threads[threadCount];

	for (unsigned int i = 0; i < threadCount; ++i)
		threadData[i].resourceManager = resourceManager;

	for (unsigned int i = 0; i < threadCount; ++i)
		dsThread_create(threads + i, &createResourceContextThread, threadData + 1, 0, NULL);

	for (unsigned int i = 0; i < threadCount; ++i)
		dsThread_join(threads + i, NULL);

	EXPECT_EQ(0U, resourceManager->resourceContextCount);
}
