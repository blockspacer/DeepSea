/*
 * Copyright 2016-2018 Aaron Barany
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

#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <gtest/gtest.h>
#include <string.h>
#include <string>
#include <vector>

namespace
{

struct PushInfo
{
	PushInfo(dsProfileType type_, const char* name_, const char* file_, const char* function_,
		unsigned int line_, bool dynamic_)
		: type(type_), name(name_), file(file_), function(function_), line(line_),
		  dynamic(dynamic_) {}

	dsProfileType type;
	std::string name;
	std::string file;
	std::string function;
	unsigned int line;
	bool dynamic;
};

struct PopInfo
{
	PopInfo(dsProfileType type_, const char* file_, const char* function_, unsigned int line_)
		: type(type_), file(file_), function(function_), line(line_) {}

	dsProfileType type;
	std::string file;
	std::string function;
	unsigned int line;
};

struct StatInfo
{
	StatInfo(const char* category_, const char* name_, double value_, const char* file_,
		const char* function_, unsigned int line_, bool dynamic_)
		: category(category_), name(name_), value(value_), file(file_), function(function_),
		  line(line_), dynamic(dynamic_) {}

	std::string category;
	std::string name;
	double value;
	std::string file;
	std::string function;
	unsigned int line;
	bool dynamic;
};

struct ProfileInfo
{
	ProfileInfo()
	{
		EXPECT_TRUE(dsSpinlock_initialize(&spinlock));
	}

	dsSpinlock spinlock;
	std::vector<PushInfo> push;
	std::vector<PopInfo> pop;
	std::vector<StatInfo> stat;
};

void profileRegisterThread(void*, const char*)
{
}

void profileStartFrame(void*)
{
}

void profileEndFrame(void*)
{
}

void profilePush(void* userData, void**, dsProfileType type, const char* name, const char* file,
	const char* function, unsigned int line, bool dynamic)
{
	EXPECT_TRUE(dsSpinlock_lock(&((ProfileInfo*)userData)->spinlock));
	((ProfileInfo*)userData)->push.emplace_back(type, name, file, function, line, dynamic);
	EXPECT_TRUE(dsSpinlock_unlock(&((ProfileInfo*)userData)->spinlock));
}

void profilePop(void* userData, dsProfileType type, const char* file, const char* function,
	unsigned int line)
{
	EXPECT_TRUE(dsSpinlock_lock(&((ProfileInfo*)userData)->spinlock));
	((ProfileInfo*)userData)->pop.emplace_back(type, file, function, line);
	EXPECT_TRUE(dsSpinlock_unlock(&((ProfileInfo*)userData)->spinlock));
}

void profileStat(void* userData, void**, const char* category, const char* name, double value,
	const char* file, const char* function, unsigned int line, bool dynamic)
{
	EXPECT_TRUE(dsSpinlock_lock(&((ProfileInfo*)userData)->spinlock));
	((ProfileInfo*)userData)->stat.emplace_back(category, name, value, file, function, line,
		dynamic);
	EXPECT_TRUE(dsSpinlock_unlock(&((ProfileInfo*)userData)->spinlock));
}

void profileGpu(void*, const char*, const char*, uint64_t)
{
}

#if DS_PROFILING_ENABLED

void voidFunction()
{
	DS_PROFILE_FUNC_START();
	DS_PROFILE_FUNC_RETURN_VOID();
}

int intFunction(int retVal)
{
	DS_PROFILE_FUNC_START_NAME("Custom Function");
	DS_PROFILE_FUNC_RETURN(retVal);
}

struct ThreadData
{
	dsMutex* mutex;
	dsConditionVariable* condition;
};

dsThreadReturnType threadFunc(void* userData)
{
	ThreadData* threadData = (ThreadData*)userData;
	EXPECT_TRUE(dsMutex_lock(threadData->mutex));
	EXPECT_TRUE(dsConditionVariable_notifyAll(threadData->condition));
	EXPECT_TRUE(dsMutex_unlock(threadData->mutex));
	return 0;
}

#endif // DS_PROFILING_ENABLED

} // namespace

TEST(Profile, SetFunctions)
{
	EXPECT_EQ(nullptr, dsProfile_getUserData());
	const dsProfileFunctions* curFunctions = dsProfile_getFunctions();
	EXPECT_EQ(nullptr, curFunctions->registerThreadFunc);
	EXPECT_EQ(nullptr, curFunctions->startFrameFunc);
	EXPECT_EQ(nullptr, curFunctions->endFrameFunc);
	EXPECT_EQ(nullptr, curFunctions->pushFunc);
	EXPECT_EQ(nullptr, curFunctions->popFunc);
	EXPECT_EQ(nullptr, curFunctions->statFunc);
	EXPECT_EQ(nullptr, curFunctions->gpuFunc);

	ProfileInfo info;
	dsProfileFunctions functions =
	{
		&profileRegisterThread, &profileStartFrame, &profileEndFrame, &profilePush, &profilePop,
		&profileStat, &profileGpu
	};
	dsProfile_setFunctions(&info, &functions);

	EXPECT_EQ(&info, dsProfile_getUserData());
	EXPECT_EQ(&profileRegisterThread, curFunctions->registerThreadFunc);
	EXPECT_EQ(&profileStartFrame, curFunctions->startFrameFunc);
	EXPECT_EQ(&profileEndFrame, curFunctions->endFrameFunc);
	EXPECT_EQ(&profilePush, curFunctions->pushFunc);
	EXPECT_EQ(&profilePop, curFunctions->popFunc);
	EXPECT_EQ(&profileStat, curFunctions->statFunc);
	EXPECT_EQ(&profileGpu, curFunctions->gpuFunc);

	dsProfile_clearFunctions();

	EXPECT_EQ(nullptr, dsProfile_getUserData());
	EXPECT_EQ(nullptr, curFunctions->registerThreadFunc);
	EXPECT_EQ(nullptr, curFunctions->startFrameFunc);
	EXPECT_EQ(nullptr, curFunctions->endFrameFunc);
	EXPECT_EQ(nullptr, curFunctions->pushFunc);
	EXPECT_EQ(nullptr, curFunctions->popFunc);
	EXPECT_EQ(nullptr, curFunctions->statFunc);
	EXPECT_EQ(nullptr, curFunctions->gpuFunc);
}

#if DS_PROFILING_ENABLED

TEST(Profile, Macros)
{
	ProfileInfo info;
	dsProfileFunctions functions =
	{
		&profileRegisterThread, &profileStartFrame, &profileEndFrame, &profilePush, &profilePop,
		&profileStat, &profileGpu
	};
	dsProfile_setFunctions(&info, &functions);

	voidFunction();
	EXPECT_EQ(10, intFunction(10));
	DS_PROFILE_SCOPE_START("Scope");
	DS_PROFILE_SCOPE_END();
	DS_PROFILE_DYNAMIC_SCOPE_START("DynamicScope");
	DS_PROFILE_SCOPE_END();
	DS_PROFILE_WAIT_START("Wait");
	DS_PROFILE_WAIT_END();
	DS_PROFILE_DYNAMIC_WAIT_START("DynamicWait");
	DS_PROFILE_WAIT_END();
	DS_PROFILE_LOCK_START("Lock");
	DS_PROFILE_LOCK_END();
	DS_PROFILE_DYNAMIC_LOCK_START("DynamicLock");
	DS_PROFILE_LOCK_END();
	DS_PROFILE_STAT("Category", "Name", 10);
	DS_PROFILE_DYNAMIC_STAT("Dynamic", "Name", 10);

	dsProfile_clearFunctions();

	const char* fileName = "ProfileTest.cpp";
	const char* functionName = "TestBody";

	// Push
	ASSERT_EQ(8U, info.push.size());
	EXPECT_EQ(dsProfileType_Function, info.push[0].type);
	EXPECT_NE(std::string::npos, info.push[0].name.find("voidFunction"));
	std::size_t separatorIndex = info.push[0].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.push[0].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.push[0].function.find("voidFunction"));
	EXPECT_NE(0U, info.push[0].line);
	EXPECT_FALSE(info.push[0].dynamic);

	EXPECT_EQ(dsProfileType_Function, info.push[1].type);
	EXPECT_EQ("Custom Function", info.push[1].name);
	separatorIndex = info.push[1].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.push[1].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.push[1].function.find("intFunction"));
	EXPECT_NE(0U, info.push[1].line);
	EXPECT_FALSE(info.push[1].dynamic);

	EXPECT_EQ(dsProfileType_Scope, info.push[2].type);
	EXPECT_EQ("Scope", info.push[2].name);
	separatorIndex = info.push[2].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.push[2].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.push[2].function.find(functionName));
	EXPECT_NE(0U, info.push[2].line);
	EXPECT_FALSE(info.push[2].dynamic);

	EXPECT_EQ(dsProfileType_Scope, info.push[3].type);
	EXPECT_EQ("DynamicScope", info.push[3].name);
	separatorIndex = info.push[3].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.push[3].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.push[3].function.find(functionName));
	EXPECT_NE(0U, info.push[3].line);
	EXPECT_TRUE(info.push[3].dynamic);

	EXPECT_EQ(dsProfileType_Wait, info.push[4].type);
	EXPECT_EQ("Wait", info.push[4].name);
	separatorIndex = info.push[4].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.push[4].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.push[4].function.find(functionName));
	EXPECT_NE(0U, info.push[4].line);
	EXPECT_FALSE(info.push[4].dynamic);

	EXPECT_EQ(dsProfileType_Wait, info.push[5].type);
	EXPECT_EQ("DynamicWait", info.push[5].name);
	separatorIndex = info.push[5].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.push[5].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.push[5].function.find(functionName));
	EXPECT_NE(0U, info.push[5].line);
	EXPECT_TRUE(info.push[5].dynamic);

	EXPECT_EQ(dsProfileType_Lock, info.push[6].type);
	EXPECT_EQ("Lock", info.push[6].name);
	separatorIndex = info.push[6].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.push[6].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.push[6].function.find(functionName));
	EXPECT_NE(0U, info.push[6].line);
	EXPECT_FALSE(info.push[6].dynamic);

	EXPECT_EQ(dsProfileType_Lock, info.push[7].type);
	EXPECT_EQ("DynamicLock", info.push[7].name);
	separatorIndex = info.push[7].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.push[7].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.push[7].function.find(functionName));
	EXPECT_NE(0U, info.push[7].line);
	EXPECT_TRUE(info.push[7].dynamic);

	// Pop
	ASSERT_EQ(8U, info.pop.size());
	EXPECT_EQ(dsProfileType_Function, info.pop[0].type);
	separatorIndex = info.pop[0].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.pop[0].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.pop[0].function.find("voidFunction"));
	EXPECT_NE(0U, info.pop[0].line);

	EXPECT_EQ(dsProfileType_Function, info.pop[1].type);
	separatorIndex = info.pop[1].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.pop[1].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.pop[1].function.find("intFunction"));
	EXPECT_NE(0U, info.pop[1].line);

	EXPECT_EQ(dsProfileType_Scope, info.pop[2].type);
	separatorIndex = info.pop[2].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.pop[2].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.pop[2].function.find(functionName));
	EXPECT_NE(0U, info.pop[2].line);

	EXPECT_EQ(dsProfileType_Scope, info.pop[3].type);
	separatorIndex = info.pop[3].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.pop[3].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.pop[3].function.find(functionName));
	EXPECT_NE(0U, info.pop[3].line);

	EXPECT_EQ(dsProfileType_Wait, info.pop[4].type);
	separatorIndex = info.pop[4].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.pop[4].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.pop[4].function.find(functionName));
	EXPECT_NE(0U, info.pop[4].line);

	EXPECT_EQ(dsProfileType_Wait, info.pop[5].type);
	separatorIndex = info.pop[5].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.pop[5].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.pop[5].function.find(functionName));
	EXPECT_NE(0U, info.pop[5].line);

	EXPECT_EQ(dsProfileType_Lock, info.pop[6].type);
	separatorIndex = info.pop[6].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.pop[6].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.pop[6].function.find(functionName));
	EXPECT_NE(0U, info.pop[6].line);

	EXPECT_EQ(dsProfileType_Lock, info.pop[7].type);
	separatorIndex = info.pop[7].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.pop[7].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.pop[7].function.find(functionName));
	EXPECT_NE(0U, info.pop[7].line);

	// Stat
	ASSERT_EQ(2U, info.stat.size());
	EXPECT_EQ("Category", info.stat[0].category);
	EXPECT_EQ("Name", info.stat[0].name);
	EXPECT_EQ(10, info.stat[0].value);
	separatorIndex = info.stat[0].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.stat[0].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.stat[0].function.find(functionName));
	EXPECT_NE(0U, info.stat[0].line);

	EXPECT_EQ("Dynamic", info.stat[1].category);
	EXPECT_EQ("Name", info.stat[1].name);
	EXPECT_EQ(10, info.stat[1].value);
	separatorIndex = info.stat[1].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ(fileName, info.stat[1].file.c_str() + separatorIndex + 1);
	EXPECT_NE(std::string::npos, info.stat[0].function.find(functionName));
	EXPECT_NE(0U, info.stat[0].line);
}

TEST(Profile, ThreadTypes)
{
	ProfileInfo info;
	dsProfileFunctions functions =
	{
		&profileRegisterThread, &profileStartFrame, &profileEndFrame, &profilePush, &profilePop,
		&profileStat, &profileGpu
	};
	dsProfile_setFunctions(&info, &functions);

	dsMutex* mutex = dsMutex_create(nullptr, nullptr);
	dsConditionVariable* condition = dsConditionVariable_create(nullptr, nullptr);

	EXPECT_TRUE(dsMutex_tryLock(mutex));
	EXPECT_TRUE(dsMutex_unlock(mutex));

	EXPECT_TRUE(dsMutex_lock(mutex));
	ThreadData threadData = {mutex, condition};
	dsThread thread;
	EXPECT_TRUE(dsThread_create(&thread, &threadFunc, &threadData, 0, nullptr));
	EXPECT_EQ(dsConditionVariableResult_Success, dsConditionVariable_wait(condition, mutex));
	EXPECT_TRUE(dsMutex_unlock(mutex));

	EXPECT_TRUE(dsThread_join(&thread, nullptr));
	dsThread_sleep(1, nullptr);

	dsMutex_destroy(mutex);
	dsConditionVariable_destroy(condition);

	dsProfile_clearFunctions();

	// push
	ASSERT_EQ(9U, info.push.size());
	EXPECT_EQ(dsProfileType_Lock, info.push[0].type);
	EXPECT_EQ("Mutex", info.push[0].name);
	std::size_t separatorIndex = info.push[0].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.push[0].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_tryLock", info.push[0].function);
	EXPECT_NE(0U, info.push[0].line);
	EXPECT_TRUE(info.push[0].dynamic);

	EXPECT_EQ(dsProfileType_Lock, info.push[1].type);
	EXPECT_EQ("Mutex", info.push[1].name);
	separatorIndex = info.push[1].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.push[1].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_lock", info.push[1].function);
	EXPECT_NE(0U, info.push[1].line);
	EXPECT_TRUE(info.push[1].dynamic);

	EXPECT_EQ(dsProfileType_Wait, info.push[2].type);
	EXPECT_EQ("Mutex", info.push[2].name);
	separatorIndex = info.push[2].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.push[2].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_lock", info.push[2].function);
	EXPECT_NE(0U, info.push[2].line);
	EXPECT_TRUE(info.push[2].dynamic);

	if (info.push[3].name == "Mutex")
	{
		EXPECT_EQ(dsProfileType_Lock, info.push[3].type);
		EXPECT_EQ("Mutex", info.push[3].name);
		separatorIndex = info.push[3].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("Mutex.c", info.push[3].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsMutex_lock", info.push[3].function);
		EXPECT_NE(0U, info.push[3].line);
		EXPECT_TRUE(info.push[3].dynamic);

		if (info.push[4].name == "Mutex")
		{
			EXPECT_EQ(dsProfileType_Wait, info.push[4].type);
			EXPECT_EQ("Mutex", info.push[4].name);
			separatorIndex = info.push[4].file.find_last_of("/\\");
			ASSERT_NE(std::string::npos, separatorIndex);
			EXPECT_STRCASEEQ("Mutex.c", info.push[4].file.c_str() + separatorIndex + 1);
			EXPECT_EQ("dsMutex_lock", info.push[4].function);
			EXPECT_NE(0U, info.push[4].line);
			EXPECT_TRUE(info.push[4].dynamic);

			EXPECT_EQ(dsProfileType_Wait, info.push[5].type);
			EXPECT_EQ("Condition", info.push[5].name);
			separatorIndex = info.push[5].file.find_last_of("/\\");
			ASSERT_NE(std::string::npos, separatorIndex);
			EXPECT_STRCASEEQ("ConditionVariable.c", info.push[5].file.c_str() + separatorIndex + 1);
			EXPECT_EQ("dsConditionVariable_wait", info.push[5].function);
			EXPECT_NE(0U, info.push[5].line);
			EXPECT_TRUE(info.push[5].dynamic);
		}
		else
		{
			EXPECT_EQ(dsProfileType_Wait, info.push[4].type);
			EXPECT_EQ("Condition", info.push[4].name);
			separatorIndex = info.push[4].file.find_last_of("/\\");
			ASSERT_NE(std::string::npos, separatorIndex);
			EXPECT_STRCASEEQ("ConditionVariable.c", info.push[4].file.c_str() + separatorIndex + 1);
			EXPECT_EQ("dsConditionVariable_wait", info.push[4].function);
			EXPECT_NE(0U, info.push[4].line);
			EXPECT_TRUE(info.push[4].dynamic);

			EXPECT_EQ(dsProfileType_Wait, info.push[5].type);
			EXPECT_EQ("Mutex", info.push[5].name);
			separatorIndex = info.push[5].file.find_last_of("/\\");
			ASSERT_NE(std::string::npos, separatorIndex);
			EXPECT_STRCASEEQ("Mutex.c", info.push[5].file.c_str() + separatorIndex + 1);
			EXPECT_EQ("dsMutex_lock", info.push[5].function);
			EXPECT_NE(0U, info.push[5].line);
			EXPECT_TRUE(info.push[5].dynamic);
		}
	}
	else
	{
		EXPECT_EQ(dsProfileType_Wait, info.push[3].type);
		EXPECT_EQ("Condition", info.push[3].name);
		separatorIndex = info.push[3].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("ConditionVariable.c", info.push[3].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsConditionVariable_wait", info.push[3].function);
		EXPECT_NE(0U, info.push[3].line);
		EXPECT_TRUE(info.push[3].dynamic);

		EXPECT_EQ(dsProfileType_Lock, info.push[4].type);
		EXPECT_EQ("Mutex", info.push[4].name);
		separatorIndex = info.push[4].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("Mutex.c", info.push[4].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsMutex_lock", info.push[4].function);
		EXPECT_NE(0U, info.push[4].line);
		EXPECT_TRUE(info.push[4].dynamic);

		EXPECT_EQ(dsProfileType_Wait, info.push[5].type);
		EXPECT_EQ("Mutex", info.push[5].name);
		separatorIndex = info.push[5].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("Mutex.c", info.push[5].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsMutex_lock", info.push[5].function);
		EXPECT_NE(0U, info.push[5].line);
		EXPECT_TRUE(info.push[5].dynamic);
	}

	EXPECT_EQ(dsProfileType_Lock, info.push[6].type);
	EXPECT_EQ("Mutex", info.push[6].name);
	separatorIndex = info.push[6].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("ConditionVariable.c", info.push[6].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsConditionVariable_wait", info.push[6].function);
	EXPECT_NE(0U, info.push[6].line);
	EXPECT_TRUE(info.push[6].dynamic);

	EXPECT_EQ(dsProfileType_Wait, info.push[7].type);
	EXPECT_EQ("Thread", info.push[7].name);
	separatorIndex = info.push[7].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Thread.c", info.push[7].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsThread_join", info.push[7].function);
	EXPECT_NE(0U, info.push[7].line);
	EXPECT_TRUE(info.push[7].dynamic);

	EXPECT_EQ(dsProfileType_Wait, info.push[8].type);
	EXPECT_EQ("Sleep", info.push[8].name);
	separatorIndex = info.push[8].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Thread.c", info.push[8].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsThread_sleep", info.push[8].function);
	EXPECT_NE(0U, info.push[8].line);
	EXPECT_TRUE(info.push[8].dynamic);

	// pop
	ASSERT_EQ(9U, info.pop.size());
	EXPECT_EQ(dsProfileType_Lock, info.pop[0].type);
	separatorIndex = info.pop[0].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.pop[0].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_unlock", info.pop[0].function);
	EXPECT_NE(0U, info.pop[0].line);

	EXPECT_EQ(dsProfileType_Wait, info.pop[1].type);
	separatorIndex = info.pop[1].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.pop[1].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_lock", info.pop[1].function);
	EXPECT_NE(0U, info.pop[1].line);

	EXPECT_EQ(dsProfileType_Lock, info.pop[2].type);
	separatorIndex = info.pop[2].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("ConditioNVariable.c", info.pop[2].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsConditionVariable_wait", info.pop[2].function);
	EXPECT_NE(0U, info.pop[2].line);

	EXPECT_EQ(dsProfileType_Wait, info.pop[3].type);
	separatorIndex = info.pop[3].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.pop[3].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_lock", info.pop[3].function);
	EXPECT_NE(0U, info.pop[3].line);

	if (info.pop[4].type == dsProfileType_Lock)
	{
		EXPECT_EQ(dsProfileType_Lock, info.pop[4].type);
		separatorIndex = info.pop[4].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("Mutex.c", info.pop[4].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsMutex_unlock", info.pop[4].function);
		EXPECT_NE(0U, info.pop[4].line);

		EXPECT_EQ(dsProfileType_Wait, info.pop[5].type);
		separatorIndex = info.pop[5].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("ConditionVariable.c", info.pop[5].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsConditionVariable_wait", info.pop[5].function);
		EXPECT_NE(0U, info.pop[5].line);
	}
	else
	{
		EXPECT_EQ(dsProfileType_Wait, info.pop[4].type);
		separatorIndex = info.pop[4].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("ConditionVariable.c", info.pop[4].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsConditionVariable_wait", info.pop[4].function);
		EXPECT_NE(0U, info.pop[4].line);

		EXPECT_EQ(dsProfileType_Lock, info.pop[5].type);
		separatorIndex = info.pop[5].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("Mutex.c", info.pop[5].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsMutex_unlock", info.pop[5].function);
		EXPECT_NE(0U, info.pop[5].line);
	}

	EXPECT_EQ(dsProfileType_Lock, info.pop[6].type);
	separatorIndex = info.pop[6].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.pop[6].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_unlock", info.pop[6].function);
	EXPECT_NE(0U, info.pop[6].line);

	EXPECT_EQ(dsProfileType_Wait, info.pop[7].type);
	separatorIndex = info.pop[7].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Thread.c", info.pop[7].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsThread_join", info.pop[7].function);
	EXPECT_NE(0U, info.pop[7].line);

	EXPECT_EQ(dsProfileType_Wait, info.pop[8].type);
	separatorIndex = info.pop[8].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Thread.c", info.pop[8].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsThread_sleep", info.pop[8].function);
	EXPECT_NE(0U, info.pop[8].line);
}

TEST(Profile, ThreadTypesNamed)
{
	ProfileInfo info;
	dsProfileFunctions functions =
	{
		&profileRegisterThread, &profileStartFrame, &profileEndFrame, &profilePush, &profilePop,
		&profileStat, &profileGpu
	};
	dsProfile_setFunctions(&info, &functions);

	dsMutex* mutex = dsMutex_create(nullptr, "TestMutex");
	dsConditionVariable* condition = dsConditionVariable_create(nullptr, "TestCondition");

	EXPECT_TRUE(dsMutex_tryLock(mutex));
	EXPECT_TRUE(dsMutex_unlock(mutex));

	EXPECT_TRUE(dsMutex_lock(mutex));
	ThreadData threadData = {mutex, condition};
	dsThread thread;
	EXPECT_TRUE(dsThread_create(&thread, &threadFunc, &threadData, 0, "TestThread"));
	EXPECT_EQ(dsConditionVariableResult_Success, dsConditionVariable_wait(condition, mutex));
	EXPECT_TRUE(dsMutex_unlock(mutex));

	EXPECT_TRUE(dsThread_join(&thread, nullptr));
	dsThread_sleep(1, "TestSleep");

	dsMutex_destroy(mutex);
	dsConditionVariable_destroy(condition);

	dsProfile_clearFunctions();

	// push
	ASSERT_EQ(9U, info.push.size());
	EXPECT_EQ(dsProfileType_Lock, info.push[0].type);
	EXPECT_EQ("TestMutex", info.push[0].name);
	std::size_t separatorIndex = info.push[0].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.push[0].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_tryLock", info.push[0].function);
	EXPECT_NE(0U, info.push[0].line);
	EXPECT_TRUE(info.push[0].dynamic);

	EXPECT_EQ(dsProfileType_Lock, info.push[1].type);
	EXPECT_EQ("TestMutex", info.push[1].name);
	separatorIndex = info.push[1].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.push[1].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_lock", info.push[1].function);
	EXPECT_NE(0U, info.push[1].line);
	EXPECT_TRUE(info.push[1].dynamic);

	EXPECT_EQ(dsProfileType_Wait, info.push[2].type);
	EXPECT_EQ("TestMutex", info.push[2].name);
	separatorIndex = info.push[2].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.push[2].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_lock", info.push[2].function);
	EXPECT_NE(0U, info.push[2].line);
	EXPECT_TRUE(info.push[2].dynamic);

	if (info.push[3].name == "TestMutex")
	{
		EXPECT_EQ(dsProfileType_Lock, info.push[3].type);
		EXPECT_EQ("TestMutex", info.push[3].name);
		separatorIndex = info.push[3].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("Mutex.c", info.push[3].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsMutex_lock", info.push[3].function);
		EXPECT_NE(0U, info.push[3].line);
		EXPECT_TRUE(info.push[3].dynamic);

		if (info.push[4].name == "TestMutex")
		{
			EXPECT_EQ(dsProfileType_Wait, info.push[4].type);
			EXPECT_EQ("TestMutex", info.push[4].name);
			separatorIndex = info.push[4].file.find_last_of("/\\");
			ASSERT_NE(std::string::npos, separatorIndex);
			EXPECT_STRCASEEQ("Mutex.c", info.push[4].file.c_str() + separatorIndex + 1);
			EXPECT_EQ("dsMutex_lock", info.push[4].function);
			EXPECT_NE(0U, info.push[4].line);
			EXPECT_TRUE(info.push[4].dynamic);

			EXPECT_EQ(dsProfileType_Wait, info.push[5].type);
			EXPECT_EQ("TestCondition", info.push[5].name);
			separatorIndex = info.push[5].file.find_last_of("/\\");
			ASSERT_NE(std::string::npos, separatorIndex);
			EXPECT_STRCASEEQ("ConditionVariable.c", info.push[5].file.c_str() + separatorIndex + 1);
			EXPECT_EQ("dsConditionVariable_wait", info.push[5].function);
			EXPECT_NE(0U, info.push[5].line);
			EXPECT_TRUE(info.push[5].dynamic);
		}
		else
		{
			EXPECT_EQ(dsProfileType_Wait, info.push[4].type);
			EXPECT_EQ("TestCondition", info.push[4].name);
			separatorIndex = info.push[4].file.find_last_of("/\\");
			ASSERT_NE(std::string::npos, separatorIndex);
			EXPECT_STRCASEEQ("ConditionVariable.c", info.push[4].file.c_str() + separatorIndex + 1);
			EXPECT_EQ("dsConditionVariable_wait", info.push[4].function);
			EXPECT_NE(0U, info.push[4].line);
			EXPECT_TRUE(info.push[4].dynamic);

			EXPECT_EQ(dsProfileType_Wait, info.push[5].type);
			EXPECT_EQ("TestMutex", info.push[5].name);
			separatorIndex = info.push[5].file.find_last_of("/\\");
			ASSERT_NE(std::string::npos, separatorIndex);
			EXPECT_STRCASEEQ("Mutex.c", info.push[5].file.c_str() + separatorIndex + 1);
			EXPECT_EQ("dsMutex_lock", info.push[5].function);
			EXPECT_NE(0U, info.push[5].line);
			EXPECT_TRUE(info.push[5].dynamic);
		}
	}
	else
	{
		EXPECT_EQ(dsProfileType_Wait, info.push[3].type);
		EXPECT_EQ("TestCondition", info.push[3].name);
		separatorIndex = info.push[3].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("ConditionVariable.c", info.push[3].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsConditionVariable_wait", info.push[3].function);
		EXPECT_NE(0U, info.push[3].line);
		EXPECT_TRUE(info.push[3].dynamic);

		EXPECT_EQ(dsProfileType_Lock, info.push[4].type);
		EXPECT_EQ("TestMutex", info.push[4].name);
		separatorIndex = info.push[4].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("Mutex.c", info.push[4].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsMutex_lock", info.push[4].function);
		EXPECT_NE(0U, info.push[4].line);
		EXPECT_TRUE(info.push[4].dynamic);

		EXPECT_EQ(dsProfileType_Wait, info.push[5].type);
		EXPECT_EQ("TestMutex", info.push[5].name);
		separatorIndex = info.push[5].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("Mutex.c", info.push[5].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsMutex_lock", info.push[5].function);
		EXPECT_NE(0U, info.push[5].line);
		EXPECT_TRUE(info.push[5].dynamic);
	}

	EXPECT_EQ(dsProfileType_Lock, info.push[6].type);
	EXPECT_EQ("TestMutex", info.push[6].name);
	separatorIndex = info.push[6].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("ConditionVariable.c", info.push[6].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsConditionVariable_wait", info.push[6].function);
	EXPECT_NE(0U, info.push[6].line);
	EXPECT_TRUE(info.push[6].dynamic);

	EXPECT_EQ(dsProfileType_Wait, info.push[7].type);
	EXPECT_EQ("TestThread", info.push[7].name);
	separatorIndex = info.push[7].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Thread.c", info.push[7].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsThread_join", info.push[7].function);
	EXPECT_NE(0U, info.push[7].line);
	EXPECT_TRUE(info.push[7].dynamic);

	EXPECT_EQ(dsProfileType_Wait, info.push[8].type);
	EXPECT_EQ("TestSleep", info.push[8].name);
	separatorIndex = info.push[8].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Thread.c", info.push[8].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsThread_sleep", info.push[8].function);
	EXPECT_NE(0U, info.push[8].line);
	EXPECT_TRUE(info.push[8].dynamic);

	// pop
	ASSERT_EQ(9U, info.pop.size());
	EXPECT_EQ(dsProfileType_Lock, info.pop[0].type);
	separatorIndex = info.pop[0].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.pop[0].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_unlock", info.pop[0].function);
	EXPECT_NE(0U, info.pop[0].line);

	EXPECT_EQ(dsProfileType_Wait, info.pop[1].type);
	separatorIndex = info.pop[1].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.pop[1].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_lock", info.pop[1].function);
	EXPECT_NE(0U, info.pop[1].line);

	EXPECT_EQ(dsProfileType_Lock, info.pop[2].type);
	separatorIndex = info.pop[2].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("ConditionVariable.c", info.pop[2].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsConditionVariable_wait", info.pop[2].function);
	EXPECT_NE(0U, info.pop[2].line);

	EXPECT_EQ(dsProfileType_Wait, info.pop[3].type);
	separatorIndex = info.pop[3].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.pop[3].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_lock", info.pop[3].function);
	EXPECT_NE(0U, info.pop[3].line);

	if (info.pop[4].type == dsProfileType_Lock)
	{
		EXPECT_EQ(dsProfileType_Lock, info.pop[4].type);
		separatorIndex = info.pop[4].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("Mutex.c", info.pop[4].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsMutex_unlock", info.pop[4].function);
		EXPECT_NE(0U, info.pop[4].line);

		EXPECT_EQ(dsProfileType_Wait, info.pop[5].type);
		separatorIndex = info.pop[5].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("ConditionVariable.c", info.pop[5].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsConditionVariable_wait", info.pop[5].function);
		EXPECT_NE(0U, info.pop[5].line);
	}
	else
	{
		EXPECT_EQ(dsProfileType_Wait, info.pop[4].type);
		separatorIndex = info.pop[4].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("ConditionVariable.c", info.pop[4].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsConditionVariable_wait", info.pop[4].function);
		EXPECT_NE(0U, info.pop[4].line);

		EXPECT_EQ(dsProfileType_Lock, info.pop[5].type);
		separatorIndex = info.pop[5].file.find_last_of("/\\");
		ASSERT_NE(std::string::npos, separatorIndex);
		EXPECT_STRCASEEQ("Mutex.c", info.pop[5].file.c_str() + separatorIndex + 1);
		EXPECT_EQ("dsMutex_unlock", info.pop[5].function);
		EXPECT_NE(0U, info.pop[5].line);
	}

	EXPECT_EQ(dsProfileType_Lock, info.pop[6].type);
	separatorIndex = info.pop[6].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Mutex.c", info.pop[6].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsMutex_unlock", info.pop[6].function);
	EXPECT_NE(0U, info.pop[6].line);

	EXPECT_EQ(dsProfileType_Wait, info.pop[7].type);
	separatorIndex = info.pop[7].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Thread.c", info.pop[7].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsThread_join", info.pop[7].function);
	EXPECT_NE(0U, info.pop[7].line);

	EXPECT_EQ(dsProfileType_Wait, info.pop[8].type);
	separatorIndex = info.pop[8].file.find_last_of("/\\");
	ASSERT_NE(std::string::npos, separatorIndex);
	EXPECT_STRCASEEQ("Thread.c", info.pop[8].file.c_str() + separatorIndex + 1);
	EXPECT_EQ("dsThread_sleep", info.pop[8].function);
	EXPECT_NE(0U, info.pop[8].line);
}

#endif // DS_PROFILING_ENABLED
