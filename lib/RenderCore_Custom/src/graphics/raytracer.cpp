//
// Created by laurens on 11/12/20.
//

#include <graphics/raytracer.h>

#include "core_settings.h"
namespace lh2core
{

ostream& operator<<( ostream& os, const float3& s )
{
	return ( os << "{" << s.x << "," << s.y << "," << s.z << "}" << std::endl );
}

float3 RayTracer::rayDirection( float u, float v, const ViewPyramid& view )
{

	return normalize( ( screenPos( u, v, view ) - view.pos ) );
}
float3 RayTracer::trace( Ray r, int count = 3 ) const
{
	if ( count <= 0 ) return BLACK; //Recursion limit
	auto intersection = environment->intersect( r );
	if ( !intersection.hitObject )
		return make_float3( 0.529, 0.808, 0.929 ); //Black if nothing hit
	if ( intersection.mat.type == DIFFUSE )
	{
		return computeDiffuseColor( intersection );
	}
	if ( intersection.mat.type == SPECULAR && intersection.mat.specularity > 0 )
	{
		return computeSpecularColor( r, count, intersection );
	}
	if ( intersection.mat.type == GLASS )
	{
		return computeGlassColor( r, count, intersection );
	}
	return BLACK;
}
float3 RayTracer::computeGlassColor( const Ray& r, int count, const Intersection& intersection ) const
{
	float cosTheta1 = dot( intersection.normal, -r.direction );
	float cosTheta111 = dot( -intersection.normal, -r.direction );
	bool rayEntersObject = cosTheta1 > 0;
	cosTheta1 = rayEntersObject ? cosTheta1 : -cosTheta1;
	auto normal = rayEntersObject ? intersection.normal : -intersection.normal;
	float n1 = rayEntersObject ? 1 : intersection.mat.refractionIndex;
	float n2 = rayEntersObject ? intersection.mat.refractionIndex : 1;
	float n1overn2 = n1 / n2;
	float k = 1 - ( n1overn2 * n1overn2 ) * ( 1 - cosTheta1 );
	float reflectivityFraction = schlick( n1, n2, cosTheta1 );
	const Intersection& newIntersection = Intersection{ intersection.location, normal, intersection.mat, intersection.hitObject };
	if ( k >= 0 ) //Refraction and reflection
	{
		float3 T = n1overn2 * r.direction + normal * ( n1overn2 * cosTheta1 - sqrt( k ) );
		return ( 1 - reflectivityFraction ) * trace( Ray{ intersection.location + 1e-3 * T, T }, count - 1 ) + reflectivityFraction * traceReflectedRay( r, count, newIntersection );
	}
	else
	{
		return traceReflectedRay( r, count, newIntersection );
	}
}

inline float schlick( float n1, float n2, float cosTheta )
{
	float r0 = sqr( ( n1 - n2 ) / ( n1 + n2 ) );
	return r0 + ( 1 - r0 ) * pow( 1 - cosTheta, 5 );
}

float3 RayTracer::computeSpecularColor( const Ray& r, int count, const Intersection& intersection ) const
{
	float3 reflectionColor = traceReflectedRay( r, count, intersection );
	return ( 1 - intersection.mat.specularity ) * computeDiffuseColor( intersection ) +
		   ( intersection.mat.specularity ) *
			   reflectionColor;
}
float3 RayTracer::traceReflectedRay( const Ray& r, int count, const Intersection& intersection ) const
{
	const float3& direction = reflect( r.direction, intersection.normal );
	Ray reflectedRay = reflect( intersection, direction );
	const float3& reflectionColor = trace( reflectedRay, count - 1 );
	return reflectionColor;
}
Ray RayTracer::reflect( const Intersection& intersection, const float3& direction )
{
	const Ray& reflectedRay = Ray{ intersection.location + ( 1e-3 * direction ), direction };
	return reflectedRay;
}
float3 RayTracer::computeDiffuseColor( const Intersection& intersection ) const
{
	auto illumination = lighting->directIllumination( intersection.location, intersection.normal );
	auto diffuseColor = illumination * intersection.mat.color;
	return diffuseColor;
}

inline float3 RayTracer::screenPos( float u, float v, const ViewPyramid& view )
{
	return view.p1 + u * ( view.p2 - view.p1 ) + v * ( view.p3 - view.p1 );
}
float3 RayTracer::reflect( const float3& direction, const float3& normal )
{
	return normalize( direction - ( ( 2 * dot( direction, normal ) ) * normal ) );
}
float3 PathTracer::trace( Ray r, int count ) const
{
	if ( count <= 0 ) return BLACK; //Recursion limit
	auto intersection = environment->intersect( r );

}
} // namespace lh2core
