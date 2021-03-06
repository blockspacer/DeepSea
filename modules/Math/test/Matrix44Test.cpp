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

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Math/Vector4.h>
#include <gtest/gtest.h>
#include <cmath>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct Matrix44TypeSelector;

template <>
struct Matrix44TypeSelector<float>
{
	typedef dsMatrix44f MatrixType;
	typedef dsVector4f Vector4Type;
	typedef dsVector3f Vector3Type;
	static const float epsilon;
	static const float inverseEpsilon;
};

template <>
struct Matrix44TypeSelector<double>
{
	typedef dsMatrix44d MatrixType;
	typedef dsVector4d Vector4Type;
	typedef dsVector3d Vector3Type;
	static const double epsilon;
	static const double inverseEpsilon;
};

const float Matrix44TypeSelector<float>::epsilon = 1e-5f;
const float Matrix44TypeSelector<float>::inverseEpsilon = 1e-3f;
const double Matrix44TypeSelector<double>::epsilon = 1e-13;
const double Matrix44TypeSelector<double>::inverseEpsilon = 1e-11;

template <typename T>
class Matrix44Test : public testing::Test
{
};

using Matrix44Types = testing::Types<float, double>;
TYPED_TEST_SUITE(Matrix44Test, Matrix44Types);

inline void dsMatrix44_affineInvert(dsMatrix44f* result, const dsMatrix44f* a)
{
	dsMatrix44f_affineInvert(result, a);
}

inline void dsMatrix44_affineInvert(dsMatrix44d* result, const dsMatrix44d* a)
{
	dsMatrix44d_affineInvert(result, a);
}

inline void dsMatrix44_invert(dsMatrix44f* result, const dsMatrix44f* a)
{
	dsMatrix44f_invert(result, a);
}

inline void dsMatrix44_invert(dsMatrix44d* result, const dsMatrix44d* a)
{
	dsMatrix44d_invert(result, a);
}

inline void dsMatrix44_inverseTranspose(dsMatrix44f* result, const dsMatrix44f* a)
{
	dsMatrix44f_inverseTranspose(result, a);
}

inline void dsMatrix44_inverseTranspose(dsMatrix44d* result, const dsMatrix44d* a)
{
	dsMatrix44d_inverseTranspose(result, a);
}

inline void dsMatrix44_makeRotate(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeRotate(result, x, y, z);
}

inline void dsMatrix44_makeRotate(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeRotate(result, x, y, z);
}

inline void dsMatrix44_makeRotateAxisAngle(dsMatrix44f* result, const dsVector3f* axis, float angle)
{
	dsMatrix44f_makeRotateAxisAngle(result, axis, angle);
}

inline void dsMatrix44_makeRotateAxisAngle(dsMatrix44d* result, const dsVector3d* axis, double angle)
{
	dsMatrix44d_makeRotateAxisAngle(result, axis, angle);
}

inline void dsMatrix44_makeTranslate(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeTranslate(result, x, y, z);
}

inline void dsMatrix44_makeTranslate(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeTranslate(result, x, y, z);
}

inline void dsMatrix44_makeScale(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeScale(result, x, y, z);
}

inline void dsMatrix44_makeScale(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeScale(result, x, y, z);
}

inline void dsMatrix44_lookAt(dsMatrix44f* result, const dsVector3f* eyePos,
	const dsVector3f* lookAtPos, const dsVector3f* upDir)
{
	dsMatrix44f_lookAt(result, eyePos, lookAtPos, upDir);
}

inline void dsMatrix44_lookAt(dsMatrix44d* result, const dsVector3d* eyePos,
	const dsVector3d* lookAtPos, const dsVector3d* upDir)
{
	dsMatrix44d_lookAt(result, eyePos, lookAtPos, upDir);
}

inline void dsMatrix44_makeOrtho(dsMatrix44f* result, float left, float right, float bottom,
	float top, float near, float far, bool halfDepth, bool invertY)
{
	dsMatrix44f_makeOrtho(result, left, right, bottom, top, near, far, halfDepth, invertY);
}

inline void dsMatrix44_makeOrtho(dsMatrix44d* result, double left, double right, double bottom,
	double top, double near, double far, bool halfDepth, bool invertY)
{
	dsMatrix44d_makeOrtho(result, left, right, bottom, top, near, far, halfDepth, invertY);
}

inline void dsMatrix44_makeFrustum(dsMatrix44f* result, float left, float right, float bottom,
	float top, float near, float far, bool halfDepth, bool invertY)
{
	dsMatrix44f_makeFrustum(result, left, right, bottom, top, near, far, halfDepth, invertY);
}

inline void dsMatrix44_makeFrustum(dsMatrix44d* result, double left, double right, double bottom,
	double top, double near, double far, bool halfDepth, bool invertY)
{
	dsMatrix44d_makeFrustum(result, left, right, bottom, top, near, far, halfDepth, invertY);
}

inline void dsMatrix44_makePerspective(dsMatrix44f* result, float fovy, float aspect,
	float near, float far, bool halfDepth, bool invertY)
{
	dsMatrix44f_makePerspective(result, fovy, aspect, near, far, halfDepth, invertY);
}

inline void dsMatrix44_makePerspective(dsMatrix44d* result, double fovy, double aspect,
	double near, double far, bool halfDepth, bool invertY)
{
	dsMatrix44d_makePerspective(result, fovy, aspect, near, far, halfDepth, invertY);
}

inline void dsVector3_normalize(dsVector3f* result, const dsVector3f* a)
{
	dsVector3f_normalize(result, a);
}

inline void dsVector3_normalize(dsVector3d* result, const dsVector3d* a)
{
	dsVector3d_normalize(result, a);
}

