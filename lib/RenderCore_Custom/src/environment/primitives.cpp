//
// Created by laurens on 11/18/20.
//
#include "environment/primitives.h"
namespace lh2core
{
float distanceToPrimitive( const Primitive& primitive, Ray r )
{
	if ( isTriangle( primitive ) ) return distanceToTriangle( primitive, r );
	if ( isPlane( primitive ) ) return distanceToPlane( primitive, r );
	if ( isSphere( primitive ) ) return distanceToSphere( primitive, r );
	return MAX_DISTANCE;
}
float distanceToSphere( const Primitive& primitive, Ray r )
{
	auto pos = primitive.v1;
	auto r2 = primitive.v2.x;
	float3 C = pos - r.start;
	float t = dot( C, r.direction );
	float3 Q = C - t * r.direction;
	float p2 = dot( Q, Q );
	if ( p2 > r2 ) return MAX_DISTANCE;
	t -= sqrt( r2 - p2 );
	return t;
}
float distanceToPlane( const Primitive& primitive, Ray r )
{
	auto normal = primitive.v1;
	auto d = primitive.v2.x;
	auto denom = dot( r.direction, normal );
	if ( abs( denom ) <= 1e-4 )
	{
		return MAX_DISTANCE;
	}
	float t = -( dot( r.start, normal ) + d ) / denom;
	if ( t <= 1e-4 ) { return MAX_DISTANCE; }
	return t;
}
float distanceToTriangle( const Primitive& primitive, Ray r )
{
	float3 edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = primitive.v2 - primitive.v1;
	edge2 = primitive.v3 - primitive.v1;
	h = cross( r.direction, edge2 );
	a = dot( edge1, h );
	if ( a > -EPSILON && a < EPSILON )
		return MAX_DISTANCE; // This ray is parallel to this triangle.
	f = 1.0 / a;
	s = r.start - primitive.v1;
	u = f * dot( s, h );
	if ( u < 0.0 || u > 1.0 )
		return MAX_DISTANCE;
	q = cross( s, edge1 );
	v = f * dot( r.direction, q );
	if ( v < 0.0 || u + v > 1.0 )
		return MAX_DISTANCE;
	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * dot( edge2, q );
	if ( t > EPSILON ) // ray intersection
	{
		return t;
	}
	return MAX_DISTANCE;
}
Intersection sphereIntersection( const Primitive& primitive, const Material& mat, Ray r, float t )
{
	const float3& intersectionPoint = locationAt( t, r );
	return Intersection{ intersectionPoint, normalize( intersectionPoint - primitive.v1 ), mat };
}
Intersection planeIntersection( const Primitive& primitive, const Material& mat, Ray r, float t )
{
	auto intersectionPoint = locationAt( t, r );
	int sum = floor( intersectionPoint.x ) + floor( intersectionPoint.z );
	if ( sum % 2 == 0 ) return Intersection{ intersectionPoint, primitive.v1, Material{ make_float3( 0.6, 0.3, 0.2 ) } };
	return Intersection{ intersectionPoint, primitive.v1, Material{ make_float3( 1, 0.9, 0.7 ) } };
}
} // namespace lh2core
