//
// Created by laurens on 11/16/20.
//
#include "core_settings.h"

namespace lh2core
{
float Sphere::distanceTo( Ray r )
{
	float3 C = pos - r.start;
	float t = dot( C, r.direction );
	float3 Q = C - t * r.direction;
	float p2 = dot( Q, Q );
	if ( p2 > r2 ) return -1;
	t -= sqrt( r2 - p2 );
	return t;
}
Intersection Sphere::intersectionAt( float3 intersectionPoint, Material* materials )
{
	return Intersection{ intersectionPoint, normalize( intersectionPoint - pos ), materials[material] };
}
Intersection Plane::intersectionAt( float3 intersectionPoint, Material* materials )
{
	int sum = floor( intersectionPoint.x ) + floor( intersectionPoint.z );
	if ( sum % 2 == 0 ) return Intersection{ intersectionPoint, normal, Material{ make_float3( 0.6, 0.3, 0.2 ) } };
	return Intersection{ intersectionPoint, normal, Material{ make_float3( 1, 0.9, 0.7 ) } };
	//	return Intersection{ intersectionPoint, normal, materials[material] };
}
float Plane::distanceTo( Ray r )
{
	auto denom = dot( r.direction, normal );
	if ( abs( denom ) <= 1e-5 )
	{
		return -1;
	}
	float t = -( dot( r.start, normal ) + d ) / denom;
	if ( t <= 1e-4 ) { return -1; }
	return t;
}

#define FIND_MIN_DISTANCE( data, count )                        \
	int minIndex = 0;                                           \
	float minDistance = -1;                                     \
	for ( int j = 0; j < count; ++j )                           \
	{                                                           \
		float d = data[j].distanceTo( r );                      \
		if ( d >= 0 && ( minDistance < 0 || d < minDistance ) ) \
		{                                                       \
			minIndex = j;                                       \
			minDistance = d;                                    \
		}                                                       \
	}                                                           \
	return ShortestDistance{ minDistance, minIndex };

ShortestDistance Spheres::minDistanceTo( Ray r )
{
	FIND_MIN_DISTANCE( spheres, count );
}
Intersection Spheres::intersectionWith( int index, float3 origin, float3 intersection )
{
	return spheres[index].intersectionAt( intersection, materials );
}
ShortestDistance Planes::minDistanceTo( Ray r )
{
	FIND_MIN_DISTANCE( planes, count );
}
Intersection Planes::intersectionWith( int index, float3 origin, float3 intersection )
{
	return planes[index].intersectionAt( intersection, materials );
}
} // namespace lh2core