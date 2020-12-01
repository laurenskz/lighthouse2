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
	Distance nearest = Distance{ MAX_DISTANCE };
	int minIndex = -1;
	for ( int i = 0; i < count; ++i )
	{
		Primitive prim = primitives[i];
		auto d = distanceToPrimitive( prim, r );
		if ( d.d > 0 && d.d < nearest.d )
		{
			nearest = d;
			minIndex = i;
		}
	}
	if ( minIndex == -1 ) return ShortestDistance{ Distance{ MAX_DISTANCE }, nullptr };
	if ( primitives[minIndex].flags > 100 )
	{
		cout << "What?" << endl;
	}
	return ShortestDistance{ nearest, &primitives[minIndex] };
}
bool BruteForceIntersector::isOccluded( const Ray& r, float object )
{
	for ( int i = 0; i < count; ++i )
	{
		auto d = distanceToPrimitive( primitives[i], r );
		if ( d.d > 0 && d.d < object )
		{
			return true; //Occlusion
		}
	}
	return false;
}
} // namespace lh2core