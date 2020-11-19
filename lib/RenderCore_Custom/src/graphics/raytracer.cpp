//
// Created by laurens on 11/12/20.
//

#include <graphics/raytracer.h>

#include "core_settings.h"

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
	if ( count <= 0 ) return make_float3( 0 );
	auto intersection = environment->intersect( r );
	if ( !intersection.hitObject )
		return make_float3( 0 ); //Black if nothing hit
	auto illumination = lighting->directIllumination( intersection.location, intersection.normal );
	auto diffuseColor = illumination * intersection.mat.color;
	if ( intersection.mat.specularity > 0 )
	{
		const Ray& reflectedRay = Ray{ intersection.location, reflect( r.direction, intersection.normal ) };
		const float3& reflectionColor = trace( reflectedRay, count - 1 );
		return ( 1 - intersection.mat.specularity ) * diffuseColor +
			   ( intersection.mat.specularity ) *
				   reflectionColor;
	}
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
