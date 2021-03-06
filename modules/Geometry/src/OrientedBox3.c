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

#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>
#include <float.h>

bool dsOrientedBox3f_transform(dsOrientedBox3f* box, const dsMatrix44f* transform)
{
	DS_ASSERT(box);
	DS_ASSERT(transform);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	dsVector4f center = {{box->center.x, box->center.y, box->center.z, 1}};

	dsMatrix33f newOrientation;
	dsVector4f newCenter;

	// Macro will work correctly for treating a 4x4 matrix as a 3x3 matrix.
	dsMatrix33_mul(newOrientation, *transform, box->orientation);

	// Extract scales to apply to the extent.
	float scaleX = dsVector3f_len(&newOrientation.columns[0]);
	float scaleY = dsVector3f_len(&newOrientation.columns[1]);
	float scaleZ = dsVector3f_len(&newOrientation.columns[2]);

	dsMatrix44_transform(newCenter, *transform, center);

	float invScaleX = 1/scaleX;
	float invScaleY = 1/scaleY;
	float invScaleZ = 1/scaleZ;
	dsVector3_scale(box->orientation.columns[0], newOrientation.columns[0], invScaleX);
	dsVector3_scale(box->orientation.columns[1], newOrientation.columns[1], invScaleY);
	dsVector3_scale(box->orientation.columns[2], newOrientation.columns[2], invScaleZ);

	box->center.x = newCenter.x;
	box->center.y = newCenter.y;
	box->center.z = newCenter.z;

	box->halfExtents.x *= scaleX;
	box->halfExtents.y *= scaleY;
	box->halfExtents.z *= scaleZ;

	return true;
}

bool dsOrientedBox3d_transform(dsOrientedBox3d* box, const dsMatrix44d* transform)
{
	DS_ASSERT(box);
	DS_ASSERT(transform);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	dsVector4d center = {{box->center.x, box->center.y, box->center.z, 1}};

	dsMatrix33d newOrientation;
	dsVector4d newCenter;

	// Macro will work correctly for treating a 4x4 matrix as a 3x3 matrix.
	dsMatrix33_mul(newOrientation, *transform, box->orientation);

	// Extract scales to apply to the extent.
	double scaleX = dsVector3d_len(&newOrientation.columns[0]);
	double scaleY = dsVector3d_len(&newOrientation.columns[1]);
	double scaleZ = dsVector3d_len(&newOrientation.columns[2]);

	dsMatrix44_transform(newCenter, *transform, center);

	double invScaleX = 1/scaleX;
	double invScaleY = 1/scaleY;
	double invScaleZ = 1/scaleZ;
	dsVector3_scale(box->orientation.columns[0], newOrientation.columns[0], invScaleX);
	dsVector3_scale(box->orientation.columns[1], newOrientation.columns[1], invScaleY);
	dsVector3_scale(box->orientation.columns[2], newOrientation.columns[2], invScaleZ);

	box->center.x = newCenter.x;
	box->center.y = newCenter.y;
	box->center.z = newCenter.z;

	box->halfExtents.x *= scaleX;
	box->halfExtents.y *= scaleY;
	box->halfExtents.z *= scaleZ;

	return true;
}

void dsOrientedBox3f_addPoint(dsOrientedBox3f* box, const dsVector3f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (dsOrientedBox3_isValid(*box))
	{
		dsVector3f localPoint;
		dsVector3f centeredPoint;
		dsVector3_sub(centeredPoint, *point, box->center);
		dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

		dsAlignedBox3f localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
			{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
		};

		dsAlignedBox3_addPoint(localBox, localPoint);

		dsVector3f localCenterOffset, centerOffset;
		dsAlignedBox3_center(localCenterOffset, localBox);
		dsMatrix33_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector3_add(box->center, box->center, centerOffset);

		dsAlignedBox3_extents(box->halfExtents, localBox);
		dsVector3_scale(box->halfExtents, box->halfExtents, 0.5f);
	}
	else
	{
		box->center = *point;
		box->halfExtents.x = 0;
		box->halfExtents.y = 0;
		box->halfExtents.z = 0;
	}
}