TYPED_TEST(Matrix44Test, Initialize)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;

	Matrix44Type matrix =
	{{
		{(TypeParam)-0.1, (TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7},
		{(TypeParam)8.9, (TypeParam)-0.1, (TypeParam)2.3, (TypeParam)-4.5},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)4.5, (TypeParam)-6.7, (TypeParam)-8.9, (TypeParam)0.1}
	}};

	EXPECT_EQ((TypeParam)-0.1, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)2.3, matrix.values[0][1]);
	EXPECT_EQ((TypeParam)-4.5, matrix.values[0][2]);
	EXPECT_EQ((TypeParam)6.7, matrix.values[0][3]);

	EXPECT_EQ((TypeParam)8.9, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)-0.1, matrix.values[1][1]);
	EXPECT_EQ((TypeParam)2.3, matrix.values[1][2]);
	EXPECT_EQ((TypeParam)-4.5, matrix.values[1][3]);

	EXPECT_EQ((TypeParam)-6.7, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)8.9, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)0.1, matrix.values[2][2]);
	EXPECT_EQ((TypeParam)-2.3, matrix.values[2][3]);

	EXPECT_EQ((TypeParam)4.5, matrix.values[3][0]);
	EXPECT_EQ((TypeParam)-6.7, matrix.values[3][1]);
	EXPECT_EQ((TypeParam)-8.9, matrix.values[3][2]);
	EXPECT_EQ((TypeParam)0.1, matrix.values[3][3]);

	EXPECT_EQ((TypeParam)-0.1, matrix.columns[0].values[0]);
	EXPECT_EQ((TypeParam)2.3, matrix.columns[0].values[1]);
	EXPECT_EQ((TypeParam)-4.5, matrix.columns[0].values[2]);
	EXPECT_EQ((TypeParam)6.7, matrix.columns[0].values[3]);

	EXPECT_EQ((TypeParam)8.9, matrix.columns[1].values[0]);
	EXPECT_EQ((TypeParam)-0.1, matrix.columns[1].values[1]);
	EXPECT_EQ((TypeParam)2.3, matrix.columns[1].values[2]);
	EXPECT_EQ((TypeParam)-4.5, matrix.columns[1].values[3]);

	EXPECT_EQ((TypeParam)-6.7, matrix.columns[2].values[0]);
	EXPECT_EQ((TypeParam)8.9, matrix.columns[2].values[1]);
	EXPECT_EQ((TypeParam)0.1, matrix.columns[2].values[2]);
	EXPECT_EQ((TypeParam)-2.3, matrix.columns[2].values[3]);

	EXPECT_EQ((TypeParam)4.5, matrix.columns[3].values[0]);
	EXPECT_EQ((TypeParam)-6.7, matrix.columns[3].values[1]);
	EXPECT_EQ((TypeParam)-8.9, matrix.columns[3].values[2]);
	EXPECT_EQ((TypeParam)0.1, matrix.columns[3].values[3]);
}

TYPED_TEST(Matrix44Test, Identity)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;

	Matrix44Type matrix;
	dsMatrix44_identity(matrix);

	EXPECT_EQ((TypeParam)1, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][2]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][3]);

	EXPECT_EQ((TypeParam)0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)1, matrix.values[1][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[1][2]);
	EXPECT_EQ((TypeParam)0, matrix.values[1][3]);

	EXPECT_EQ((TypeParam)0, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)1, matrix.values[2][2]);
	EXPECT_EQ((TypeParam)0, matrix.values[2][3]);

	EXPECT_EQ((TypeParam)0, matrix.values[3][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[3][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[3][2]);
	EXPECT_EQ((TypeParam)1, matrix.values[3][3]);
}

TYPED_TEST(Matrix44Test, Multiply)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Matrix44Type matrix1 =
	{{
		{(TypeParam)-0.1, (TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7},
		{(TypeParam)8.9, (TypeParam)-0.1, (TypeParam)2.3, (TypeParam)-4.5},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)4.5, (TypeParam)-6.7, (TypeParam)-8.9, (TypeParam)0.1}
	}};

	Matrix44Type matrix2 =
	{{
		{(TypeParam)1.0, (TypeParam)-3.2, (TypeParam)-5.4, (TypeParam)7.6},
		{(TypeParam)-9.8, (TypeParam)1.0, (TypeParam)-3.2, (TypeParam)5.4},
		{(TypeParam)7.6, (TypeParam)-9.8, (TypeParam)1.0, (TypeParam)-3.2},
		{(TypeParam)-5.4, (TypeParam)7.6, (TypeParam)9.8, (TypeParam)-1.0}
	}};

	Matrix44Type result;
	dsMatrix44_mul(result, matrix1, matrix2);

	EXPECT_NEAR((TypeParam)41.8, result.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)-96.36, result.values[0][1], epsilon);
	EXPECT_NEAR((TypeParam)-80.04, result.values[0][2], epsilon);
	EXPECT_NEAR((TypeParam)34.28, result.values[0][3], epsilon);

	EXPECT_NEAR((TypeParam)55.62, result.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)-87.3, result.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)-1.98, result.values[1][2], epsilon);
	EXPECT_NEAR((TypeParam)-62.26, result.values[1][3], epsilon);

	EXPECT_NEAR((TypeParam)-109.08, result.values[2][0], epsilon);
	EXPECT_NEAR((TypeParam)48.8, result.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)-28.16, result.values[2][2], epsilon);
	EXPECT_NEAR((TypeParam)92.4, result.values[2][3], epsilon);

	EXPECT_NEAR((TypeParam)-1.98, result.values[3][0], epsilon);
	EXPECT_NEAR((TypeParam)80.74, result.values[3][1], epsilon);
	EXPECT_NEAR((TypeParam)51.66, result.values[3][2], epsilon);
	EXPECT_NEAR((TypeParam)-93.02, result.values[3][3], epsilon);
}

