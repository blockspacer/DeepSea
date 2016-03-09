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

#pragma once

#include <DeepSea/Core/Config.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used by the Memory portion of the DeepSea/Coroe library.
 */

/**
 * @brief Structure that defines a memory allocator.
 *
 * This can be "subclassed" by having it as the first member of other allocator structures. This can
 * be done to add additional data to the allocator and have it be freely casted between the
 * dsAllocator and the true allocator type.
 */
typedef struct dsAllocator dsAllocator;

/**
 * @brief Function for allocating from the allocator.
 *
 * The allocated memory must be at least 16-byte aligned.
 *
 * This should update the size for the allocator.
 *
 * @param allocator The allocator to allocate from.
 * @param size The size to allocate.
 * @return The allocated memory or NULL if an error occured.
 */
typedef void* (*dsAllocatorAllocFunction)(dsAllocator* allocator, size_t size);

/**
 * @brief Function for freeing memory from the allocator.
 *
 * This should update the size for the allocator.
 *
 * @param allocator The allocator to free from.
 * @param ptr The memory pointer to free.
 * @return True if the memory could be freed.
 */
typedef bool (*dsAllocatorFreeFunction)(dsAllocator* allocator, void* ptr);

/** @copydoc dsAllocator */
struct dsAllocator
{
	/**
	 * @brief The current size of allocated memory.
	 */
	size_t size;

	/**
	 * @brief The allocation function.
	 */
	dsAllocatorAllocFunction allocFunc;

	/**
	 * @brief The free function.
	 *
	 * If this function is NULL, then the memory allocated from allocFunc should not be freed. The
	 * allocator itself may also be temporary (such as with dsBufferAllocator) and pointers
	 * shouldn't be kept for later use.
	 */
	dsAllocatorFreeFunction freeFunc;
};

/**
 * @brief Structure for a system allocator.
 *
 * This is effectively a subclass of dsAllocator and a pointer to dsSystemAllocator can be freely
 * cast between the two types.
 */
typedef struct dsSystemAllocator
{
	/**
	 * @brief The general allocator.
	 */
	dsAllocator generalAllocator;
} dsSystemAllocator;

#ifdef __cplusplus
}
#endif
