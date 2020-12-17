#include "environment/environment.h"
namespace lh2core
{
#define FLOAT_EQ( x, y ) abs( x - y ) < 1e-3;

Intersection TestEnvironment::intersect( Ray& r )
{
	for ( int i = 0; i < rays.size(); ++i )
	{
		if ( abs( r.start.x - rays[i].start.x ) < 1e-3 && abs( r.start.y - rays[i].start.y ) < 1e-3 && abs( r.start.z - rays[i].start.z ) < 1e-3 )
		{
			return intersections[i];
		}
	}
	return Intersection{};
}
Intersection Environment::intersect( Ray& r )
{
	intersector->intersect( r );
	if ( r.t == MAX_DISTANCE )
	{
		return Intersection{};
	}
	auto intersection = geometry->intersectionInformation( r );
	intersection.hitObject = true;
	return intersection;
}
float3 Environment::skyColor( const float3& direction )
{
	float u = 1 + atan2( direction.x, -direction.z ) / PI;
	float v = acos( direction.y ) / PI;
	int x = round( u * skyWidth );
	int y = round( v * skyHeight );
	int index = x + y * skyWidth;
	if ( index > skyWidth * skyHeight ) return BLACK;
	return skyPixels[index];
}
void Environment::SetSkyData( const float3* pixels, const uint width, const uint height )
{
	skyPixels = new float3[width * height];
	skyHeight = height;
	skyWidth = width;
	memcpy( skyPixels, pixels, sizeof( float3 ) * width * height );
}
} // namespace lh2core
