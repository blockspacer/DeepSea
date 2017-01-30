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

#include "MaterialInfo.h"
#include <DeepSea/Core/Assert.h>

uint16_t dsMaterialType_size(dsMaterialType type)
{
	static const uint16_t size[] =
	{
		// Scalars and vectors
		sizeof(float),        // dsMaterialType_Float
		sizeof(dsVector2f),   // dsMaterialType_Vec2
		sizeof(dsVector3f),   // dsMaterialType_Vec3
		sizeof(dsVector4f),   // dsMaterialType_Vec4
		sizeof(double),       // dsMaterialType_Double
		sizeof(dsVector2d),   // dsMaterialType_DVec2
		sizeof(dsVector3d),   // dsMaterialType_DVec3
		sizeof(dsVector4d),   // dsMaterialType_DVec4
		sizeof(int),          // dsMaterialType_Int
		sizeof(dsVector2i),   // dsMaterialType_IVec2
		sizeof(dsVector3i),   // dsMaterialType_IVec3
		sizeof(dsVector4i),   // dsMaterialType_IVec4
		sizeof(unsigned int), // dsMaterialType_UInt
		sizeof(dsVector2i),   // dsMaterialType_UVec2
		sizeof(dsVector3i),   // dsMaterialType_UVec3
		sizeof(dsVector4i),   // dsMaterialType_UVec4
		sizeof(int),          // dsMaterialType_Bool
		sizeof(dsVector2i),   // dsMaterialType_BVec2
		sizeof(dsVector3i),   // dsMaterialType_BVec3
		sizeof(dsVector4i),   // dsMaterialType_BVec4

		// Matrices
		sizeof(float)*2*2,  // dsMaterialType_Mat2
		sizeof(float)*3*3,  // dsMaterialType_Mat3
		sizeof(float)*4*4,  // dsMaterialType_Mat4
		sizeof(float)*2*3,  // dsMaterialType_Mat2x3
		sizeof(float)*2*4,  // dsMaterialType_Mat2x4
		sizeof(float)*3*2,  // dsMaterialType_Mat3x2
		sizeof(float)*3*4,  // dsMaterialType_Mat3x4
		sizeof(float)*4*2,  // dsMaterialType_Mat4x2
		sizeof(float)*4*3,  // dsMaterialType_Mat4x3
		sizeof(double)*2*2, // dsMaterialType_DMat2
		sizeof(double)*3*3, // dsMaterialType_DMat3
		sizeof(double)*4*4, // dsMaterialType_DMat4
		sizeof(double)*2*3, // dsMaterialType_DMat2x3
		sizeof(double)*2*4, // dsMaterialType_DMat2x4
		sizeof(double)*3*2, // dsMaterialType_DMat3x2
		sizeof(double)*3*4, // dsMaterialType_DMat3x4
		sizeof(double)*4*2, // dsMaterialType_DMat4x2
		sizeof(double)*4*3, // dsMaterialType_DMat4x3

		// Other types
		sizeof(void*), // dsMaterialType_Texture
		sizeof(void*), // dsMaterialType_Image
		sizeof(void*), // dsMaterialType_VariableGroup
		sizeof(void*), // dsMaterialType_UniformBlock
		sizeof(void*), // dsMaterialType_UniformBuffer
	};

	DS_STATIC_ASSERT(DS_ARRAY_SIZE(size) == dsMaterialType_Count, array_enum_mismatch);
	DS_ASSERT(type >= 0 && type < dsMaterialType_Count);
	return size[type];
}

