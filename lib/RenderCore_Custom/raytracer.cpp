//
// Created by laurens on 11/12/20.
//

#include "core_settings.h"

float3 RayTracer::rayDirection( float u, float v, const ViewPyramid& view )
{

	return normalize( ( screenPos( u, v, view ) ) );
}
float3 RayTracer::trace( Ray r )
{
	auto intersection = scene.nearestIntersection( r );
	if ( intersection.location.x == 0 ) return make_float3( 0 );
	auto illumination = scene.directIllumination( intersection.location, intersection.normal );
	return illumination * intersection.mat.color;
}

inline float3 RayTracer::screenPos( float u, float v, const ViewPyramid& view )
{
	return view.p1 + u * ( view.p2 - view.p1 ) + v * ( view.p3 - view.p1 );
}

Scene::Scene()
{
	spheres = new Sphere[1];
	planes = new Plane[1];
	materials = new Material[1];
	materials[0].color = make_float3( 0, 0, 1 );
}
Intersection Scene::nearestIntersection( Ray r )
{
	Intersectable* objects[] = { spheres, planes };
	int counts[] = { 1, 1 };
	return Scene::nearestIntersectionWith( r, objects, 1, counts );
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
	return objects[minObject][minIndex].intersectionAt( r.start + ( minDistance * r.direction ), materials );
}
float Scene::directIllumination( const float3& pos, float3 normal )
{
	return illuminationFrom( make_float3( 1, 2.3, 1 ), 2, pos, normal );
//		illuminationFrom( make_float3( 2, 1, 1 ), 2, pos, normal );
	//		   illuminationFrom( make_float3( 3, 1, 1 ), 0, pos, normal );
}
float Scene::illuminationFrom( const float3& lightSource, float lightIntensity, const float3& pos, const float3& normal )
{
	if ( pos.x == 0 ) return 0;
	const float3& lightDirection = lightSource - pos;
	float d = length( lightDirection );
	float lightnormal = clamp( dot( normalize( lightDirection ), normalize( normal ) ), 0.0, 1.0 );
	return lightIntensity / ( d * d );
}

float Sphere::distanceTo( Ray r )
{
	float3 C = pos - r.start;
	float t = dot( C, r.direction );
	float3 Q = C - t * r.direction;
	float p2 = dot( Q, Q );
	if ( p2 > r2 ) return -1;
	t -= sqr( r2 - p2 );
	return t;
}
Intersection Sphere::intersectionAt( float3 intersectionPoint, Material* materials )
{
	return Intersection{ intersectionPoint, normalize( pos - intersectionPoint ), materials[material] };
}
Intersection Plane::intersectionAt( float3 intersectionPoint, Material* materials )
{
	return Intersection{ intersectionPoint, make_float3( 0, 1, 0 ), materials[0] };
}
float Plane::distanceTo( Ray r )
{
	return -( dot( r.start, direction ) + d ) / ( dot( r.direction, direction ) );
}