TYPED_TEST(Matrix44Test, Transform)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	typedef typename Matrix44TypeSelector<TypeParam>::Vector4Type Vector4Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Matrix44Type matrix =
	{{
		{(TypeParam)-0.1, (TypeParam)8.9, (TypeParam)-6.7, (TypeParam)4.5},
		{(TypeParam)2.3, (TypeParam)-0.1, (TypeParam)8.9, (TypeParam)-6.7},
		{(TypeParam)-4.5, (TypeParam)2.3, (TypeParam)0.1, (TypeParam)-8.9},
		{(TypeParam)6.7, (TypeParam)-4.5, (TypeParam)-2.3, (TypeParam)0.1}
	}};

	Vector4Type vector = {{(TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6}};
	Vector4Type result;

	dsMatrix44_transform(result, matrix, vector);

	EXPECT_NEAR((TypeParam)82.68, result.values[0], epsilon);
	EXPECT_NEAR((TypeParam)-55.84, result.values[1], epsilon);
	EXPECT_NEAR((TypeParam)17.16, result.values[2], epsilon);
	EXPECT_NEAR((TypeParam)22.88, result.values[3], epsilon);
}

TYPED_TEST(Matrix44Test, TransformTransposed)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	typedef typename Matrix44TypeSelector<TypeParam>::Vector4Type Vector4Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Matrix44Type matrix =
	{{
		{(TypeParam)-0.1, (TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7},
		{(TypeParam)8.9, (TypeParam)-0.1, (TypeParam)2.3, (TypeParam)-4.5},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)4.5, (TypeParam)-6.7, (TypeParam)-8.9, (TypeParam)0.1}
	}};

	Vector4Type vector = {{(TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4, (TypeParam)7.6}};
	Vector4Type result;

	dsMatrix44_transformTransposed(result, matrix, vector);

	EXPECT_NEAR((TypeParam)82.68, result.values[0], epsilon);
	EXPECT_NEAR((TypeParam)-55.84, result.values[1], epsilon);
	EXPECT_NEAR((TypeParam)17.16, result.values[2], epsilon);
	EXPECT_NEAR((TypeParam)22.88, result.values[3], epsilon);
}

TYPED_TEST(Matrix44Test, Transpose)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;

	Matrix44Type matrix =
	{{
		{(TypeParam)-0.1, (TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7},
		{(TypeParam)8.9, (TypeParam)-0.1, (TypeParam)2.3, (TypeParam)-4.5},
		{(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)4.5, (TypeParam)-6.7, (TypeParam)-8.9, (TypeParam)0.1}
	}};

	Matrix44Type result;
	dsMatrix44_transpose(result, matrix);

	EXPECT_EQ((TypeParam)-0.1, result.values[0][0]);
	EXPECT_EQ((TypeParam)2.3, result.values[1][0]);
	EXPECT_EQ((TypeParam)-4.5, result.values[2][0]);
	EXPECT_EQ((TypeParam)6.7, result.values[3][0]);

	EXPECT_EQ((TypeParam)8.9, result.values[0][1]);
	EXPECT_EQ((TypeParam)-0.1, result.values[1][1]);
	EXPECT_EQ((TypeParam)2.3, result.values[2][1]);
	EXPECT_EQ((TypeParam)-4.5, result.values[3][1]);

	EXPECT_EQ((TypeParam)-6.7, result.values[0][2]);
	EXPECT_EQ((TypeParam)8.9, result.values[1][2]);
	EXPECT_EQ((TypeParam)0.1, result.values[2][2]);
	EXPECT_EQ((TypeParam)-2.3, result.values[3][2]);

	EXPECT_EQ((TypeParam)4.5, result.values[0][3]);
	EXPECT_EQ((TypeParam)-6.7, result.values[1][3]);
	EXPECT_EQ((TypeParam)-8.9, result.values[2][3]);
	EXPECT_EQ((TypeParam)0.1, result.values[3][3]);
}

