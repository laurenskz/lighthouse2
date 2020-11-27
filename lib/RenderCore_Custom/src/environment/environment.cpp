#include "environment/environment.h"
namespace lh2core
{
#define FLOAT_EQ( x, y ) abs( x - y ) < 1e-3;
Intersection Environment::intersect( const Ray& r )
{
	const ShortestDistance& nearest = intersector->nearest( r );
	if ( nearest.minDistance.d == MAX_DISTANCE )
	{
		return Intersection(); //We hit nothing
	}
	Intersection intersection = geometry->intersectionInformation( *nearest.primitive, nearest.minDistance, r );
	intersection.hitObject = true;
	return intersection;
}
Intersection TestEnvironment::intersect( const Ray& r )
{
	for ( int i = 0; i < rays.size(); ++i )
	{
		if ( abs( r.start.x - rays[i].start.x ) < 1e-3 && abs( r.start.y - rays[i].start.y ) < 1e-3 && abs( r.start.z - rays[i].start.z ) < 1e-3 )
		{
			return intersections[i];
		}
	}
	return Intersection();
}
} // namespace lh2core