void dsOrientedBox3d_addPoint(dsOrientedBox3d* box, const dsVector3d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (dsOrientedBox3_isValid(*box))
	{
		dsVector3d localPoint;
		dsVector3d centeredPoint;
		dsVector3_sub(centeredPoint, *point, box->center);
		dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

		dsAlignedBox3d localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
			{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
		};

		dsAlignedBox3_addPoint(localBox, localPoint);

		dsVector3d localCenterOffset, centerOffset;
		dsAlignedBox3_center(localCenterOffset, localBox);
		dsMatrix33_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector3_add(box->center, box->center, centerOffset);

		dsAlignedBox3_extents(box->halfExtents, localBox);
		dsVector3_scale(box->halfExtents, box->halfExtents, 0.5);
	}
	else
	{
		box->center = *point;
		box->halfExtents.x = 0;
		box->halfExtents.y = 0;
		box->halfExtents.z = 0;
	}
}

bool dsOrientedBox3f_addBox(dsOrientedBox3f* box, const dsOrientedBox3f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3_isValid(*otherBox))
		return false;

	if (dsOrientedBox3_isValid(*box))
	{
		dsAlignedBox3f localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
			{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
		};

		dsVector3f corners[DS_BOX3_CORNER_COUNT];
		DS_VERIFY(dsOrientedBox3f_corners(corners, otherBox));
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		{
			dsVector3f localCorner;
			dsVector3f centeredPoint;
			dsVector3_sub(centeredPoint, corners[i], box->center);
			dsMatrix33_transformTransposed(localCorner, box->orientation, centeredPoint);

			dsAlignedBox3_addPoint(localBox, localCorner);
		}

		dsVector3f localCenterOffset, centerOffset;
		dsAlignedBox3_center(localCenterOffset, localBox);
		dsMatrix33_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector3_add(box->center, box->center, centerOffset);

		dsAlignedBox3_extents(box->halfExtents, localBox);
		dsVector3_scale(box->halfExtents, box->halfExtents, 0.5f);
	}
	else
		*box = *otherBox;

	return true;
}

bool dsOrientedBox3d_addBox(dsOrientedBox3d* box, const dsOrientedBox3d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3_isValid(*otherBox))
		return false;

	if (dsOrientedBox3_isValid(*box))
	{
		dsAlignedBox3d localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
			{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
		};

		dsVector3d corners[DS_BOX3_CORNER_COUNT];
		DS_VERIFY(dsOrientedBox3d_corners(corners, otherBox));
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		{
			dsVector3d localCorner;
			dsVector3d centeredPoint;
			dsVector3_sub(centeredPoint, corners[i], box->center);
			dsMatrix33_transformTransposed(localCorner, box->orientation, centeredPoint);

			dsAlignedBox3_addPoint(localBox, localCorner);
		}

		dsVector3d localCenterOffset, centerOffset;
		dsAlignedBox3_center(localCenterOffset, localBox);
		dsMatrix33_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector3_add(box->center, box->center, centerOffset);

		dsAlignedBox3_extents(box->halfExtents, localBox);
		dsVector3_scale(box->halfExtents, box->halfExtents, 0.5);
	}
	else
		*box = *otherBox;

	return true;
}

bool dsOrientedBox3f_corners(dsVector3f corners[DS_BOX3_CORNER_COUNT], const dsOrientedBox3f* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	corners[0].x = -box->halfExtents.x;
	corners[0].y = -box->halfExtents.y;
	corners[0].z = -box->halfExtents.z;

	corners[1].x = -box->halfExtents.x;
	corners[1].y = -box->halfExtents.y;
	corners[1].z = box->halfExtents.z;

	corners[2].x = -box->halfExtents.x;
	corners[2].y = box->halfExtents.y;
	corners[2].z = -box->halfExtents.z;

	corners[3].x = -box->halfExtents.x;
	corners[3].y = box->halfExtents.y;
	corners[3].z = box->halfExtents.z;

	corners[4].x = box->halfExtents.x;
	corners[4].y = -box->halfExtents.y;
	corners[4].z = -box->halfExtents.z;

	corners[5].x = box->halfExtents.x;
	corners[5].y = -box->halfExtents.y;
	corners[5].z = box->halfExtents.z;

	corners[6].x = box->halfExtents.x;
	corners[6].y = box->halfExtents.y;
	corners[6].z = -box->halfExtents.z;

	corners[7].x = box->halfExtents.x;
	corners[7].y = box->halfExtents.y;
	corners[7].z = box->halfExtents.z;

	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		dsVector3f worldOffset;
		dsMatrix33_transform(worldOffset, box->orientation, corners[i]);
		dsVector3_add(corners[i], worldOffset, box->center);
	}

	return true;
}

