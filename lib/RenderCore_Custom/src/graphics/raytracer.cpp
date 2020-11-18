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
float3 RayTracer::trace( Ray r ) const
{
	auto intersection = environment->intersect( r );
	auto illumination = lighting->directIllumination( intersection.location, intersection.normal );
	return illumination * intersection.mat.color;
}

inline float3 RayTracer::screenPos( float u, float v, const ViewPyramid& view )
{
	return view.p1 + u * ( view.p2 - view.p1 ) + v * ( view.p3 - view.p1 );
}
