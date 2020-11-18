//
// Created by laurens on 11/16/20.
//
#include "environment/intersections.h"

namespace lh2core
{

void BruteForceIntersector::setPrimitives( Primitive* primitives, int count )
{
	this->primitives = primitives;
	this->count = count;
}
ShortestDistance BruteForceIntersector::nearest( const Ray& r )
{
	float nearest = MAX_DISTANCE;
	int minIndex = -1;
	for ( int i = 0; i < count; ++i )
	{
		float d = distanceToPrimitive( primitives[i], r );
		if ( d < nearest )
		{
			nearest = d;
			minIndex = i;
		}
	}
	return ShortestDistance{ nearest, &primitives[minIndex] };
}
} // namespace lh2core