TYPED_TEST(Matrix44Test, Determinant)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::inverseEpsilon;

	Matrix44Type matrix =
	{{
		{(TypeParam)-0.1, (TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7},
		{(TypeParam)8.9, (TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4},
		{(TypeParam)-7.6, (TypeParam)9.8, (TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)4.5, (TypeParam)-6.7, (TypeParam)-8.9, (TypeParam)1.0}
	}};

	EXPECT_NEAR((TypeParam)6163.7587, dsMatrix44_determinant(matrix), epsilon);
}

TYPED_TEST(Matrix44Test, Invert)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::inverseEpsilon;

	Matrix44Type matrix =
	{{
		{(TypeParam)-0.1, (TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7},
		{(TypeParam)8.9, (TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4},
		{(TypeParam)-7.6, (TypeParam)9.8, (TypeParam)0.1, (TypeParam)-2.3},
		{(TypeParam)4.5, (TypeParam)-6.7, (TypeParam)-8.9, (TypeParam)1.0}
	}};

	Matrix44Type inverse;
	dsMatrix44_invert(&inverse, &matrix);

	Matrix44Type result;
	dsMatrix44_mul(result, inverse, matrix);

	EXPECT_NEAR((TypeParam)0.08204279638656, inverse.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.105776528857303, inverse.values[0][1], epsilon);
	EXPECT_NEAR((TypeParam)-0.0109040608614341, inverse.values[0][2], epsilon);
	EXPECT_NEAR((TypeParam)-0.0035728199418310, inverse.values[0][3], epsilon);

	EXPECT_NEAR((TypeParam)0.089704841949766, inverse.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.07537365147017, inverse.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)0.076787723698529, inverse.values[1][2], epsilon);
	EXPECT_NEAR((TypeParam)-0.017392958617928, inverse.values[1][3], epsilon);

	EXPECT_NEAR((TypeParam)-0.01362918376412108, inverse.values[2][0], epsilon);
	EXPECT_NEAR((TypeParam)-0.00647819000442061, inverse.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)-0.071711600261055, inverse.values[2][2], epsilon);
	EXPECT_NEAR((TypeParam)-0.108603375404686, inverse.values[2][3], epsilon);

	EXPECT_NEAR((TypeParam)0.110530121823231, inverse.values[3][0], epsilon);
	EXPECT_NEAR((TypeParam)-0.028646806047096, inverse.values[3][1], epsilon);
	EXPECT_NEAR((TypeParam)-0.074687219666792, inverse.values[3][2], epsilon);
	EXPECT_NEAR((TypeParam)-0.067025174103588, inverse.values[3][3], epsilon);

	EXPECT_NEAR(1, result.values[0][0], epsilon);
	EXPECT_NEAR(0, result.values[0][1], epsilon);
	EXPECT_NEAR(0, result.values[0][2], epsilon);
	EXPECT_NEAR(0, result.values[0][3], epsilon);

	EXPECT_NEAR(0, result.values[1][0], epsilon);
	EXPECT_NEAR(1, result.values[1][1], epsilon);
	EXPECT_NEAR(0, result.values[1][2], epsilon);
	EXPECT_NEAR(0, result.values[1][3], epsilon);

	EXPECT_NEAR(0, result.values[2][0], epsilon);
	EXPECT_NEAR(0, result.values[2][1], epsilon);
	EXPECT_NEAR(1, result.values[2][2], epsilon);
	EXPECT_NEAR(0, result.values[2][3], epsilon);

	EXPECT_NEAR(0, result.values[3][0], epsilon);
	EXPECT_NEAR(0, result.values[3][1], epsilon);
	EXPECT_NEAR(0, result.values[3][2], epsilon);
	EXPECT_NEAR(1, result.values[3][3], epsilon);
}

TYPED_TEST(Matrix44Test, MakeRotate)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Matrix44Type rotateX;
	dsMatrix44_makeRotate(&rotateX, (TypeParam)dsDegreesToRadians(30), 0, 0);

	EXPECT_EQ((TypeParam)1, rotateX.values[0][0]);
	EXPECT_EQ((TypeParam)0, rotateX.values[0][1]);
	EXPECT_EQ((TypeParam)0, rotateX.values[0][2]);
	EXPECT_EQ((TypeParam)0, rotateX.values[0][3]);

	EXPECT_EQ((TypeParam)0, rotateX.values[1][0]);
	EXPECT_NEAR((TypeParam)0.866025403784439, rotateX.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)0.5, rotateX.values[1][2], epsilon);
	EXPECT_EQ((TypeParam)0, rotateX.values[1][3]);

	EXPECT_EQ((TypeParam)0, rotateX.values[2][0]);
	EXPECT_NEAR((TypeParam)-0.5, rotateX.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)0.866025403784439, rotateX.values[2][2], epsilon);
	EXPECT_EQ((TypeParam)0, rotateX.values[2][3]);

	EXPECT_EQ((TypeParam)0, rotateX.values[3][0]);
	EXPECT_EQ((TypeParam)0, rotateX.values[3][1]);
	EXPECT_EQ((TypeParam)0, rotateX.values[3][2]);
	EXPECT_EQ((TypeParam)1, rotateX.values[3][3]);

	Matrix44Type rotateY;
	dsMatrix44_makeRotate(&rotateY, 0, (TypeParam)dsDegreesToRadians(-15), 0);

	EXPECT_NEAR((TypeParam)0.9659258262890683, rotateY.values[0][0], epsilon);
	EXPECT_EQ((TypeParam)0, rotateY.values[0][1]);
	EXPECT_NEAR((TypeParam)0.2588190451025208, rotateY.values[0][2], epsilon);
	EXPECT_EQ((TypeParam)0, rotateY.values[0][3]);

	EXPECT_EQ((TypeParam)0, rotateY.values[1][0]);
	EXPECT_EQ((TypeParam)1, rotateY.values[1][1]);
	EXPECT_EQ((TypeParam)0, rotateY.values[1][2]);
	EXPECT_EQ((TypeParam)0, rotateY.values[1][3]);

	EXPECT_NEAR((TypeParam)-0.2588190451025208, rotateY.values[2][0], epsilon);
	EXPECT_EQ((TypeParam)0, rotateY.values[2][1]);
	EXPECT_NEAR((TypeParam)0.9659258262890683, rotateY.values[2][2], epsilon);
	EXPECT_EQ((TypeParam)0, rotateY.values[2][3]);

	EXPECT_EQ((TypeParam)0, rotateY.values[3][0]);
	EXPECT_EQ((TypeParam)0, rotateY.values[3][1]);
	EXPECT_EQ((TypeParam)0, rotateY.values[3][2]);
	EXPECT_EQ((TypeParam)1, rotateY.values[3][3]);

	Matrix44Type rotateZ;
	dsMatrix44_makeRotate(&rotateZ, 0, 0, (TypeParam)dsDegreesToRadians(60));

	EXPECT_NEAR((TypeParam)0.5, rotateZ.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.866025403784439, rotateZ.values[0][1], epsilon);
	EXPECT_EQ((TypeParam)0, rotateZ.values[0][2]);
	EXPECT_EQ((TypeParam)0, rotateZ.values[0][3]);

	EXPECT_NEAR((TypeParam)-0.866025403784439, rotateZ.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.5, rotateZ.values[1][1], epsilon);
	EXPECT_EQ((TypeParam)0, rotateZ.values[1][2]);
	EXPECT_EQ((TypeParam)0, rotateZ.values[1][3]);

	EXPECT_EQ((TypeParam)0, rotateZ.values[2][0]);
	EXPECT_EQ((TypeParam)0, rotateZ.values[2][1]);
	EXPECT_EQ((TypeParam)1, rotateZ.values[2][2]);
	EXPECT_EQ((TypeParam)0, rotateZ.values[2][3]);

	EXPECT_EQ((TypeParam)0, rotateZ.values[3][0]);
	EXPECT_EQ((TypeParam)0, rotateZ.values[3][1]);
	EXPECT_EQ((TypeParam)0, rotateZ.values[3][2]);
	EXPECT_EQ((TypeParam)1, rotateZ.values[3][3]);

	Matrix44Type temp, result;
	dsMatrix44_mul(temp, rotateY, rotateX);
	dsMatrix44_mul(result, rotateZ, temp);

	Matrix44Type rotateXYZ;
	dsMatrix44_makeRotate(&rotateXYZ, (TypeParam)dsDegreesToRadians(30),
		(TypeParam)dsDegreesToRadians(-15), (TypeParam)dsDegreesToRadians(60));

	EXPECT_NEAR(result.values[0][0], rotateXYZ.values[0][0], epsilon);
	EXPECT_NEAR(result.values[0][1], rotateXYZ.values[0][1], epsilon);
	EXPECT_NEAR(result.values[0][2], rotateXYZ.values[0][2], epsilon);
	EXPECT_NEAR(result.values[0][3], rotateXYZ.values[0][3], epsilon);

	EXPECT_NEAR(result.values[1][0], rotateXYZ.values[1][0], epsilon);
	EXPECT_NEAR(result.values[1][1], rotateXYZ.values[1][1], epsilon);
	EXPECT_NEAR(result.values[1][2], rotateXYZ.values[1][2], epsilon);
	EXPECT_NEAR(result.values[1][3], rotateXYZ.values[1][3], epsilon);

	EXPECT_NEAR(result.values[2][0], rotateXYZ.values[2][0], epsilon);
	EXPECT_NEAR(result.values[2][1], rotateXYZ.values[2][1], epsilon);
	EXPECT_NEAR(result.values[2][2], rotateXYZ.values[2][2], epsilon);
	EXPECT_NEAR(result.values[2][3], rotateXYZ.values[2][3], epsilon);

	EXPECT_NEAR(result.values[3][0], rotateXYZ.values[3][0], epsilon);
	EXPECT_NEAR(result.values[3][1], rotateXYZ.values[3][1], epsilon);
	EXPECT_NEAR(result.values[3][2], rotateXYZ.values[3][2], epsilon);
	EXPECT_NEAR(result.values[3][3], rotateXYZ.values[3][3], epsilon);
}

