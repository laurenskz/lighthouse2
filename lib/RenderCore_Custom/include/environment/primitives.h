#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"

namespace lh2core
{
#define SET_BIT( pos ) ( (uint)1 << ( pos ) )
//#define CULLING
const uint SPHERE_BIT = (uint)1 << (uint)0;
const uint TRIANGLE_BIT = (uint)1 << (uint)1;
const uint PLANE_BIT = (uint)1 << (uint)2;
const uint CULLING_BIT = (uint)1 << (uint)3;
const uint LIGHT_BIT = (uint)1 << (uint)4;
const uint TRANSPARENT_BIT = (uint)1 << (uint)5;

struct Primitives
{
	Primitive* data;
	int size;
};

struct Distance
{
	float d{};
	float u{};
	float v{};
};


inline bool isSphere( const Primitive& primitive ) { return primitive.flags & SPHERE_BIT; }
inline bool isTriangle( const Primitive& primitive ) { return primitive.flags & TRIANGLE_BIT; }
inline bool isPlane( const Primitive& primitive ) { return primitive.flags & PLANE_BIT; }

inline float3 intersectionLocation( const Ray& r ) { return r.start + r.t * r.direction; };

void intersectPrimitive( const Primitive* primitive, Ray& r );
void intersectSphere( const Primitive* primitive, Ray& r );
void intersectSphereInside( const Primitive* primitive, Ray& r );
void intersectTriangle( const Primitive* primitive, Ray& r );
void intersectPlane( const Primitive* primitive, Ray& r );

Intersection sphereIntersection( const Ray& r, const Material& mat );
Intersection planeIntersection( const Ray& r, const Material& mat );
inline void updateT( float t, const Primitive* primitive, Ray& ray )
{
	if ( t < ray.t )
	{
		ray.t = t;
		ray.primitive = primitive;
	}
}
} // namespace lh2core