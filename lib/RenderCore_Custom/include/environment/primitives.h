#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"

namespace lh2core
{
#define SET_BIT( pos ) ( (uint)1 << ( pos ) )
const uint SPHERE_BIT = (uint)1 << (uint)0;
const uint TRIANGLE_BIT = (uint)1 << (uint)1;
const uint PLANE_BIT = (uint)1 << (uint)2;

struct Primitive
{
	uint flags;
	float3 v1;
	float3 v2;
	float3 v3;
	int meshIndex;		// The index of the mesh
	int triangleNumber; //Number of this triangle inside the mesh
	int instanceIndex; //The transformation
};

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

inline float3 locationAt( float t, Ray r ) { return r.start + t * r.direction; };

Distance distanceToPrimitive( const Primitive& primitive, Ray r );
float distanceToSphere( const Primitive& primitive, Ray r );
float distanceToSphereFromInside(const Primitive& primitive, Ray r);
Distance distanceToTriangle( const Primitive& primitive, Ray r );
float distanceToPlane( const Primitive& primitive, Ray r );

Intersection sphereIntersection( const Primitive& primitive, const Material& mat, Ray r, float t );
Intersection planeIntersection( const Primitive& primitive, const Material& mat, Ray r, float t );
} // namespace lh2core