bool dsOrientedBox3d_corners(dsVector3d corners[DS_BOX3_CORNER_COUNT], const dsOrientedBox3d* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	corners[0].x = -box->halfExtents.x;
	corners[0].y = -box->halfExtents.y;
	corners[0].z = -box->halfExtents.z;

	corners[1].x = -box->halfExtents.x;
	corners[1].y = -box->halfExtents.y;
	corners[1].z = box->halfExtents.z;

	corners[2].x = -box->halfExtents.x;
	corners[2].y = box->halfExtents.y;
	corners[2].z = -box->halfExtents.z;

	corners[3].x = -box->halfExtents.x;
	corners[3].y = box->halfExtents.y;
	corners[3].z = box->halfExtents.z;

	corners[4].x = box->halfExtents.x;
	corners[4].y = -box->halfExtents.y;
	corners[4].z = -box->halfExtents.z;

	corners[5].x = box->halfExtents.x;
	corners[5].y = -box->halfExtents.y;
	corners[5].z = box->halfExtents.z;

	corners[6].x = box->halfExtents.x;
	corners[6].y = box->halfExtents.y;
	corners[6].z = -box->halfExtents.z;

	corners[7].x = box->halfExtents.x;
	corners[7].y = box->halfExtents.y;
	corners[7].z = box->halfExtents.z;

	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		dsVector3d worldOffset;
		dsMatrix33_transform(worldOffset, box->orientation, corners[i]);
		dsVector3_add(corners[i], worldOffset, box->center);
	}

	return true;
}

bool dsOrientedBox3f_intersects(const dsOrientedBox3f* box, const dsOrientedBox3f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3_isValid(*box) || !dsOrientedBox3_isValid(*otherBox))
		return false;

	// Use separating axis theorem.
	dsVector3f otherBoxCorners[DS_BOX3_CORNER_COUNT];
	dsOrientedBox3f_corners(otherBoxCorners, otherBox);

	// Test axes of box.
	{
		dsVector3f centeredOtherBoxCorners[DS_BOX3_CORNER_COUNT];
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		{
			dsVector3_sub(centeredOtherBoxCorners[i], otherBoxCorners[i], box->center);
		}

		for (unsigned int i = 0; i < 3; ++i)
		{
			float boxMin = -box->halfExtents.values[i];
			float boxMax = box->halfExtents.values[i];

			float otherBoxMin = FLT_MAX;
			float otherBoxMax = -FLT_MAX;
			for (unsigned int j = 0; j < DS_BOX3_CORNER_COUNT; ++j)
			{
				float projectedPoint = dsVector3_dot(box->orientation.columns[i],
					centeredOtherBoxCorners[j]);
				otherBoxMin = dsMin(otherBoxMin, projectedPoint);
				otherBoxMax = dsMax(otherBoxMax, projectedPoint);
			}

			if (!((boxMin >= otherBoxMin && boxMin <= otherBoxMax) ||
				  (otherBoxMin >= boxMin && otherBoxMin <= boxMax)))
			{
				return false;
			}
		}
	}

	dsVector3f boxCorners[DS_BOX3_CORNER_COUNT];
	dsOrientedBox3f_corners(boxCorners, box);

	// Test axes of other box.
	{
		dsVector3f centeredBoxCorners[DS_BOX3_CORNER_COUNT];
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		{
			dsVector3_sub(centeredBoxCorners[i], boxCorners[i], otherBox->center);
		}

		for (unsigned int i = 0; i < 3; ++i)
		{
			float boxMin = FLT_MAX;
			float boxMax = -FLT_MAX;
			for (unsigned int j = 0; j < DS_BOX3_CORNER_COUNT; ++j)
			{
				float projectedPoint = dsVector3_dot(otherBox->orientation.columns[i],
					centeredBoxCorners[j]);
				boxMin = dsMin(boxMin, projectedPoint);
				boxMax = dsMax(boxMax, projectedPoint);
			}

			float otherBoxMin = -otherBox->halfExtents.values[i];
			float otherBoxMax = otherBox->halfExtents.values[i];

			if (!((boxMin >= otherBoxMin && boxMin <= otherBoxMax) ||
				  (otherBoxMin >= boxMin && otherBoxMin <= boxMax)))
			{
				return false;
			}
		}
	}

	// Test the permutations of the pairs of edges of each box.
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			dsVector3f axis;
			dsVector3_cross(axis, box->orientation.columns[i], otherBox->orientation.columns[j]);

			float boxMin = FLT_MAX;
			float boxMax = -FLT_MAX;
			for (unsigned int k = 0; k < DS_BOX3_CORNER_COUNT; ++k)
			{
				float projectedPoint = dsVector3_dot(axis, boxCorners[k]);
				boxMin = dsMin(boxMin, projectedPoint);
				boxMax = dsMax(boxMax, projectedPoint);
			}

			float otherBoxMin = FLT_MAX;
			float otherBoxMax = -FLT_MAX;
			for (unsigned int k = 0; k < DS_BOX3_CORNER_COUNT; ++k)
			{
				float projectedPoint = dsVector3_dot(axis, otherBoxCorners[k]);
				otherBoxMin = dsMin(otherBoxMin, projectedPoint);
				otherBoxMax = dsMax(otherBoxMax, projectedPoint);
			}

			if (!((boxMin >= otherBoxMin && boxMin <= otherBoxMax) ||
			     (otherBoxMin >= boxMin && otherBoxMin <= boxMax)))
			{
				return false;
			}
		}
	}

	return true;
}