TYPED_TEST(Matrix44Test, MakeRotateAxisAngle)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	typedef typename Matrix44TypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Vector3Type axis = {{(TypeParam)-0.289967871131, (TypeParam)0.0171578621971,
		(TypeParam)0.51473586591302}};
	dsVector3_normalize(&axis, &axis);
	Matrix44Type matrix;
	dsMatrix44_makeRotateAxisAngle(&matrix, &axis,
		(TypeParam)dsDegreesToRadians(17.188733853924894));

	EXPECT_NEAR((TypeParam)0.96608673169969, matrix.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.25673182392846, matrix.values[0][1], epsilon);
	EXPECT_NEAR((TypeParam)-0.02766220194012, matrix.values[0][2], epsilon);
	EXPECT_EQ((TypeParam)0, matrix.values[0][3]);

	EXPECT_NEAR((TypeParam)-0.25800404198456, matrix.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.95537412871306, matrix.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)-0.14385474794174, matrix.values[1][2], epsilon);
	EXPECT_EQ((TypeParam)0, matrix.values[1][3]);

	EXPECT_NEAR((TypeParam)-0.01050433974302, matrix.values[2][0], epsilon);
	EXPECT_NEAR((TypeParam)0.14611312318926, matrix.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)0.98921211783846, matrix.values[2][2], epsilon);
	EXPECT_EQ((TypeParam)0, matrix.values[2][3]);

	EXPECT_EQ((TypeParam)0, matrix.values[3][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[3][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[3][2]);
	EXPECT_EQ((TypeParam)1, matrix.values[3][3]);
}

TYPED_TEST(Matrix44Test, MakeTranslate)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;

	Matrix44Type matrix;
	dsMatrix44_makeTranslate(&matrix, (TypeParam)1.2, (TypeParam)-3.4, (TypeParam)5.6);

	EXPECT_EQ((TypeParam)1, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][2]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][3]);

	EXPECT_EQ((TypeParam)0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)1, matrix.values[1][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[1][2]);
	EXPECT_EQ((TypeParam)0, matrix.values[1][3]);

	EXPECT_EQ((TypeParam)0, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)1, matrix.values[2][2]);
	EXPECT_EQ((TypeParam)0, matrix.values[2][3]);

	EXPECT_EQ((TypeParam)1.2, matrix.values[3][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[3][1]);
	EXPECT_EQ((TypeParam)5.6, matrix.values[3][2]);
	EXPECT_EQ((TypeParam)1, matrix.values[3][3]);
}

