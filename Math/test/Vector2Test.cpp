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

#include <DeepSea/Math/Vector2.h>
#include <gtest/gtest.h>
#include <cmath>

template <typename T>
struct Vector2TypeSelector;

template <>
struct Vector2TypeSelector<float> {typedef dsVector2f Type;};

template <>
struct Vector2TypeSelector<double> {typedef dsVector2d Type;};

template <>
struct Vector2TypeSelector<int> {typedef dsVector2i Type;};

template <typename T>
class Vector2Test : public testing::Test
{
};

using Vector2Types = testing::Types<float, double, int>;
TYPED_TEST_CASE(Vector2Test, Vector2Types);

template <typename T>
class Vector2FloatTest : public Vector2Test<T>
{
};

using Vector2FloatTypes = testing::Types<float, double>;
TYPED_TEST_CASE(Vector2FloatTest, Vector2FloatTypes);

inline float dsVector2_len(dsVector2f* a)
{
	return dsVector2f_len(a);
}

inline double dsVector2_len(dsVector2d* a)
{
	return dsVector2d_len(a);
}

inline double dsVector2_len(dsVector2i* a)
{
	return dsVector2i_len(a);
}

inline float dsVector2_dist(dsVector2f* a, dsVector2f* b)
{
	return dsVector2f_dist(a, b);
}

inline double dsVector2_dist(dsVector2d* a, dsVector2d* b)
{
	return dsVector2d_dist(a, b);
}

inline double dsVector2_dist(dsVector2i* a, dsVector2i* b)
{
	return dsVector2i_dist(a, b);
}

inline void dsVector2_normalize(dsVector2f* result, dsVector2f* a)
{
	dsVector2f_normalize(result, a);
}

inline void dsVector2_normalize(dsVector2d* result, dsVector2d* a)
{
	dsVector2d_normalize(result, a);
}

TYPED_TEST(Vector2Test, Initialize)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};

	EXPECT_EQ((TypeParam)-2.3, a.x);
	EXPECT_EQ((TypeParam)4.5, a.y);

	EXPECT_EQ((TypeParam)-2.3, a.s);
	EXPECT_EQ((TypeParam)4.5, a.t);

	EXPECT_EQ((TypeParam)-2.3, a.r);
	EXPECT_EQ((TypeParam)4.5, a.g);

	EXPECT_EQ((TypeParam)-2.3, a.values[0]);
	EXPECT_EQ((TypeParam)4.5, a.values[1]);
}

TYPED_TEST(Vector2Test, Add)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};
	Vector2Type result;

	dsVector2_add(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 + (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 + (TypeParam)-5.4, result.y);
}

TYPED_TEST(Vector2Test, Subtract)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};
	Vector2Type result;

	dsVector2_sub(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 - (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 - (TypeParam)-5.4, result.y);
}

TYPED_TEST(Vector2Test, Multiply)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};
	Vector2Type result;

	dsVector2_mul(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)-5.4, result.y);
}

TYPED_TEST(Vector2Test, Divide)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};
	Vector2Type result;

	dsVector2_div(result, a, b);
	EXPECT_EQ((TypeParam)-2.3 / (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 / (TypeParam)-5.4, result.y);
}

TYPED_TEST(Vector2Test, Scale)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type result;

	dsVector2_scale(result, a, (TypeParam)3.2);
	EXPECT_EQ((TypeParam)-2.3 * (TypeParam)3.2, result.x);
	EXPECT_EQ((TypeParam)4.5 * (TypeParam)3.2, result.y);
}

TYPED_TEST(Vector2Test, Neg)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type result;

	dsVector2_neg(result, a);
	EXPECT_EQ(-a.x, result.x);
	EXPECT_EQ(-a.y, result.y);
}

TYPED_TEST(Vector2Test, Dot)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};

	EXPECT_EQ((TypeParam)-2.3*(TypeParam)3.2 +
			  (TypeParam)4.5*(TypeParam)-5.4,
			  dsVector2_dot(a, b));
}

TYPED_TEST(Vector2Test, Length)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};

	EXPECT_EQ(dsPow2((TypeParam)-2.3) +
			  dsPow2((TypeParam)4.5),
			  dsVector2_len2(a));
	EXPECT_EQ(std::sqrt(dsPow2((TypeParam)-2.3) +
						dsPow2((TypeParam)4.5)),
			  dsVector2_len(&a));
}

TYPED_TEST(Vector2Test, Distance)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type b = {{(TypeParam)3.2, (TypeParam)-5.4}};

	EXPECT_EQ(dsPow2((TypeParam)-2.3 - (TypeParam)3.2) +
			  dsPow2((TypeParam)4.5 - (TypeParam)-5.4),
			  dsVector2_dist2(a, b));
	EXPECT_EQ(std::sqrt(dsPow2((TypeParam)-2.3 - (TypeParam)3.2) +
						dsPow2((TypeParam)4.5 - (TypeParam)-5.4)),
			  dsVector2_dist(&a, &b));
}

TYPED_TEST(Vector2FloatTest, Normalize)
{
	typedef typename Vector2TypeSelector<TypeParam>::Type Vector2Type;

	Vector2Type a = {{(TypeParam)-2.3, (TypeParam)4.5}};
	Vector2Type result;

	TypeParam length = dsVector2_len(&a);
	dsVector2_normalize(&result, &a);
	EXPECT_EQ((TypeParam)-2.3*(1/length), result.x);
	EXPECT_EQ((TypeParam)4.5*(1/length), result.y);
}

TEST(Vector2, ConvertFloatToDouble)
{
	dsVector2f vectorf = {{-2.3f, 4.5f}};

	dsVector2d vectord;
	dsConvertFloatToDouble(vectord, vectorf);

	EXPECT_FLOAT_EQ(vectorf.x, (float)vectord.x);
	EXPECT_FLOAT_EQ(vectorf.y, (float)vectord.y);
}

TEST(Vector2, ConvertDoubleToFloat)
{
	dsVector2d vectord = {{-2.3, 4.5}};

	dsVector2f vectorf;
	dsConvertDoubleToFloat(vectorf, vectord);

	EXPECT_FLOAT_EQ((float)vectord.x, vectorf.x);
	EXPECT_FLOAT_EQ((float)vectord.y, vectorf.y);
}

TEST(Vector2, ConvertFloatToInt)
{
	dsVector2f vectorf = {{-2, 3}};

	dsVector2i vectori;
	dsConvertFloatToInt(vectori, vectorf);

	EXPECT_EQ(vectorf.x, vectori.x);
	EXPECT_EQ(vectorf.y, vectori.y);
}

TEST(Vector2, ConvertIntToFloat)
{
	dsVector2i vectori = {{-2, 3}};

	dsVector2f vectorf;
	dsConvertIntToFloat(vectorf, vectori);

	EXPECT_EQ(vectori.x, vectorf.x);
	EXPECT_EQ(vectori.y, vectorf.y);
}

TEST(Vector2, ConvertDoubleToInt)
{
	dsVector2d vectord = {{-2, 3}};

	dsVector2i vectori;
	dsConvertDoubleToInt(vectori, vectord);

	EXPECT_EQ(vectord.x, vectori.x);
	EXPECT_EQ(vectord.y, vectori.y);
}

TEST(Vector2, ConvertIntToDouble)
{
	dsVector2i vectori = {{-2, 3}};

	dsVector2d vectord;
	dsConvertIntToDouble(vectord, vectori);

	EXPECT_EQ(vectori.x, vectord.x);
	EXPECT_EQ(vectori.y, vectord.y);
}
