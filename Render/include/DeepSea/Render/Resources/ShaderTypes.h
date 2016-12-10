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
#include <DeepSea/Core/Memory/Types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Enum for the type of a material member.
 */
typedef enum dsMaterialType
{
	// Scalars and vectors
	dsMaterialType_Float,  ///< float
	dsMaterialType_Vec2,   ///< vec2
	dsMaterialType_Vec3,   ///< vec3
	dsMaterialType_Vec4,   ///< vec4
	dsMaterialType_Double, ///< double
	dsMaterialType_DVec2,  ///< dvec2
	dsMaterialType_DVec3,  ///< dvec3
	dsMaterialType_DVec4,  ///< dvec4
	dsMaterialType_Int,    ///< int
	dsMaterialType_IVec2,  ///< ivec2
	dsMaterialType_IVec3,  ///< ivec3
	dsMaterialType_IVec4,  ///< ivec4
	dsMaterialType_UInt,   ///< unsigned int
	dsMaterialType_UVec2,  ///< uvec2
	dsMaterialType_UVec3,  ///< uvec3
	dsMaterialType_UVec4,  ///< uvec4
	dsMaterialType_Bool,   ///< bool
	dsMaterialType_BVec2,  ///< bvec2
	dsMaterialType_BVec3,  ///< bvec3
	dsMaterialType_BVec4,  ///< bvec4

	// Matrices
	dsMaterialType_Mat2,    ///< mat2, mat2x2
	dsMaterialType_Mat3,    ///< mat3, mat3x3
	dsMaterialType_Mat4,    ///< mat4, mat4x4
	dsMaterialType_Mat2x3,  ///< mat2x3
	dsMaterialType_Mat2x4,  ///< mat2x4
	dsMaterialType_Mat3x2,  ///< mat3x2
	dsMaterialType_Mat3x4,  ///< mat3x4
	dsMaterialType_Mat4x2,  ///< mat4x2
	dsMaterialType_Mat4x3,  ///< mat4x3
	dsMaterialType_DMat2,   ///< dmat2, dmat2x2
	dsMaterialType_DMat3,   ///< dmat3, dmat3x3
	dsMaterialType_DMat4,   ///< dmat4, dmat4x4
	dsMaterialType_DMat2x3, ///< dmat2x3
	dsMaterialType_DMat2x4, ///< dmat2x4
	dsMaterialType_DMat3x2, ///< dmat3x2
	dsMaterialType_DMat3x4, ///< dmat3x4
	dsMaterialType_DMat4x2, ///< dmat4x2
	dsMaterialType_DMat4x3, ///< dmat4x3

	// Other types
	dsMaterialType_Texture,       ///< Sampled texture.
	dsMaterialType_Image,         ///< Unsampled image texture.
	dsMaterialType_VariableGroup, ///< Group of variables from dsShaderVariableGroup.
	dsMaterialType_UniformBlock,  ///< Graphics buffer bound as a shader block.
	dsMaterialType_UniformBuffer, ///< Graphics buffer bound as a shader buffer.
} dsMaterialType;

/// \{
typedef struct dsResourceManager dsResourceManager;
typedef struct mslModule mslModule;
typedef struct mslPipeline mslPipeline;
/// \}

/**
 * @brief Struct for a shader module.
 *
 * A shader module contains the data for multiple shaders. Shaders may then be loaded from the
 * module to render with.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 */
typedef struct dsShaderModule
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The underlying module data.
	 *
	 * This is accessed with the ModularShaderLanguage library.
	 */
	mslModule* module;
} dsShaderModule;

/**
 * @brief Struct for a shader.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 */
typedef struct dsShader
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The module this shader belongs to.
	 */
	dsShaderModule* module;

	/**
	 * @brief The index of the shader pipeline.
	 */
	uint32_t pipelineIndex;

	/**
	 * @brief The pipeline for the shader.
	 *
	 * This is accessed with the ModularShaderLanguage library.
	 */
	mslPipeline* pipeline;
} dsShader;

/**
 * @brief Struct describing an element of a material.
 */
typedef struct dsMaterialElement
{
	/**
	 * @brief The name of the element.
	 */
	const char* name;

	/**
	 * @brief The type of the element.
	 */
	dsMaterialType type;

	/**
	 * @brief The number of array elements. This must be at least 1.
	 */
	uint32_t count;
} dsMaterialElement;

/**
 * @brief Struct holding a description of a material.
 *
 * This is provided when creating a shader to aid in assigning material values. It is also
 * used to create material instances.
 *
 * The same material description may be used with multiple shaders, so long as all of the uniforms
 * of the shader are provided. It is still valid if extra material parameters are avoided.
 *
 * It is encouraged to re-use the same material description for multiple shaders when they use
 * similar parameters. This allows materials to be shared across those shaders and may make
 * rendering more efficient.
 *
 * Render implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsResourceManager and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 */
typedef struct dsMaterialDesc
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The number of material elements.
	 */
	uint32_t elementCount;

	/**
	 * @brief The material elements.
	 */
	dsMaterialElement* elements;
} dsMaterialDesc;

/**
 * @brief Struct defining a material to be applied to shaders.
 *
 * Material instances are created with a dsMaterialDesc instance to describe the variables that are
 * set. The values set on this structure will be used to populate the uniforms of a shader.
 *
 * This type is opaque and implemented by the core Render library.
 */
typedef struct dsMaterial dsMaterial;

/**
 * @brief Struct describing the position of a shader variable in the final buffer.
 *
 * This is specified by the implementation to be used in the core Render library.
 */
typedef struct dsShaderVariablePos
{
	/**
	 * @brief The offset of the variable in the buffer.
	 */
	uint32_t offset;

	/**
	 * @brief The stride of each array element.
	 */
	uint32_t stride;
} dsShaderVariablePos;

/**
 * @brief Struct holding a description of a shader variable group.
 *
 * This is very similar to dsMaterialDesc, but is used for dsShaderVariableGroup. When shader
 * buffers are supported, the implementation should populate the offsets array.
 *
 * @remark None of the members should be modified outside of the implementation.
 */
typedef struct dsShaderVariableGroupDesc
{
	/**
	 * @brief The resource manager this was created with.
	 */
	dsResourceManager* resourceManager;

	/**
	 * @brief The allocator this was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The number of material elements.
	 */
	uint32_t elementCount;

	/**
	 * @brief The material elements.
	 */
	dsMaterialElement* elements;

	/**
	 * @brief The position for the elements.
	 *
	 * This is only necessary when shader buffers are supported.
	 */
	dsShaderVariablePos* positions;
} dsShaderVariableGroupDesc;

/**
 * @brief Struct holding a group of shader variables.

 * This allows conditional usage of buffers for shader blocks. When shader blocks are supported,
 * a graphics buffer will be used internally.
 *
 * This type is opaque and implemented by the core Render library.
 */
typedef struct dsShaderVariableGroup dsShaderVariableGroup;

#ifdef __cplusplus
}
#endif