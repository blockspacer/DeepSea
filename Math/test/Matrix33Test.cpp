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

#include <DeepSea/Math/Matrix33.h>
#include <gtest/gtest.h>

template <typename T>
struct Matrix33TypeSelector;

template <>
struct Matrix33TypeSelector<float>
{
	typedef dsMatrix33f MatrixType;
	typedef dsVector3f VectorType;
	static const float epsilon;
};

template <>
struct Matrix33TypeSelector<double>
{
	typedef dsMatrix33d MatrixType;
	typedef dsVector3d VectorType;
	static const double epsilon;
};

const float Matrix33TypeSelector<float>::epsilon = 1e-4f;
const double Matrix33TypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class Matrix33Test : public testing::Test
{
};

using Matrix33Types = testing::Types<float, double>;
TYPED_TEST_CASE(Matrix33Test, Matrix33Types);

inline void dsMatrix33_affineInvert(dsMatrix33f* result, dsMatrix33f* a)
{
	dsMatrix33f_affineInvert(result, a);
}

inline void dsMatrix33_affineInvert(dsMatrix33d* result, dsMatrix33d* a)
{
	dsMatrix33d_affineInvert(result, a);
}

inline void dsMatrix33_invert(dsMatrix33f* result, dsMatrix33f* a)
{
	dsMatrix33f_invert(result, a);
}

inline void dsMatrix33_invert(dsMatrix33d* result, dsMatrix33d* a)
{
	dsMatrix33d_invert(result, a);
}

inline void dsMatrix33_makeRotate(dsMatrix33f* result, float angle)
{
	dsMatrix33f_makeRotate(result, angle);
}

inline void dsMatrix33_makeRotate(dsMatrix33d* result, double angle)
{
	dsMatrix33d_makeRotate(result, angle);
}

inline void dsMatrix33_makeTranslate(dsMatrix33f* result, float x, float y)
{
	dsMatrix33f_makeTranslate(result, x, y);
}

inline void dsMatrix33_makeTranslate(dsMatrix33d* result, double x, double y)
{
	dsMatrix33d_makeTranslate(result, x, y);
}

inline void dsMatrix33_makeScale(dsMatrix33f* result, float x, float y)
{
	dsMatrix33f_makeScale(result, x, y);
}

inline void dsMatrix33_makeScale(dsMatrix33d* result, double x, double y)
{
	dsMatrix33d_makeScale(result, x, y);
}

TYPED_TEST(Matrix33Test, Initialize)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix =
	{
		(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5,
		(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1,
		(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7
	};

	EXPECT_EQ((TypeParam)0.1, matrix.values[0][0]);
	EXPECT_EQ((TypeParam)-2.3, matrix.values[0][1]);
	EXPECT_EQ((TypeParam)4.5, matrix.values[0][2]);

	EXPECT_EQ((TypeParam)-6.7, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)8.9, matrix.values[1][1]);
	EXPECT_EQ((TypeParam)-0.1, matrix.values[1][2]);

	EXPECT_EQ((TypeParam)2.3, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)-4.5, matrix.values[2][1]);
	EXPECT_EQ((TypeParam)6.7, matrix.values[2][2]);

	EXPECT_EQ((TypeParam)0.1, matrix.columns[0].values[0]);
	EXPECT_EQ((TypeParam)-2.3, matrix.columns[0].values[1]);
	EXPECT_EQ((TypeParam)4.5, matrix.columns[0].values[2]);

	EXPECT_EQ((TypeParam)-6.7, matrix.columns[1].values[0]);
	EXPECT_EQ((TypeParam)8.9, matrix.columns[1].values[1]);
	EXPECT_EQ((TypeParam)-0.1, matrix.columns[1].values[2]);

	EXPECT_EQ((TypeParam)2.3, matrix.columns[2].values[0]);
	EXPECT_EQ((TypeParam)-4.5, matrix.columns[2].values[1]);
	EXPECT_EQ((TypeParam)6.7, matrix.columns[2].values[2]);
}

TYPED_TEST(Matrix33Test, Identity)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix;
	dsMatrix33_identity(matrix);

	EXPECT_EQ(1, matrix.values[0][0]);
	EXPECT_EQ(0, matrix.values[0][1]);
	EXPECT_EQ(0, matrix.values[0][2]);

	EXPECT_EQ(0, matrix.values[1][0]);
	EXPECT_EQ(1, matrix.values[1][1]);
	EXPECT_EQ(0, matrix.values[1][2]);

	EXPECT_EQ(0, matrix.values[2][0]);
	EXPECT_EQ(0, matrix.values[2][1]);
	EXPECT_EQ(1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33Test, Multiply)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix1 =
	{
		(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5,
		(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1,
		(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7
	};

	Matrix33Type matrix2 =
	{
		(TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4,
		(TypeParam)7.6, (TypeParam)-9.8, (TypeParam)1.0,
		(TypeParam)-3.2, (TypeParam)5.4, (TypeParam)-7.6
	};

	Matrix33Type result;
	dsMatrix33_mul(result, matrix1, matrix2);

	EXPECT_NEAR((TypeParam)-33.96, result.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)55.08, result.values[0][1], epsilon);
	EXPECT_NEAR((TypeParam)-41.0, result.values[0][2], epsilon);

	EXPECT_NEAR((TypeParam)68.72, result.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)-109.2, result.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)41.88, result.values[1][2], epsilon);

	EXPECT_NEAR((TypeParam)-53.98, result.values[2][0], epsilon);
	EXPECT_NEAR((TypeParam)89.62, result.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)-65.86, result.values[2][2], epsilon);
}

