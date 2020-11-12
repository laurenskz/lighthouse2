//
// Created by laurens on 11/12/20.
//

#include "core_settings.h"

float3 RayTracer::rayDirection( float u, float v, const ViewPyramid& view )
{

	return normalize( ( screenPos( u, v, view ) ) );
}
float3 RayTracer::trace( float3 start, float3 direction )
{
	return direction;
}

inline float3 RayTracer::screenPos( float u, float v, const ViewPyramid& view )
{
	return view.p1 + u * ( view.p2 - view.p1 ) + v * ( view.p3 - view.p1 );
}