bool dsOrientedBox3d_intersects(const dsOrientedBox3d* box, const dsOrientedBox3d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3_isValid(*box) || !dsOrientedBox3_isValid(*otherBox))
		return false;

	// Use separating axis theorem.
	dsVector3d otherBoxCorners[DS_BOX3_CORNER_COUNT];
	dsOrientedBox3d_corners(otherBoxCorners, otherBox);

	// Test axes of box.
	{
		dsVector3d centeredOtherBoxCorners[DS_BOX3_CORNER_COUNT];
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		{
			dsVector3_sub(centeredOtherBoxCorners[i], otherBoxCorners[i], box->center);
		}

		for (unsigned int i = 0; i < 3; ++i)
		{
			double boxMin = -box->halfExtents.values[i];
			double boxMax = box->halfExtents.values[i];

			double otherBoxMin = DBL_MAX;
			double otherBoxMax = -DBL_MAX;
			for (unsigned int j = 0; j < DS_BOX3_CORNER_COUNT; ++j)
			{
				double projectedPoint = dsVector3_dot(box->orientation.columns[i],
					centeredOtherBoxCorners[j]);
				otherBoxMin = dsMin(otherBoxMin, projectedPoint);
				otherBoxMax = dsMax(otherBoxMax, projectedPoint);
			}

			if (!((boxMin >= otherBoxMin && boxMin <= otherBoxMax) ||
				  (otherBoxMin >= boxMin && otherBoxMin <= boxMax)))
			{
				return false;
			}
		}
	}

	// Test axes of other box.
	dsVector3d boxCorners[DS_BOX3_CORNER_COUNT];
	dsOrientedBox3d_corners(boxCorners, box);

	// Test axes of other box.
	{
		dsVector3d centeredBoxCorners[DS_BOX3_CORNER_COUNT];
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		{
			dsVector3_sub(centeredBoxCorners[i], boxCorners[i], otherBox->center);
		}

		for (unsigned int i = 0; i < 3; ++i)
		{
			double boxMin = DBL_MAX;
			double boxMax = -DBL_MAX;
			for (unsigned int j = 0; j < DS_BOX3_CORNER_COUNT; ++j)
			{
				double projectedPoint = dsVector3_dot(otherBox->orientation.columns[i],
					centeredBoxCorners[j]);
				boxMin = dsMin(boxMin, projectedPoint);
				boxMax = dsMax(boxMax, projectedPoint);
			}

			double otherBoxMin = -otherBox->halfExtents.values[i];
			double otherBoxMax = otherBox->halfExtents.values[i];

			if (!((boxMin >= otherBoxMin && boxMin <= otherBoxMax) ||
				  (otherBoxMin >= boxMin && otherBoxMin <= boxMax)))
			{
				return false;
			}
		}
	}

	// Test the permutations of the pairs of edges of each box.
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			dsVector3d axis;
			dsVector3_cross(axis, box->orientation.columns[i], otherBox->orientation.columns[j]);

			double boxMin = DBL_MAX;
			double boxMax = -DBL_MAX;
			for (unsigned int k = 0; k < DS_BOX3_CORNER_COUNT; ++k)
			{
				double projectedPoint = dsVector3_dot(axis, boxCorners[k]);
				boxMin = dsMin(boxMin, projectedPoint);
				boxMax = dsMax(boxMax, projectedPoint);
			}

			double otherBoxMin = DBL_MAX;
			double otherBoxMax = -DBL_MAX;
			for (unsigned int k = 0; k < DS_BOX3_CORNER_COUNT; ++k)
			{
				double projectedPoint = dsVector3_dot(axis, otherBoxCorners[k]);
				otherBoxMin = dsMin(otherBoxMin, projectedPoint);
				otherBoxMax = dsMax(otherBoxMax, projectedPoint);
			}

			if (!((boxMin >= otherBoxMin && boxMin <= otherBoxMax) ||
			      (otherBoxMin >= boxMin && otherBoxMin <= boxMax)))
			{
				return false;
			}
		}
	}

	return true;
}