TYPED_TEST(Matrix44Test, MakeScale)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;

	Matrix44Type matrix;
	dsMatrix44_makeScale(&matrix, (TypeParam)1.2, (TypeParam)-3.4, (TypeParam)5.6);

	EXPECT_EQ((TypeParam)1.2, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][2]);
	EXPECT_EQ((TypeParam)0, matrix.values[0][3]);

	EXPECT_EQ((TypeParam)0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[1][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[1][2]);
	EXPECT_EQ((TypeParam)0, matrix.values[1][3]);

	EXPECT_EQ((TypeParam)0, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)5.6, matrix.values[2][2]);
	EXPECT_EQ((TypeParam)0, matrix.values[2][3]);

	EXPECT_EQ((TypeParam)0, matrix.values[3][0]);
	EXPECT_EQ((TypeParam)0, matrix.values[3][1]);
	EXPECT_EQ((TypeParam)0, matrix.values[3][2]);
	EXPECT_EQ((TypeParam)1, matrix.values[3][3]);
}

TYPED_TEST(Matrix44Test, LookAt)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	typedef typename Matrix44TypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Vector3Type eyePos = {{0, -1, 1}};
	Vector3Type lookAtPos = {{0, 0, 0}};
	Vector3Type upDir = {{0, 1, 0}};

	Matrix44Type matrix;
	dsMatrix44_lookAt(&matrix, &eyePos, &lookAtPos, &upDir);

	Matrix44Type rotation, translation, reference;
	dsMatrix44_makeRotate(&rotation, (TypeParam)dsDegreesToRadians(45), 0, 0);
	dsMatrix44_makeTranslate(&translation, eyePos.x, eyePos.y, eyePos.z);
	dsMatrix44_mul(reference, translation, rotation);

	EXPECT_NEAR(reference.values[0][0], matrix.values[0][0], epsilon);
	EXPECT_NEAR(reference.values[0][1], matrix.values[0][1], epsilon);
	EXPECT_NEAR(reference.values[0][2], matrix.values[0][2], epsilon);
	EXPECT_NEAR(reference.values[0][3], matrix.values[0][3], epsilon);

	EXPECT_NEAR(reference.values[1][0], matrix.values[1][0], epsilon);
	EXPECT_NEAR(reference.values[1][1], matrix.values[1][1], epsilon);
	EXPECT_NEAR(reference.values[1][2], matrix.values[1][2], epsilon);
	EXPECT_NEAR(reference.values[1][3], matrix.values[1][3], epsilon);

	EXPECT_NEAR(reference.values[2][0], matrix.values[2][0], epsilon);
	EXPECT_NEAR(reference.values[2][1], matrix.values[2][1], epsilon);
	EXPECT_NEAR(reference.values[2][2], matrix.values[2][2], epsilon);
	EXPECT_NEAR(reference.values[2][3], matrix.values[2][3], epsilon);

	EXPECT_NEAR(reference.values[3][0], matrix.values[3][0], epsilon);
	EXPECT_NEAR(reference.values[3][1], matrix.values[3][1], epsilon);
	EXPECT_NEAR(reference.values[3][2], matrix.values[3][2], epsilon);
	EXPECT_NEAR(reference.values[3][3], matrix.values[3][3], epsilon);
}

TYPED_TEST(Matrix44Test, FastInvert)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Matrix44Type rotate;
	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30),
		(TypeParam)dsDegreesToRadians(-15), (TypeParam)dsDegreesToRadians(60));

	Matrix44Type translate;
	dsMatrix44_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4, (TypeParam)5.6);

	Matrix44Type matrix;
	dsMatrix44_mul(matrix, translate, rotate);

	Matrix44Type inverse;
	dsMatrix44_fastInvert(inverse, matrix);

	Matrix44Type result;
	dsMatrix44_mul(result, inverse, matrix);

	EXPECT_NEAR(1, result.values[0][0], epsilon);
	EXPECT_NEAR(0, result.values[0][1], epsilon);
	EXPECT_NEAR(0, result.values[0][2], epsilon);
	EXPECT_NEAR(0, result.values[0][3], epsilon);

	EXPECT_NEAR(0, result.values[1][0], epsilon);
	EXPECT_NEAR(1, result.values[1][1], epsilon);
	EXPECT_NEAR(0, result.values[1][2], epsilon);
	EXPECT_NEAR(0, result.values[1][3], epsilon);

	EXPECT_NEAR(0, result.values[2][0], epsilon);
	EXPECT_NEAR(0, result.values[2][1], epsilon);
	EXPECT_NEAR(1, result.values[2][2], epsilon);
	EXPECT_NEAR(0, result.values[2][3], epsilon);

	EXPECT_NEAR(0, result.values[3][0], epsilon);
	EXPECT_NEAR(0, result.values[3][1], epsilon);
	EXPECT_NEAR(0, result.values[3][2], epsilon);
	EXPECT_NEAR(1, result.values[3][3], epsilon);
}

TYPED_TEST(Matrix44Test, AffineInvert)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Matrix44Type rotate;
	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30),
		(TypeParam)dsDegreesToRadians(-15), (TypeParam)dsDegreesToRadians(60));

	Matrix44Type translate;
	dsMatrix44_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4, (TypeParam)5.6);

	Matrix44Type scale;
	dsMatrix44_makeTranslate(&scale, (TypeParam)-2.1, (TypeParam)4.3, (TypeParam)-6.5);

	Matrix44Type temp;
	dsMatrix44_affineMul(temp, scale, rotate);

	Matrix44Type matrix;
	dsMatrix44_affineMul(matrix, translate, temp);

	Matrix44Type inverse;
	dsMatrix44_affineInvert(&inverse, &matrix);

	Matrix44Type result;
	dsMatrix44_affineMul(result, inverse, matrix);

	EXPECT_NEAR(1, result.values[0][0], epsilon);
	EXPECT_NEAR(0, result.values[0][1], epsilon);
	EXPECT_NEAR(0, result.values[0][2], epsilon);
	EXPECT_NEAR(0, result.values[0][3], epsilon);

	EXPECT_NEAR(0, result.values[1][0], epsilon);
	EXPECT_NEAR(1, result.values[1][1], epsilon);
	EXPECT_NEAR(0, result.values[1][2], epsilon);
	EXPECT_NEAR(0, result.values[1][3], epsilon);

	EXPECT_NEAR(0, result.values[2][0], epsilon);
	EXPECT_NEAR(0, result.values[2][1], epsilon);
	EXPECT_NEAR(1, result.values[2][2], epsilon);
	EXPECT_NEAR(0, result.values[2][3], epsilon);

	EXPECT_NEAR(0, result.values[3][0], epsilon);
	EXPECT_NEAR(0, result.values[3][1], epsilon);
	EXPECT_NEAR(0, result.values[3][2], epsilon);
	EXPECT_NEAR(1, result.values[3][3], epsilon);
}

