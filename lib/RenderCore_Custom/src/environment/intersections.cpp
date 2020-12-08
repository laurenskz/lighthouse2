//
// Created by laurens on 11/16/20.
//
#include "environment/intersections.h"

namespace lh2core
{

void BruteForceIntersector::setPrimitives( Primitive* newPrimitives, int newCount )
{
	this->primitives = newPrimitives;
	this->count = newCount;
}
void BruteForceIntersector::intersect( Ray& r )
{
	for ( int i = 0; i < count; ++i )
	{
		intersectPrimitive( &primitives[i], r );
	}
}
bool BruteForceIntersector::isOccluded( Ray& r, float object )
{
	for ( int i = 0; i < count; ++i )
	{
		//		Transparent objects don't occlude
		if ( primitives[i].flags & TRANSPARENT_BIT ) continue;
		intersectPrimitive( &primitives[i], r );
		if ( r.t < object ) return true;
	}
	return false;
}
void BruteForceIntersector::intersectPacket( const RayPacket& packet )
{
	for ( int i = 0; i < packet.rayCount; ++i )
	{
		intersect( packet.rays[i] );
	}
}
void BruteForceIntersector::packetOccluded( const RayPacket& packet )
{
	for ( int i = 0; i < packet.rayCount; ++i )
	{
		packet.occlusions[i] = isOccluded( packet.rays[i], packet.occlusionDistances[i] );
	}
}
} // namespace lh2core