uint16_t dsMaterialType_machineAlignment(dsMaterialType type)
{
	static const uint16_t alignment[] =
	{
		// Scalars and vectors
		sizeof(float),        // dsMaterialType_Float
		sizeof(float),        // dsMaterialType_Vec2
		sizeof(float),        // dsMaterialType_Vec3
		sizeof(float),        // dsMaterialType_Vec4
		sizeof(double),       // dsMaterialType_Double
		sizeof(double),       // dsMaterialType_DVec2
		sizeof(double),       // dsMaterialType_DVec3
		sizeof(double),       // dsMaterialType_DVec4
		sizeof(int),          // dsMaterialType_Int
		sizeof(int),          // dsMaterialType_IVec2
		sizeof(int),          // dsMaterialType_IVec3
		sizeof(int),          // dsMaterialType_IVec4
		sizeof(unsigned int), // dsMaterialType_UInt
		sizeof(unsigned int), // dsMaterialType_UVec2
		sizeof(unsigned int), // dsMaterialType_UVec3
		sizeof(unsigned int), // dsMaterialType_UVec4
		sizeof(int),          // dsMaterialType_Bool
		sizeof(int),          // dsMaterialType_BVec2
		sizeof(int),          // dsMaterialType_BVec3
		sizeof(int),          // dsMaterialType_BVec4

		// Matrices
		sizeof(float),  // dsMaterialType_Mat2
		sizeof(float),  // dsMaterialType_Mat3
		sizeof(float),  // dsMaterialType_Mat4
		sizeof(float),  // dsMaterialType_Mat2x3
		sizeof(float),  // dsMaterialType_Mat2x4
		sizeof(float),  // dsMaterialType_Mat3x2
		sizeof(float),  // dsMaterialType_Mat3x4
		sizeof(float),  // dsMaterialType_Mat4x2
		sizeof(float),  // dsMaterialType_Mat4x3
		sizeof(double), // dsMaterialType_DMat2
		sizeof(double), // dsMaterialType_DMat3
		sizeof(double), // dsMaterialType_DMat4
		sizeof(double), // dsMaterialType_DMat2x3
		sizeof(double), // dsMaterialType_DMat2x4
		sizeof(double), // dsMaterialType_DMat3x2
		sizeof(double), // dsMaterialType_DMat3x4
		sizeof(double), // dsMaterialType_DMat4x2
		sizeof(double), // dsMaterialType_DMat4x3

		// Other types
		sizeof(void*), // dsMaterialType_Texture
		sizeof(void*), // dsMaterialType_Image
		sizeof(void*), // dsMaterialType_VariableGroup
		sizeof(void*), // dsMaterialType_UniformBlock
		sizeof(void*), // dsMaterialType_UniformBuffer
	};

	DS_STATIC_ASSERT(DS_ARRAY_SIZE(alignment) == dsMaterialType_Count, array_enum_mismatch);
	DS_ASSERT(type >= 0 && type < dsMaterialType_Count);
	return alignment[type];
}

unsigned int dsMaterialType_matrixRows(dsMaterialType type)
{
	if (type < dsMaterialType_Mat2 || type > dsMaterialType_DMat4x3)
		return 0;

	static const unsigned int rows[] =
	{
		2, // dsMaterialType_Mat2
		3, // dsMaterialType_Mat3
		4, // dsMaterialType_Mat4
		3, // dsMaterialType_Mat2x3
		4, // dsMaterialType_Mat2x4
		2, // dsMaterialType_Mat3x2
		4, // dsMaterialType_Mat3x4
		2, // dsMaterialType_Mat4x2
		3, // dsMaterialType_Mat4x3
		2, // dsMaterialType_DMat2
		3, // dsMaterialType_DMat3
		4, // dsMaterialType_DMat4
		3, // dsMaterialType_DMat2x3
		4, // dsMaterialType_DMat2x4
		2, // dsMaterialType_DMat3x2
		4, // dsMaterialType_DMat3x4
		2, // dsMaterialType_DMat4x2
		3, // dsMaterialType_DMat4x3
	};
	DS_ASSERT(type - dsMaterialType_Mat2 < DS_ARRAY_SIZE(rows));
	return rows[type - dsMaterialType_Mat2];
}

unsigned int dsMaterialType_matrixColumns(dsMaterialType type)
{
	if (type < dsMaterialType_Mat2 || type > dsMaterialType_DMat4x3)
		return 0;

	static const unsigned int columns[] =
	{
		2, // dsMaterialType_Mat2
		3, // dsMaterialType_Mat3
		4, // dsMaterialType_Mat4
		2, // dsMaterialType_Mat2x3
		2, // dsMaterialType_Mat2x4
		3, // dsMaterialType_Mat3x2
		3, // dsMaterialType_Mat3x4
		4, // dsMaterialType_Mat4x2
		4, // dsMaterialType_Mat4x3
		2, // dsMaterialType_DMat2
		3, // dsMaterialType_DMat3
		4, // dsMaterialType_DMat4
		2, // dsMaterialType_DMat2x3
		2, // dsMaterialType_DMat2x4
		3, // dsMaterialType_DMat3x2
		3, // dsMaterialType_DMat3x4
		4, // dsMaterialType_DMat4x2
		4, // dsMaterialType_DMat4x3
	};
	DS_ASSERT(type - dsMaterialType_Mat2 < DS_ARRAY_SIZE(columns));
	return columns[type - dsMaterialType_Mat2];
}