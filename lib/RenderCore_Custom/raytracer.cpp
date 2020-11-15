//
// Created by laurens on 11/12/20.
//

#include "core_settings.h"

ostream& operator<<( ostream& os, const float3& s )
{
	return ( os << "{" << s.x << "," << s.y << "," << s.z << "}" << std::endl );
}

float3 RayTracer::rayDirection( float u, float v, const ViewPyramid& view )
{

	return normalize( ( screenPos( u, v, view ) ) );
}
float3 RayTracer::trace( Ray r )
{
	auto intersection = scene.nearestIntersection( r );
	auto illumination = scene.directIllumination( intersection.location, intersection.normal );
	return illumination * intersection.mat.color;
}

inline float3 RayTracer::screenPos( float u, float v, const ViewPyramid& view )
{
	return view.p1 + u * ( view.p2 - view.p1 ) + v * ( view.p3 - view.p1 );
}

Scene::Scene()
{
	sphereCount = 1;
	spheres = new Sphere[sphereCount];
	spheres[0] = Sphere( make_float3( 0, -3, 5 ), 1, 0 );
	planes = new Plane[1];
	planes[0] = Plane( make_float3( 0, 1, 0 ), 4, 1 );
	materials = new Material[2];
	materials[0].color = make_float3( 0, 0, 1 );
	materials[1].color = make_float3( 1, 0, 0 );
}
Intersection Scene::nearestIntersection( Ray r )
{
	Intersectable* objects[] = { spheres, planes };
	int counts[] = { 1, 1 };
	return Scene::nearestIntersectionWith( r, objects, 2, counts );
}
Intersection Scene::nearestIntersectionWith( Ray r, Intersectable* objects[], int count, const int counts[] )
{
	int minObject = 0;
	int minIndex = 0;
	float minDistance = -1;
	for ( int i = 0; i < count; ++i )
	{
		auto array = objects[i];
		for ( int j = 0; j < counts[i]; ++j )
		{
			float d = array[j].distanceTo( r );
			if ( d >= 0 && ( minDistance < 0 || d < minDistance ) )
			{
				minObject = i;
				minIndex = j;
				minDistance = d;
			}
		}
	}
	if ( minDistance < 0 )
	{
		return Intersection();
	}
	auto intersectionLocation = locationAt( minDistance, r );
	return objects[minObject][minIndex].intersectionAt( intersectionLocation, materials );
}
float Scene::directIllumination( const float3& pos, float3 normal )
{
	return illuminationFrom( PointLight{ make_float3( 0, 0, 0 ), 35 }, pos, normal );
//		   illuminationFrom( PointLight{ make_float3( 3, 5, 0 ), 35 }, pos, normal ) ;
//		   illuminationFrom( PointLight{ make_float3( 0, 5, 3 ), 35 }, pos, normal ) +
//		   illuminationFrom( PointLight{ make_float3( 3, 5, 3 ), 35 }, pos, normal );
	//		+ illuminationFrom( make_float3( 8, 6, -3 ), 50, pos, normal );
	//		illuminationFrom( make_float3( -2, -5, -3 ), 10, pos, normal );
	//		illuminationFrom( make_float3( 2, 3, 1 ), 3, pos, normal )+
	//			   illuminationFrom( make_float3( 2, 10, 2 ), 50, pos, normal );
}
float Scene::illuminationFrom( const PointLight& light, const float3& pos, const float3& normal )
{
	const float3& lightDirection = light.location - pos;
	float d = length( lightDirection );
	float lightnormal = clamp( dot( normalize( lightDirection ), normalize( normal ) ), 0.0, 1.0 );
	return lightnormal * light.intensity / ( d * d );
}

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
	return Intersection{ intersectionPoint, normal, materials[material] };
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
