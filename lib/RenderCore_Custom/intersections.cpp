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
Mesh::Mesh( int vertexCount )
{
	normals = new float3[vertexCount];
	positions = new float3[vertexCount];
	triangleCount = vertexCount / 3;
	this->vertexCount = vertexCount;
}
float Mesh::distanceTo( Ray r, float3 v0, float3 v1, float3 v2 )
{
	float3 v0v1 = v1 - v0;
	float3 v0v2 = v2 - v0;
	float3 triangleNormal = cross( v0v1, v0v2 ); // N

	float NdotRayDirection = dot( triangleNormal, r.direction );
	//	Check if ray and triangle are parallel
	if ( fabs( NdotRayDirection ) < 1e-5 )
		return -1;

	float d = dot( v0, triangleNormal );

	float t = ( dot( triangleNormal, r.start ) + d ) / NdotRayDirection;
	if ( t < 0 ) return -1;

	float3 P = locationAt( t, r );

	float3 C{};

	float3 edge0 = v1 - v0;
	float3 vp0 = P - v0;
	C = cross( edge0, vp0 );
	if ( dot( triangleNormal, C ) < 0 ) return -1;

	float3 edge1 = v2 - v1;
	float3 vp1 = P - v1;
	C = cross( edge1, vp1 );
	if ( dot( triangleNormal, C ) < 0 ) return -1;

	float3 edge2 = v0 - v2;
	float3 vp2 = P - v2;
	C = cross( edge2, vp2 );

	if ( dot( triangleNormal, C ) < 0 ) return -1;

	return t;
}
ShortestDistance Mesh::distanceTo( Ray r ) const
{
	int minIndex = -1;
	float minDistance = -1;
	for ( int i = 0; i < triangleCount; ++i )
	{
		float d = Mesh::distanceTo( r, positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2] );
		if ( d > 0 && ( minDistance < 0 || d < minDistance ) )
		{
			minDistance = d;
			minIndex = i;
		}
	}
	return ShortestDistance{ minDistance, minIndex };
}
Intersection Mesh::intersectionAt( float3 intersectionPoint, int triangleIndex )
{
	float3 v0 = positions[triangleIndex * 3];
	float3 v1 = positions[triangleIndex * 3 + 1];
	float3 v2 = positions[triangleIndex * 3 + 2];
	float3 v0v1 = v1 - v0;
	float3 v0v2 = v2 - v0;
	float3 triangleNormal = cross( v0v1, v0v2 ); // N

	return Intersection{ intersectionPoint, make_float3( 0, 1, 0 ), make_float3( 0, 1, 0 ) };
}

float minPositive( float first, float second )
{
	if(first>0&&second>0)return min(first,second);
	if(first>0)return first;
	if(second>0)return second;
	return -1;
}
} // namespace lh2core