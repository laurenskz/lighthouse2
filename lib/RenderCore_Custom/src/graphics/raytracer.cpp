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
	spheres = Spheres();
	spheres.spheres = new Sphere[1];
	spheres.materials = new Material[1];
	spheres.materials[0].color = make_float3( 0, 0, 1 );
	spheres.count = 1;
	spheres.spheres[0] = Sphere( make_float3( 0, -2, 5 ), 1, 0 );
	planes = Planes();
	planes.planes = new Plane[1];
	planes.count = 1;
	planes.planes[0] = Plane( make_float3( 0, 1, 0 ), 4, 1 );
}
Intersection Scene::nearestIntersection( Ray r )
{
	const ShortestDistance& meshDistance = mesh->distanceTo( r );
	const ShortestDistance& sphereDistance = spheres.minDistanceTo( r );
	const ShortestDistance& planeDistance = planes.minDistanceTo( r );
	const float nearest = minPositive( meshDistance.minDistance, minPositive( sphereDistance.minDistance, planeDistance.minDistance ) );
	if ( nearest < 0 ) return Intersection{};
	if ( sphereDistance.minDistance == nearest )
	{
		return spheres.intersectionWith( sphereDistance.index, r.start, locationAt( sphereDistance.minDistance, r ) );
	}
	if ( planeDistance.minDistance == nearest )
	{
		return planes.intersectionWith( planeDistance.index, r.start, locationAt( planeDistance.minDistance, r ) );
	}
	if ( meshDistance.minDistance == nearest )
	{
		return mesh->intersectionAt( locationAt( meshDistance.minDistance, r ), meshDistance.index );
	}

	return Intersection{};
}

float Scene::directIllumination( const float3& pos, float3 normal )
{
	return illuminationFrom( PointLight{ make_float3( 0, 0, 0 ), 35 }, pos, normal );
}
float Scene::illuminationFrom( const PointLight& light, const float3& pos, const float3& normal )
{
	const float3& lightDirection = light.location - pos;
	float d = length( lightDirection );
	float lightnormal = clamp( dot( normalize( lightDirection ), normalize( normal ) ), 0.0, 1.0 );
	return lightnormal * light.intensity / ( d * d );
}
