/*
 * Copyright 2020 Aaron Barany
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

#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector4.h>

#define dsQuaternion4_fromEulerAnglesImpl(result, cosX, sinX, cosY, sinY, cosZ, sinZ) \
	do \
	{ \
		(result).values[0] = cosX*cosY*cosZ + sinX*sinY*sinZ; \
		(result).values[1] = sinX*cosY*cosZ - cosX*sinY*sinZ; \
		(result).values[2] = cosX*sinY*cosZ + sinX*cosY*sinZ; \
		(result).values[3] = cosX*cosY*sinZ - sinX*sinY*cosZ; \
	} \
	while (0)

#define dsQuaternion4_fromMatrixImpl(result, matrix, w, inv4w) \
	do \
	{ \
		(result).values[0] = w; \
		(result).values[1] = ((matrix).values[1][2] - (matrix).values[2][1])*inv4w; \
		(result).values[2] = ((matrix).values[2][0] - (matrix).values[0][2])*inv4w; \
		(result).values[3] = ((matrix).values[0][1] - (matrix).values[1][0])*inv4w; \
	} \
	while (0)

#define dsQuaternion4_toMatrixImpl(result, a) \
	do \
	{ \
		(result).values[0][0] = 1 - 2*(dsPow2((a).values[2]) + dsPow2((a).values[3])); \
		(result).values[0][1] = 2*((a).values[1]*(a).values[2] + (a).values[0]*(a).values[3]); \
		(result).values[0][2] = 2*((a).values[1]*(a).values[3] - (a).values[0]*(a).values[2]); \
		\
		(result).values[1][0] = 2*((a).values[1]*(a).values[2] - (a).values[0]*(a).values[3]); \
		(result).values[1][1] = 1 - 2*(dsPow2((a).values[1]) + dsPow2((a).values[3])); \
		(result).values[1][2] = 2*((a).values[0]*(a).values[1] + (a).values[2]*(a).values[3]); \
		\
		(result).values[2][0] = 2*((a).values[0]*(a).values[2] + (a).values[1]*(a).values[3]); \
		(result).values[2][1] = 2*((a).values[2]*(a).values[3] - (a).values[0]*(a).values[1]); \
		(result).values[2][2] = 1 - 2*(dsPow2((a).values[1]) + dsPow2((a).values[2])); \
	} \
	while (0)

void dsQuaternion4f_fromEulerAngles(dsQuaternion4f* result, float x, float y, float z)
{
	DS_ASSERT(result);

	float cosX = cosf(x*0.5f);
	float sinX = sinf(x*0.5f);
	float cosY = cosf(y*0.5f);
	float sinY = sinf(y*0.5f);
	float cosZ = cosf(z*0.5f);
	float sinZ = sinf(z*0.5f);

	dsQuaternion4_fromEulerAnglesImpl(*result, cosX, sinX, cosY, sinY, cosZ, sinZ);
}

void dsQuaternion4d_fromEulerAngles(dsQuaternion4d* result, double x, double y, double z)
{
	DS_ASSERT(result);

	double cosX = cos(x*0.5);
	double sinX = sin(x*0.5);
	double cosY = cos(y*0.5);
	double sinY = sin(y*0.5);
	double cosZ = cos(z*0.5);
	double sinZ = sin(z*0.5);

	dsQuaternion4_fromEulerAnglesImpl(*result, cosX, sinX, cosY, sinY, cosZ, sinZ);
}

void dsQuaternion4f_fromAxisAngle(dsQuaternion4f* result, const dsVector3f* axis,
	float angle)
{
	DS_ASSERT(result);
	DS_ASSERT(axis);

	float cosAngle = cosf(angle*0.5f);
	float sinAngle = sinf(angle*0.5f);

	result->r = cosAngle;
	result->i = axis->x*sinAngle;
	result->j = axis->y*sinAngle;
	result->k = axis->z*sinAngle;
}

void dsQuaternion4d_fromAxisAngle(dsQuaternion4d* result, const dsVector3d* axis,
	double angle)
{
	DS_ASSERT(result);
	DS_ASSERT(axis);

	double cosAngle = cos(angle*0.5);
	double sinAngle = sin(angle*0.5);

	result->r = cosAngle;
	result->i = axis->x*sinAngle;
	result->j = axis->y*sinAngle;
	result->k = axis->z*sinAngle;
}

void dsQuaternion4f_fromMatrix33(dsQuaternion4f* result, const dsMatrix33f* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	float w = sqrtf(1.0f + matrix->values[0][0] + matrix->values[1][1] + matrix->values[2][2])/2.0f;
	float inv4w = 1.0f/(4.0f*w);
	dsQuaternion4_fromMatrixImpl(*result, *matrix, w, inv4w);
}

void dsQuaternion4d_fromMatrix33(dsQuaternion4d* result, const dsMatrix33d* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	double w = sqrt(1.0 + matrix->values[0][0] + matrix->values[1][1] + matrix->values[2][2])/2.0;
	double inv4w = 1.0/(4.0*w);
	dsQuaternion4_fromMatrixImpl(*result, *matrix, w, inv4w);
}

void dsQuaternion4f_fromMatrix44(dsQuaternion4f* result, const dsMatrix44f* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	float w = sqrtf(1.0f + matrix->values[0][0] + matrix->values[1][1] + matrix->values[2][2])/2.0f;
	float inv4w = 1.0f/(4.0f*w);
	dsQuaternion4_fromMatrixImpl(*result, *matrix, w, inv4w);
}

void dsQuaternion4d_fromMatrix44(dsQuaternion4d* result, const dsMatrix44d* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	double w = sqrt(1.0 + matrix->values[0][0] + matrix->values[1][1] + matrix->values[2][2])/2.0;
	double inv4w = 1.0/(4.0*w);
	dsQuaternion4_fromMatrixImpl(*result, *matrix, w, inv4w);
}

void dsQuaternion4f_toMatrix33(dsMatrix33f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsQuaternion4_toMatrixImpl(*result, *a);
}

void dsQuaternion4d_toMatrix33(dsMatrix33d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsQuaternion4_toMatrixImpl(*result, *a);
}

void dsQuaternion4f_toMatrix44(dsMatrix44f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsQuaternion4_toMatrixImpl(*result, *a);
	result->values[0][3] = 0.0f;
	result->values[1][3] = 0.0f;
	result->values[2][3] = 0.0f;

	result->values[3][0] = 0.0f;
	result->values[3][1] = 0.0f;
	result->values[3][2] = 0.0f;
	result->values[3][3] = 1.0f;
}

void dsQuaternion4d_toMatrix44(dsMatrix44d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsQuaternion4_toMatrixImpl(*result, *a);
	result->values[0][3] = 0.0;
	result->values[1][3] = 0.0;
	result->values[2][3] = 0.0;

	result->values[3][0] = 0.0;
	result->values[3][1] = 0.0;
	result->values[3][2] = 0.0;
	result->values[3][3] = 1.0;
}

void dsQuaternion4f_slerp(dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b,
	float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	float cosAB = dsVector4_dot(*a, *b);

	// Make sure the shortest path is taken.
	dsQuaternion4f negB;
	if (cosAB < 0)
	{
		dsVector4_neg(negB, *b);
		b = &negB;
		cosAB = -cosAB;
	}

	// If too close, do a lerp instead.
	const float epsilon = 1e-6f;
	if (cosAB > (1.0f - epsilon))
	{
		dsVector4_lerp(*result, *a, *b, t);
		dsQuaternion4f_normalize(result, result);
		return;
	}

	float thetaAB = acosf(cosAB);
	float theta = thetaAB*t;
	float sinTheta = sinf(theta);
	float sinThetaAB = sinf(thetaAB);

	float scaleB = sinTheta/sinThetaAB;
	float scaleA = cosf(theta) - cosAB*scaleB;

	dsQuaternion4f scaledA, scaledB;
	dsVector4_scale(scaledA, *a, scaleA);
	dsVector4_scale(scaledB, *b, scaleB);
	dsVector4_add(*result, scaledA, scaledB);
}

void dsQuaternion4d_slerp(dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b,
	double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	double cosAB = dsVector4_dot(*a, *b);

	// Make sure the shortest path is taken.
	dsQuaternion4d negB;
	if (cosAB < 0)
	{
		dsVector4_neg(negB, *b);
		b = &negB;
		cosAB = -cosAB;
	}

	// If too close, do a lerp instead.
	const double epsilon = 1e-15f;
	if (cosAB > (1.0f - epsilon))
	{
		dsVector4_lerp(*result, *a, *b, t);
		dsQuaternion4d_normalize(result, result);
		return;
	}

	double thetaAB = acos(cosAB);
	double theta = thetaAB*t;
	double sinTheta = sin(theta);
	double sinThetaAB = sin(thetaAB);

	double scaleB = sinTheta/sinThetaAB;
	double scaleA = cos(theta) - cosAB*scaleB;

	dsQuaternion4d scaledA, scaledB;
	dsVector4_scale(scaledA, *a, scaleA);
	dsVector4_scale(scaledB, *b, scaleB);
	dsVector4_add(*result, scaledA, scaledB);
}

float dsQuaternion4f_getXAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getXAngle(const dsQuaternion4d* a);

float dsQuaternion4f_getYAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getYAngle(const dsQuaternion4d* a);

float dsQuaternion4f_getZAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getZAngle(const dsQuaternion4d* a);

void dsQuaternion4f_getRotationAxis(dsVector3f* result, const dsQuaternion4f* a);
void dsQuaternion4d_getRotationAxis(dsVector3d* result, const dsQuaternion4d* a);

float dsQuaternion4f_getAxisAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getAxisAngle(const dsQuaternion4d* a);

void dsQuaternion4f_normalize(dsQuaternion4f* result, const dsQuaternion4f* a);
void dsQuaternion4d_normalize(dsQuaternion4d* result, const dsQuaternion4d* a);

void dsQuaternion4f_rotate(dsVector3f* result, const dsQuaternion4f* a, const dsVector3f* v);
void dsQuaternion4d_rotate(dsVector3d* result, const dsQuaternion4d* a, const dsVector3d* v);

void dsQuaternion4f_mul(dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b);
void dsQuaternion4d_mul(dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b);

void dsQuaternion4f_invert(dsQuaternion4f* result, const dsQuaternion4f* a);
void dsQuaternion4d_invert(dsQuaternion4d* result, const dsQuaternion4d* a);