TYPED_TEST(Matrix44Test, InverseTranspose)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Matrix44Type rotate;
	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30),
		(TypeParam)dsDegreesToRadians(-15), (TypeParam)dsDegreesToRadians(60));

	Matrix44Type translate;
	dsMatrix44_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4, (TypeParam)5.6);

	Matrix44Type scale;
	dsMatrix44_makeTranslate(&scale, (TypeParam)-2.1, (TypeParam)4.3, (TypeParam)-6.5);

	Matrix44Type temp;
	dsMatrix44_mul(temp, scale, rotate);

	Matrix44Type matrix;
	dsMatrix44_mul(matrix, translate, temp);

	Matrix44Type inverseTranspose;
	dsMatrix44_inverseTranspose(&inverseTranspose, &matrix);

	Matrix44Type inverse, inverseTransposeCheck;
	dsMatrix44_invert(&inverse, &matrix);
	dsMatrix44_transpose(inverseTransposeCheck, inverse);

	EXPECT_NEAR(inverseTransposeCheck.values[0][0], inverseTranspose.values[0][0], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[0][1], inverseTranspose.values[0][1], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[0][2], inverseTranspose.values[0][2], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[0][3], inverseTranspose.values[0][3], epsilon);

	EXPECT_NEAR(inverseTransposeCheck.values[1][0], inverseTranspose.values[1][0], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[1][1], inverseTranspose.values[1][1], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[1][2], inverseTranspose.values[1][2], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[1][3], inverseTranspose.values[1][3], epsilon);

	EXPECT_NEAR(inverseTransposeCheck.values[2][0], inverseTranspose.values[2][0], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[2][1], inverseTranspose.values[2][1], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[2][2], inverseTranspose.values[2][2], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[2][3], inverseTranspose.values[2][3], epsilon);

	EXPECT_NEAR(inverseTransposeCheck.values[3][0], inverseTranspose.values[3][0], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[3][1], inverseTranspose.values[3][1], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[3][2], inverseTranspose.values[3][2], epsilon);
	EXPECT_NEAR(inverseTransposeCheck.values[3][3], inverseTranspose.values[3][3], epsilon);
}

TYPED_TEST(Matrix44Test, MakeOrtho)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	typedef typename Matrix44TypeSelector<TypeParam>::Vector4Type Vector4Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Vector4Type minPoint = {{-2, -4, 6, 1}};
	Vector4Type maxPoint = {{3, 5, -7, 1}};

	Matrix44Type matrix;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, true, false);

	Vector4Type projPoint;
	dsMatrix44_transform(projPoint, matrix, minPoint);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(0, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, false, false);

	dsMatrix44_transform(projPoint, matrix, minPoint);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(-1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, true, true);

	dsMatrix44_transform(projPoint, matrix, minPoint);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(0, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, false, true);

	dsMatrix44_transform(projPoint, matrix, minPoint);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(-1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);
}

