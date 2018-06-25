/*
 * Copyright 2018 Aaron Barany
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

#include <DeepSea/Geometry/SimpleHoledPolygon.h>

#include "BasePolygon.h"
#include <DeepSea/Geometry/SimplePolygon.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <float.h>
#include <string.h>

struct dsSimpleHoledPolygon
{
	dsBasePolygon base;

	uint32_t* equalVertexList;
	uint32_t maxEqualVertices;

	uint32_t mainEdgeCount;

	uint32_t* loopVerts;
	uint32_t loopVertCount;
	uint32_t maxLoopVerts;

	dsSimplePolygon* simplePolygon;
};

static bool isPolygonLoopCCW(dsBasePolygon* polygon, const dsSimplePolygonLoop* loop)
{
	// https://en.wikipedia.org/wiki/Shoelace_formula
	// Negative area is counter-clockwise, positive is clockwise.
	uint32_t curIndex = loop->firstPoint + loop->pointCount - 1;
	uint32_t nextIndex = loop->firstPoint;
	double doubleArea =
		(polygon->vertices[curIndex].point.x + polygon->vertices[nextIndex].point.x)*
		(polygon->vertices[curIndex].point.y - polygon->vertices[nextIndex].point.y);

	curIndex = loop->firstPoint;
	nextIndex = loop->firstPoint + 1;
	for (uint32_t i = 0; i < loop->pointCount - 1; ++i, ++curIndex, ++nextIndex)
	{
		doubleArea += (polygon->vertices[curIndex].point.x + polygon->vertices[nextIndex].point.x)*
			(polygon->vertices[curIndex].point.y - polygon->vertices[nextIndex].point.y);
	}

	return doubleArea <= 0.0;
}


static bool addVertices(dsSimpleHoledPolygon* polygon, const void* points,
	uint32_t pointCount, dsPolygonPositionFunction pointPositionFunc)
{
	dsBasePolygon* base = &polygon->base;
	DS_ASSERT(base->vertexCount == 0);
	if (!DS_RESIZEABLE_ARRAY_ADD(base->allocator, base->vertices, base->vertexCount,
		base->maxVertices, pointCount))
	{
		return false;
	}

	DS_ASSERT(base->vertexCount == pointCount);
	DS_ASSERT(base->edgeCount == pointCount);
	for (uint32_t i = 0; i < pointCount; ++i)
	{
		Vertex* vertex = base->vertices + i;
		if (!pointPositionFunc(&vertex->point, base->userData, points, i))
			return false;

		if (i > 0 && dsVector2d_epsilonEqual(&vertex->point, &base->vertices[i - 1].point,
			EPSILON))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon may not have duplicate points in a series.");
			return false;
		}

		vertex->prevEdges.head.edge = NOT_FOUND;
		vertex->prevEdges.head.nextConnection = NOT_FOUND;
		vertex->prevEdges.tail = NOT_FOUND;
		vertex->nextEdges.head.edge = NOT_FOUND;
		vertex->nextEdges.head.nextConnection = NOT_FOUND;
		vertex->nextEdges.tail = NOT_FOUND;
	}

	return true;
}

static bool addLoopEdges(dsSimpleHoledPolygon* polygon, const dsSimplePolygonLoop* loops,
	uint32_t loopCount)
{
	dsBasePolygon* base = &polygon->base;

	uint32_t edgeCount = 0;
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		if (loops[i].pointCount == 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Empty polygon loop.");
			return false;
		}

		if (!DS_IS_BUFFER_RANGE_VALID(loops[i].firstPoint, loops[i].pointCount, base->vertexCount))
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon loop points of out of range.");
			return false;
		}

		edgeCount += loops[i].pointCount;
	}

	DS_ASSERT(base->edgeCount == 0);
	if (!DS_RESIZEABLE_ARRAY_ADD(base->allocator, base->edges, base->edgeCount, base->maxEdges,
		edgeCount))
	{
		return false;
	}

	uint32_t baseEdgeIdx = 0;
	for (uint32_t i = 0; i < loopCount; ++i)
	{
		const dsSimplePolygonLoop* loop = loops + i;
		bool loopCCW = isPolygonLoopCCW(base, loop);
		// Use CCW for outer loop, CW for inner loop.
		bool flip = i == 0 ? !loopCCW : loopCCW;

		for (uint32_t j = 0; j < loop->pointCount; ++j)
		{
			Vertex* vertex = base->vertices + loop->firstPoint + j;
			if (vertex->prevEdges.head.edge != NOT_FOUND)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG,
					"The same point may not be a part of multiple polygon loops.");
				return false;
			}

			Edge* edge = base->edges + baseEdgeIdx + j;
			edge->prevVertex = loop->firstPoint + j;
			if (flip)
			{
				edge->nextVertex = loop->firstPoint + (j == 0 ? loop->pointCount - 1 : j - 1);
				edge->prevEdge = baseEdgeIdx + (j == loop->pointCount - 1 ? 0 : j + 1);
				edge->nextEdge = baseEdgeIdx + (j == 0 ? loop->pointCount - 1 : j - 1);
			}
			else
			{
				edge->nextVertex = loop->firstPoint + (j == loop->pointCount - 1 ? 0 : j + 1);
				edge->prevEdge = baseEdgeIdx + (j == 0 ? loop->pointCount - 1 : j - 1);
				edge->nextEdge = baseEdgeIdx + (j == loop->pointCount - 1 ? 0 : j + 1);
			}
			edge->visited = false;

			vertex->prevEdges.head.edge = edge->prevEdge;
			vertex->nextEdges.head.edge = baseEdgeIdx + j;
		}

		if (dsVector2d_epsilonEqual(&base->vertices[loop->firstPoint].point,
			&base->vertices[loop->firstPoint + loop->pointCount - 1].point, EPSILON))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG,
				"Polygon loop may not duplicate the first and last point.");
			return false;
		}

		baseEdgeIdx += loop->pointCount;
	}

	polygon->mainEdgeCount = base->edgeCount;
	return true;
}

static bool findEqualVertices(dsSimpleHoledPolygon* polygon)
{
	dsBasePolygon* base = &polygon->base;
	if (!dsBasePolygon_sortVertices(&polygon->base))
		return false;

	if (!polygon->equalVertexList || polygon->maxEqualVertices < base->maxVertices)
	{
		DS_VERIFY(dsAllocator_free(base->allocator, polygon->equalVertexList));
		polygon->maxEqualVertices = base->maxVertices;
		polygon->equalVertexList = DS_ALLOCATE_OBJECT_ARRAY(base->allocator, uint32_t,
			polygon->maxEqualVertices);
		if (!polygon->equalVertexList)
			return false;
	}
	memset(polygon->equalVertexList, 0xFF, base->vertexCount*sizeof(uint32_t));

	for (uint32_t i = 0; i < base->vertexCount; ++i)
	{
		if (polygon->equalVertexList[i] != NOT_FOUND)
			continue;

		// Vertices are sorted along x axis. Find all equal points until we're over two epsilon
		// away. Use indices to form a circular linked list.
		const dsVector2d* firstPoint = &base->vertices[base->sortedVerts[i]].point;
		uint32_t firstIndex = base->sortedVerts[i];
		uint32_t lastIndex = firstIndex;
		for (uint32_t j = i + 1; i < base->vertexCount; ++j)
		{
			const dsVector2d* curPoint = &base->vertices[base->sortedVerts[j]].point;
			if (curPoint->x > firstPoint->x + 2*EPSILON)
				break;

			// Use 2 epsilon to compare the X since we're starting on the left-most boundary.
			if (dsEpsilonEquald(firstPoint->x, curPoint->x, 2*EPSILON) &&
				dsEpsilonEquald(firstPoint->y, curPoint->y, EPSILON))
			{
				uint32_t curIndex = base->sortedVerts[j];
				polygon->equalVertexList[lastIndex] = curIndex;
				polygon->equalVertexList[curIndex] = firstIndex;
				lastIndex = curIndex;
			}
		}
	}

	return true;
}

static bool addVerticesAndEdges(dsSimpleHoledPolygon* polygon, const void* points,
	uint32_t pointCount, const dsSimplePolygonLoop* loops, uint32_t loopCount,
	dsPolygonPositionFunction pointPositionFunc)
{
	if (!addVertices(polygon, points, pointCount, pointPositionFunc))
		return false;

	if (!addLoopEdges(polygon, loops, loopCount))
		return false;

	return findEqualVertices(polygon);
}

static bool canConnectEdge(const dsSimpleHoledPolygon* polygon, uint32_t fromVertIdx,
	uint32_t toVertIdx)
{
	const dsBasePolygon* base = &polygon->base;

	// Check against the original edges using the acceleration structure.
	if (!dsBasePolygon_canConnectEdge(base, fromVertIdx, toVertIdx))
		return false;

	// Check against the new edges, sicne they should be few in number. Only need to check against
	// every other edge since they are in pairs.
	const dsVector2d* fromPos = &base->vertices[fromVertIdx].point;
	const dsVector2d* toPos = &base->vertices[toVertIdx].point;
	dsAlignedBox2d edgeBounds = {*fromPos, *fromPos};
	dsAlignedBox2_addPoint(edgeBounds, *toPos);

	DS_ASSERT(polygon->mainEdgeCount <= base->edgeCount);
	for (uint32_t i = polygon->mainEdgeCount; i < base->edgeCount; i += 2)
	{
		const Edge* otherEdge = base->edges + i;

		// Don't count neighboring edges.
		if (otherEdge->prevVertex == fromVertIdx || otherEdge->prevVertex == toVertIdx ||
			otherEdge->nextVertex == fromVertIdx || otherEdge->nextVertex == toVertIdx)
		{
			continue;
		}

		const dsVector2d* otherFrom = &base->vertices[otherEdge->prevVertex].point;
		const dsVector2d* otherTo = &base->vertices[otherEdge->nextVertex].point;
		if (dsIntersectPolygonEdges(NULL, NULL, NULL, fromPos, toPos,
			otherFrom, otherTo))
		{
			return false;
		}
	}

	return true;
}

static bool connectLoops(dsSimpleHoledPolygon* polygon, const dsSimplePolygonLoop* loops,
	uint32_t loopCount)
{
	if (loopCount <= 1)
		return true;

	DS_ASSERT(polygon->mainEdgeCount == polygon->base.edgeCount);
	if (!dsBasePolygon_buildEdgeBVH(&polygon->base))
		return false;

	dsBasePolygon* base = &polygon->base;
	const dsSimplePolygonLoop* outerLoop = loops;
	for (uint32_t i = 1; i < loopCount; ++i)
	{
		const dsSimplePolygonLoop* innerLoop = loops + i;
		for (uint32_t j = 0; j < innerLoop->pointCount; ++j)
		{
			bool found = false;
			for (uint32_t k = 0; k < outerLoop->pointCount; ++k)
			{
				uint32_t fromVert = outerLoop->firstPoint + k;
				uint32_t toVert = innerLoop->firstPoint + j;
				if (canConnectEdge(polygon, fromVert, toVert))
				{
					if (!dsBasePolygon_addSeparatingEdge(base, fromVert, toVert, true))
						return false;
					found = true;
					break;
				}
			}

			if (found)
				break;
		}
	}

	return true;
}

static uint32_t findNextEdge(const dsSimpleHoledPolygon* polygon, uint32_t edgeIdx)
{
	const dsBasePolygon* base = &polygon->base;

	const Edge* edge = base->edges + edgeIdx;
	dsVector2d edgeDir;
	dsVector2_sub(edgeDir, base->vertices[edge->nextVertex].point,
		base->vertices[edge->prevVertex].point);
	dsVector2d_normalize(&edgeDir, &edgeDir);

	uint32_t closestEdge = NOT_FOUND;
	double closestAngle = DBL_MAX;

	uint32_t firstVert = edge->nextVertex;
	uint32_t curVert = firstVert;
	do
	{
		const EdgeConnection* curConnection = &base->vertices[curVert].nextEdges.head;
		do
		{
			if (!base->edges[curConnection->edge].visited)
			{
				double angle = dsBasePolygon_edgeAngle(base, curConnection->edge, &edgeDir, false,
					true);
				if (angle < closestAngle)
				{
					closestEdge = curConnection->edge;
					closestAngle = angle;
				}
			}

			if (curConnection->nextConnection == NOT_FOUND)
				curConnection = NULL;
			else
				curConnection = base->edgeConnections + curConnection->nextConnection;
		} while (curConnection);

		curVert = polygon->equalVertexList[curVert];
	} while (curVert != firstVert && curVert != NOT_FOUND);

	return closestEdge;
}

bool getLoopPosition(dsVector2d* outPosition, void* userData, const void* points, uint32_t index)
{
	DS_UNUSED(userData);
	const dsSimpleHoledPolygon* polygon = (const dsSimpleHoledPolygon*)points;
	const dsBasePolygon* base = &polygon->base;
	*outPosition = base->vertices[polygon->loopVerts[index]].point;
	return true;
}

static bool triangulateLoop(dsSimpleHoledPolygon* polygon, uint32_t startEdge,
	dsTriangulateWinding winding)
{
	dsBasePolygon* base = &polygon->base;
	polygon->loopVertCount = 0;
	uint32_t edge = startEdge;
	do
	{
		DS_ASSERT(!base->edges[edge].visited);
		uint32_t loopVertIdx = polygon->loopVertCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(base->allocator, polygon->loopVerts, polygon->loopVertCount,
			polygon->maxLoopVerts, 1))
		{
			return false;
		}

		polygon->loopVerts[loopVertIdx] = base->edges[edge].prevVertex;
		base->edges[edge].visited = true;

		edge = findNextEdge(polygon, edge);
		DS_ASSERT(edge != NOT_FOUND);
	} while (edge != startEdge);

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon->simplePolygon,
		polygon, polygon->loopVertCount, &getLoopPosition, winding);
	uint32_t firstIndex = base->indexCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(base->allocator, base->indices, base->indexCount, base->maxIndices,
		indexCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < indexCount; ++i)
		base->indices[firstIndex + i] = polygon->loopVerts[indices[i]];

	return true;
}

static bool triangulateLoops(dsSimpleHoledPolygon* polygon, dsTriangulateWinding winding)
{
	dsBasePolygon* base = &polygon->base;
	for (uint32_t i = 0; i < base->edgeCount; ++i)
	{
		if (base->edges[i].visited)
			continue;

		if (!triangulateLoop(polygon, i, winding))
			return false;
	}

	return true;
}

dsSimpleHoledPolygon* dsSimpleHoledPolygon_create(dsAllocator* allocator, void* userData)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Simple polygon allocator must support freeing memory.");
		return NULL;
	}

	dsSimpleHoledPolygon* polygon = DS_ALLOCATE_OBJECT(allocator, dsSimpleHoledPolygon);
	if (!polygon)
		return NULL;

	memset(polygon, 0, sizeof(dsSimpleHoledPolygon));
	polygon->base.allocator = dsAllocator_keepPointer(allocator);
	polygon->base.userData = userData;
	polygon->simplePolygon = dsSimplePolygon_create(allocator, userData);
	if (!polygon->simplePolygon)
	{
		DS_VERIFY(dsAllocator_free(allocator, polygon));
		return NULL;
	}
	return polygon;
}

void* dsSimpleHoledPolygon_getUserData(const dsSimpleHoledPolygon* polygon)
{
	if (!polygon)
		return NULL;

	return polygon->base.userData;
}

void dsSimpleHoledPolygon_setUserData(dsSimpleHoledPolygon* polygon, void* userData)
{
	if (polygon)
		polygon->base.userData = userData;
}

const uint32_t* dsSimpleHoledPolygon_triangulate(uint32_t* outIndexCount,
	dsSimpleHoledPolygon* polygon, const void* points, uint32_t pointCount,
	const dsSimplePolygonLoop* loops, uint32_t loopCount,
	dsPolygonPositionFunction pointPositionFunc, dsTriangulateWinding winding)
{
	if (outIndexCount)
		*outIndexCount = 0;

	if (!outIndexCount || !polygon || !points || pointCount == 0 || !loops || loopCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	dsBasePolygon* base = &polygon->base;
	dsBasePolygon_reset(base);

	// Single loop is the same as a normal simple polygon.
	if (loopCount == 1 && loops[0].firstPoint == 0)
	{
		if (loops[0].pointCount > pointCount)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_GEOMETRY_LOG_TAG, "Polygon loop points of out of range.");
			return NULL;
		}

		return dsSimplePolygon_triangulate(outIndexCount, polygon->simplePolygon, points,
			loops[0].pointCount, pointPositionFunc, winding);
	}

	if (!pointPositionFunc)
		pointPositionFunc = &dsSimplePolygon_getPointVector2d;

	if (!addVerticesAndEdges(polygon, points, pointCount, loops, loopCount, pointPositionFunc))
		return NULL;

	if (!connectLoops(polygon, loops, loopCount))
		return NULL;

	if (!triangulateLoops(polygon, winding))
		return NULL;

	*outIndexCount = base->indexCount;
	return base->indices;
}

void dsSimpleHolePolygon_destroy(dsSimpleHoledPolygon* polygon)
{
	if (!polygon || !polygon->base.allocator)
		return;

	dsBasePolygon_shutdown(&polygon->base);
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon->equalVertexList));
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon->loopVerts));
	dsSimplePolygon_destroy(polygon->simplePolygon);
	DS_VERIFY(dsAllocator_free(polygon->base.allocator, polygon));
}
