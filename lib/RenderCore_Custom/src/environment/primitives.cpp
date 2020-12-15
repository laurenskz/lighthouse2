//
// Created by laurens on 11/18/20.
//
#include "environment/primitives.h"
namespace lh2core
{
void intersectPrimitive( const Primitive* primitive, Ray& r )
{
	if ( isTriangle( *primitive ) ) intersectTriangle( primitive, r );
	if ( isPlane( *primitive ) ) intersectPlane( primitive, r );
	if ( isSphere( *primitive ) ) intersectSphereInside( primitive, r );
}
void intersectSphere( const Primitive* primitive, Ray& r )
{
	auto pos = primitive->v1;
	auto r2 = primitive->v2.x;
	float3 C = pos - r.start;
	float t = dot( C, r.direction );
	float3 Q = C - t * r.direction;
	float p2 = dot( Q, Q );
	if ( p2 > r2 ) return;
	t -= sqrt( r2 - p2 );
	updateT( t, primitive, r );
}

void intersectSphereInside( const Primitive* primitive, Ray& r )
{
	auto center = primitive->v1;
	auto r2 = primitive->v2.x;
	float3 oc = r.start - center;
	float a = dot( r.direction, r.direction );
	float b = 2.0f * dot( oc, r.direction );
	float c = dot( oc, oc ) - r2;
	float discriminant = b * b - 4 * a * c;
	if ( discriminant < 0.0 )
	{
		return;
	}
	else
	{
		float t;
		float root = sqrt( discriminant );
		float numerator = -b - root;
		if ( numerator > 0.0 )
		{
			t = numerator / ( 2.0f * a );
			updateT( t, primitive, r );
			return;
		}

		numerator = -b + root;
		if ( numerator > 0.0 )
		{
			t = numerator / ( 2.0f * a );
		}
		else
		{
			t = MAX_DISTANCE;
		}
		updateT( t, primitive, r );
	}
}

void intersectPlane( const Primitive* primitive, Ray& r )
{
	auto normal = primitive->v1;
	auto d = primitive->v2.x;
	auto denom = dot( r.direction, normal );
	if ( abs( denom ) <= 1e-4 )
	{
		return;
	}
	float t = -( dot( r.start, normal ) + d ) / denom;
	if ( t <= 1e-4 ) { return; }
	updateT( t, primitive, r );
}
void intersectTriangle( const Primitive* primitive, Ray& r )
{
	float3 edge1{}, edge2{}, h{}, s{}, q{};
	float a, f, u, v;
	edge1 = primitive->v2 - primitive->v1;
	edge2 = primitive->v3 - primitive->v1;
	h = cross( r.direction, edge2 );
	a = dot( edge1, h );
#ifndef CULLING
	if ( a < EPS )
		return; // This ray is parallel to this triangle.
#else
	if ( ( a > -EPS ) && a < EPS )
		return;
#endif
	f = 1.0f / a;
	s = r.start - primitive->v1;
	u = f * dot( s, h );
	if ( u < 0.0 || u > 1.0 )
		return;
	q = cross( s, edge1 );
	v = f * dot( r.direction, q );
	if ( v < 0.0 || u + v > 1.0 )
		return;
	float t = f * dot( edge2, q );
	if ( t > EPS && t < r.t ) // ray intersection
	{
		r.t = t;
		r.u = u;
		r.v = v;
		r.primitive = primitive;
	}
}
Intersection sphereIntersection( const Ray& r, const Material& mat )
{
	const float3& intersectionPoint = intersectionLocation( r );
	Intersection intersection{};
	intersection.location = intersectionLocation( r );
	intersection.normal = normalize( intersectionPoint - r.primitive->v1 );
	intersection.mat = mat;
	return intersection;
}
Intersection planeIntersection( const Ray& r, const Material& mat )
{
	auto intersectionPoint = intersectionLocation( r );
	Intersection intersection{};
	intersection.location = intersectionPoint;
	intersection.normal = r.primitive->v1;
	intersection.mat.type = DIFFUSE;
	int sum = floor( intersectionPoint.x ) + floor( intersectionPoint.z );
	if ( sum % 2 == 0 )
	{
		intersection.mat.color = make_float3( 0.6, 0.3, 0.2 );
	}
	else
	{
		intersection.mat.color = make_float3( 1, 0.9, 0.7 );
	}
	return intersection;
}
} // namespace lh2core