bool dsOrientedBox3f_closestPoint(dsVector3f* result, const dsOrientedBox3f* box,
	const dsVector3f* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	dsAlignedBox3f localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	dsVector3f localPoint;
	dsVector3f centeredPoint;
	dsVector3_sub(centeredPoint, *point, box->center);
	dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

	dsVector3f localResult;
	dsAlignedBox3_closestPoint(localResult, localBox, localPoint);
	dsMatrix33_transform(*result, box->orientation, localResult);
	dsVector3_add(*result, *result, box->center);

	return true;
}

bool dsOrientedBox3d_closestPoint(dsVector3d* result, const dsOrientedBox3d* box,
	const dsVector3d* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	dsAlignedBox3d localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	dsVector3d localPoint;
	dsVector3d centeredPoint;
	dsVector3_sub(centeredPoint, *point, box->center);
	dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

	dsVector3d localResult;
	dsAlignedBox3_closestPoint(localResult, localBox, localPoint);
	dsMatrix33_transform(*result, box->orientation, localResult);
	dsVector3_add(*result, *result, box->center);

	return true;
}

float dsOrientedBox3f_dist2(const dsOrientedBox3f* box, const dsVector3f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3_isValid(*box))
		return -1;

	dsAlignedBox3f localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	dsVector3f localPoint;
	dsVector3f centeredPoint;
	dsVector3_sub(centeredPoint, *point, box->center);
	dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

	return dsAlignedBox3f_dist2(&localBox, &localPoint);
}

double dsOrientedBox3d_dist2(const dsOrientedBox3d* box, const dsVector3d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3_isValid(*box))
		return -1;

	dsAlignedBox3d localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	dsVector3d localPoint;
	dsVector3d centeredPoint;
	dsVector3_sub(centeredPoint, *point, box->center);
	dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

	return dsAlignedBox3d_dist2(&localBox, &localPoint);
}

float dsOrientedBox3f_dist(const dsOrientedBox3f* box, const dsVector3f* point)
{
	float distance2 = dsOrientedBox3f_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrtf(distance2);
}

double dsOrientedBox3d_dist(const dsOrientedBox3d* box, const dsVector3d* point)
{
	double distance2 = dsOrientedBox3d_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrt(distance2);
}

bool dsOrientedBox3f_isValid(const dsOrientedBox3f* box);
bool dsOrientedBox3d_isValid(const dsOrientedBox3d* box);

void dsOrientedBox3f_fromAlignedBox(dsOrientedBox3f* result, const dsAlignedBox3f* alignedBox);
void dsOrientedBox3d_fromAlignedBox(dsOrientedBox3d* result, const dsAlignedBox3d* alignedBox);

void dsOrientedBox3f_makeInvalid(dsOrientedBox3f* result);
void dsOrientedBox3d_makeInvalid(dsOrientedBox3d* result);