TYPED_TEST(Matrix44Test, MakeFrustum)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	typedef typename Matrix44TypeSelector<TypeParam>::Vector4Type Vector4Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	Vector4Type minPoint = {{-2, -4, -1, 1}};
	Vector4Type maxPoint = {{3*7, 5*7, -7, 1}};

	Matrix44Type matrix;
	dsMatrix44_makeFrustum(&matrix, -2, 3, -4, 5, 1, 7, true, false);

	Vector4Type projPoint;
	dsMatrix44_transform(projPoint, matrix, minPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(0, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makeFrustum(&matrix, -2, 3, -4, 5, 1, 7, false, false);

	dsMatrix44_transform(projPoint, matrix, minPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(-1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makeFrustum(&matrix, -2, 3, -4, 5, 1, 7, true, true);

	dsMatrix44_transform(projPoint, matrix, minPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(0, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makeFrustum(&matrix, -2, 3, -4, 5, 1, 7, false, true);

	dsMatrix44_transform(projPoint, matrix, minPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(-1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makeFrustum(&matrix, -2, 3, -4, 5, 1, INFINITY, true, false);
	EXPECT_EQ(TypeParam(-1), matrix.values[2][2]);
	EXPECT_EQ(TypeParam(-1), matrix.values[3][2]);

	dsMatrix44_makeFrustum(&matrix, -2, 3, -4, 5, 1, INFINITY, false, false);
	EXPECT_EQ(TypeParam(-1), matrix.values[2][2]);
	EXPECT_EQ(TypeParam(-2), matrix.values[3][2]);
}

TYPED_TEST(Matrix44Test, MakePerspective)
{
	typedef typename Matrix44TypeSelector<TypeParam>::MatrixType Matrix44Type;
	typedef typename Matrix44TypeSelector<TypeParam>::Vector4Type Vector4Type;
	TypeParam epsilon = Matrix44TypeSelector<TypeParam>::epsilon;

	TypeParam fov = (TypeParam)dsDegreesToRadians(30);
	TypeParam aspect = (TypeParam)1.5;
	TypeParam halfHeight = std::tan(fov/2);
	TypeParam halfWidth = aspect*halfHeight;

	Vector4Type minPoint = {{-halfWidth, -halfHeight, -1, 1}};
	Vector4Type maxPoint = {{halfWidth*7, halfHeight*7, -7, 1}};

	Matrix44Type matrix;
	dsMatrix44_makePerspective(&matrix, fov, aspect, 1, 7, true, false);

	Vector4Type projPoint;
	dsMatrix44_transform(projPoint, matrix, minPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(0, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makePerspective(&matrix, fov, aspect, 1, 7, false, false);

	dsMatrix44_transform(projPoint, matrix, minPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(-1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makePerspective(&matrix, fov, aspect, 1, 7, true, true);

	dsMatrix44_transform(projPoint, matrix, minPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(0, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makePerspective(&matrix, fov, aspect, 1, 7, false, true);

	dsMatrix44_transform(projPoint, matrix, minPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(-1, projPoint.x, epsilon);
	EXPECT_NEAR(1, projPoint.y, epsilon);
	EXPECT_NEAR(-1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_transform(projPoint, matrix, maxPoint);
	dsVector4_scale(projPoint, projPoint, 1/projPoint.w);
	EXPECT_NEAR(1, projPoint.x, epsilon);
	EXPECT_NEAR(-1, projPoint.y, epsilon);
	EXPECT_NEAR(1, projPoint.z, epsilon);
	EXPECT_NEAR(1, projPoint.w, epsilon);

	dsMatrix44_makePerspective(&matrix, fov, aspect, 1, INFINITY, true, false);
	EXPECT_EQ(TypeParam(-1), matrix.values[2][2]);
	EXPECT_EQ(TypeParam(-1), matrix.values[3][2]);

	dsMatrix44_makePerspective(&matrix, fov, aspect, 1, INFINITY, false, false);
	EXPECT_EQ(TypeParam(-1), matrix.values[2][2]);
	EXPECT_EQ(TypeParam(-2), matrix.values[3][2]);
}

TEST(Matrix44, ConvertFloatToDouble)
{
	dsMatrix44f matrixf =
	{{
		{-0.1f, 2.3f, -4.5f, 6.7f},
		{8.9f, -0.1f, 2.3f, -4.5f},
		{-6.7f, 8.9f, 0.1f, -2.3f},
		{4.5f, -6.7f, -8.9f, 0.1f}
	}};

	dsMatrix44d matrixd;
	dsConvertFloatToDouble(matrixd, matrixf);

	EXPECT_FLOAT_EQ(matrixf.values[0][0], (float)matrixd.values[0][0]);
	EXPECT_FLOAT_EQ(matrixf.values[0][1], (float)matrixd.values[0][1]);
	EXPECT_FLOAT_EQ(matrixf.values[0][2], (float)matrixd.values[0][2]);
	EXPECT_FLOAT_EQ(matrixf.values[0][3], (float)matrixd.values[0][3]);

	EXPECT_FLOAT_EQ(matrixf.values[1][0], (float)matrixd.values[1][0]);
	EXPECT_FLOAT_EQ(matrixf.values[1][1], (float)matrixd.values[1][1]);
	EXPECT_FLOAT_EQ(matrixf.values[1][2], (float)matrixd.values[1][2]);
	EXPECT_FLOAT_EQ(matrixf.values[1][3], (float)matrixd.values[1][3]);

	EXPECT_FLOAT_EQ(matrixf.values[2][0], (float)matrixd.values[2][0]);
	EXPECT_FLOAT_EQ(matrixf.values[2][1], (float)matrixd.values[2][1]);
	EXPECT_FLOAT_EQ(matrixf.values[2][2], (float)matrixd.values[2][2]);
	EXPECT_FLOAT_EQ(matrixf.values[2][3], (float)matrixd.values[2][3]);
}

TEST(Matrix44, ConvertDoubleToFloat)
{
	dsMatrix44d matrixd =
	{{
		{-0.1, 2.3, -4.5, 6.7},
		{8.9, -0.1, 2.3, -4.5},
		{-6.7, 8.9, 0.1, -2.3},
		{4.5, -6.7, -8.9, 0.1}
	}};

	dsMatrix44f matrixf;
	dsConvertDoubleToFloat(matrixf, matrixd);

	EXPECT_FLOAT_EQ((float)matrixd.values[0][0], matrixf.values[0][0]);
	EXPECT_FLOAT_EQ((float)matrixd.values[0][1], matrixf.values[0][1]);
	EXPECT_FLOAT_EQ((float)matrixd.values[0][2], matrixf.values[0][2]);
	EXPECT_FLOAT_EQ((float)matrixd.values[0][3], matrixf.values[0][3]);

	EXPECT_FLOAT_EQ((float)matrixd.values[1][0], matrixf.values[1][0]);
	EXPECT_FLOAT_EQ((float)matrixd.values[1][1], matrixf.values[1][1]);
	EXPECT_FLOAT_EQ((float)matrixd.values[1][2], matrixf.values[1][2]);
	EXPECT_FLOAT_EQ((float)matrixd.values[1][3], matrixf.values[1][3]);

	EXPECT_FLOAT_EQ((float)matrixd.values[2][0], matrixf.values[2][0]);
	EXPECT_FLOAT_EQ((float)matrixd.values[2][1], matrixf.values[2][1]);
	EXPECT_FLOAT_EQ((float)matrixd.values[2][2], matrixf.values[2][2]);
	EXPECT_FLOAT_EQ((float)matrixd.values[2][3], matrixf.values[2][3]);
}
