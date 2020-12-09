#include "environment/environment.h"
namespace lh2core
{
#define FLOAT_EQ( x, y ) abs( x - y ) < 1e-3;

Intersection TestEnvironment::intersect( Ray& r )
{
	for ( int i = 0; i < rays.size(); ++i )
	{
		if ( abs( r.start.x - rays[i].start.x ) < 1e-3 && abs( r.start.y - rays[i].start.y ) < 1e-3 && abs( r.start.z - rays[i].start.z ) < 1e-3 )
		{
			return intersections[i];
		}
	}
	return Intersection{};
}
void TestEnvironment::intersectPacket( const RayPacket& rayPacket )
{
	for ( int i = 0; i < rayPacket.rayCount; ++i )
	{
		rayPacket.intersections[i] = intersect( rayPacket.rays[i] );
	}
}
void Environment::intersectPacket( const RayPacket& rayPacket )
{
	for ( int i = 0; i < rayPacket.rayCount; ++i )
	{
		rayPacket.intersections[i] = intersect( rayPacket.rays[i] );
	}
}
Intersection Environment::intersect( Ray& r )
{
	intersector->intersect( r );
	if ( r.t == MAX_DISTANCE )
	{
		return Intersection{};
	}
	auto intersection = geometry->intersectionInformation( r );
	intersection.hitObject = true;
	return intersection;
}
} // namespace lh2core