TYPED_TEST(Matrix33Test, Transform)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	typedef typename Matrix33TypeSelector<TypeParam>::VectorType Vector3Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix =
	{
		(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5,
		(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1,
		(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7
	};

	Vector3Type vector = {(TypeParam)-1.0, (TypeParam)3.2, (TypeParam)-5.4};
	Vector3Type result;

	dsMatrix33_transform(result, matrix, vector);

	EXPECT_NEAR((TypeParam)-31.76, result.values[0], epsilon);
	EXPECT_NEAR((TypeParam)35.72, result.values[1], epsilon);
	EXPECT_NEAR((TypeParam)-52.88, result.values[2], epsilon);
}

TYPED_TEST(Matrix33Test, Transpose)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix =
	{
		(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5,
		(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1,
		(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7
	};

	Matrix33Type result;
	dsMatrix33_transpose(result, matrix);

	EXPECT_EQ((TypeParam)0.1, result.values[0][0]);
	EXPECT_EQ((TypeParam)-2.3, result.values[1][0]);
	EXPECT_EQ((TypeParam)4.5, result.values[2][0]);

	EXPECT_EQ((TypeParam)-6.7, result.values[0][1]);
	EXPECT_EQ((TypeParam)8.9, result.values[1][1]);
	EXPECT_EQ((TypeParam)-0.1, result.values[2][1]);

	EXPECT_EQ((TypeParam)2.3, result.values[0][2]);
	EXPECT_EQ((TypeParam)-4.5, result.values[1][2]);
	EXPECT_EQ((TypeParam)6.7, result.values[2][2]);
}

TYPED_TEST(Matrix33Test, Determinant)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix =
	{
		(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5,
		(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1,
		(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7
	};

	EXPECT_NEAR((TypeParam)-53.24, dsMatrix33_determinant(matrix), epsilon);
}

TYPED_TEST(Matrix33Test, Invert)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix =
	{
		(TypeParam)0.1, (TypeParam)-2.3, (TypeParam)4.5,
		(TypeParam)-6.7, (TypeParam)8.9, (TypeParam)-0.1,
		(TypeParam)2.3, (TypeParam)-4.5, (TypeParam)6.7
	};

	Matrix33Type inverse;
	dsMatrix33_invert(&inverse, &matrix);

	Matrix33Type result;
	dsMatrix33_mul(result, inverse, matrix);

	EXPECT_NEAR((TypeParam)-1.11157024793389, inverse.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.090909090909, inverse.values[0][1], epsilon);
	EXPECT_NEAR((TypeParam)0.74793388429752, inverse.values[0][2], epsilon);

	EXPECT_NEAR((TypeParam)-0.83884297520661, inverse.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.181818181818182, inverse.values[1][1], epsilon);
	EXPECT_NEAR((TypeParam)0.56611570247934, inverse.values[1][2], epsilon);

	EXPECT_NEAR((TypeParam)-0.18181818181818, inverse.values[2][0], epsilon);
	EXPECT_NEAR((TypeParam)0.090909090909091, inverse.values[2][1], epsilon);
	EXPECT_NEAR((TypeParam)0.272727272727273, inverse.values[2][2], epsilon);

	EXPECT_NEAR(1, result.values[0][0], epsilon);
	EXPECT_NEAR(0, result.values[0][1], epsilon);
	EXPECT_NEAR(0, result.values[0][2], epsilon);

	EXPECT_NEAR(0, result.values[1][0], epsilon);
	EXPECT_NEAR(1, result.values[1][1], epsilon);
	EXPECT_NEAR(0, result.values[1][2], epsilon);

	EXPECT_NEAR(0, result.values[2][0], epsilon);
	EXPECT_NEAR(0, result.values[2][1], epsilon);
	EXPECT_NEAR(1, result.values[2][2], epsilon);
}

TYPED_TEST(Matrix33Test, MakeRotate)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type matrix;
	dsMatrix33_makeRotate(&matrix, (TypeParam)dsDegreesToRadians(30));

	EXPECT_NEAR((TypeParam)0.866025403784439, matrix.values[0][0], epsilon);
	EXPECT_NEAR((TypeParam)0.5, matrix.values[0][1], epsilon);
	EXPECT_EQ(0, matrix.values[0][2]);

	EXPECT_NEAR((TypeParam)-0.5, matrix.values[1][0], epsilon);
	EXPECT_NEAR((TypeParam)0.866025403784439, matrix.values[1][1], epsilon);
	EXPECT_EQ(0, matrix.values[1][2]);

	EXPECT_EQ(0, matrix.values[2][0]);
	EXPECT_EQ(0, matrix.values[2][1]);
	EXPECT_EQ(1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33Test, MakeTranslate)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix;
	dsMatrix33_makeTranslate(&matrix, (TypeParam)1.2, (TypeParam)-3.4);

	EXPECT_EQ(1, matrix.values[0][0]);
	EXPECT_EQ(0, matrix.values[0][1]);
	EXPECT_EQ(0, matrix.values[0][2]);

	EXPECT_EQ(0, matrix.values[1][0]);
	EXPECT_EQ(1, matrix.values[1][1]);
	EXPECT_EQ(0, matrix.values[1][2]);

	EXPECT_EQ((TypeParam)1.2, matrix.values[2][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[2][1]);
	EXPECT_EQ(1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33Test, MakeScale)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;

	Matrix33Type matrix;
	dsMatrix33_makeScale(&matrix, (TypeParam)1.2, (TypeParam)-3.4);

	EXPECT_EQ((TypeParam)1.2, matrix.values[0][0]);
	EXPECT_EQ(0, matrix.values[0][1]);
	EXPECT_EQ(0, matrix.values[0][2]);

	EXPECT_EQ(0, matrix.values[1][0]);
	EXPECT_EQ((TypeParam)-3.4, matrix.values[1][1]);
	EXPECT_EQ(0, matrix.values[1][2]);

	EXPECT_EQ(0, matrix.values[2][0]);
	EXPECT_EQ(0, matrix.values[2][1]);
	EXPECT_EQ(1, matrix.values[2][2]);
}

TYPED_TEST(Matrix33Test, FastInvert)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type rotate;
	dsMatrix33_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30));

	Matrix33Type translate;
	dsMatrix33_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4);

	Matrix33Type matrix;
	dsMatrix33_mul(matrix, translate, rotate);

	Matrix33Type inverse;
	dsMatrix33_fastInvert(inverse, matrix);

	Matrix33Type result;
	dsMatrix33_mul(result, inverse, matrix);

	EXPECT_NEAR(1, result.values[0][0], epsilon);
	EXPECT_NEAR(0, result.values[0][1], epsilon);
	EXPECT_NEAR(0, result.values[0][2], epsilon);

	EXPECT_NEAR(0, result.values[1][0], epsilon);
	EXPECT_NEAR(1, result.values[1][1], epsilon);
	EXPECT_NEAR(0, result.values[1][2], epsilon);

	EXPECT_NEAR(0, result.values[2][0], epsilon);
	EXPECT_NEAR(0, result.values[2][1], epsilon);
	EXPECT_NEAR(1, result.values[2][2], epsilon);
}

TYPED_TEST(Matrix33Test, AffineInvert)
{
	typedef typename Matrix33TypeSelector<TypeParam>::MatrixType Matrix33Type;
	TypeParam epsilon = Matrix33TypeSelector<TypeParam>::epsilon;

	Matrix33Type rotate;
	dsMatrix33_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30));

	Matrix33Type translate;
	dsMatrix33_makeTranslate(&translate, (TypeParam)1.2, (TypeParam)-3.4);

	Matrix33Type scale;
	dsMatrix33_makeTranslate(&scale, (TypeParam)-2.1, (TypeParam)4.3);

	Matrix33Type temp;
	dsMatrix33_mul(temp, scale, rotate);

	Matrix33Type matrix;
	dsMatrix33_mul(matrix, translate, temp);

	Matrix33Type inverse;
	dsMatrix33_affineInvert(&inverse, &matrix);

	Matrix33Type result;
	dsMatrix33_mul(result, inverse, matrix);

	EXPECT_NEAR(1, result.values[0][0], epsilon);
	EXPECT_NEAR(0, result.values[0][1], epsilon);
	EXPECT_NEAR(0, result.values[0][2], epsilon);

	EXPECT_NEAR(0, result.values[1][0], epsilon);
	EXPECT_NEAR(1, result.values[1][1], epsilon);
	EXPECT_NEAR(0, result.values[1][2], epsilon);

	EXPECT_NEAR(0, result.values[2][0], epsilon);
	EXPECT_NEAR(0, result.values[2][1], epsilon);
	EXPECT_NEAR(1, result.values[2][2], epsilon);
}
