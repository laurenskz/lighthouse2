
#include "guiding/utils.h"
namespace lh2core
{

float randFloat()
{
	return static_cast<float>( rand() ) / static_cast<float>( RAND_MAX );
}
float3 projectIntoWorldSpace( const float3& direction, const float3& normal )
{
	float3 nt, nb;
	if ( std::fabs( normal.x ) > std::fabs( normal.y ) )
		nt = make_float3( normal.z, 0, -normal.x ) / sqrtf( normal.x * normal.x + normal.z * normal.z );
	else
		nt = make_float3( 0, -normal.z, normal.y ) / sqrtf( normal.y * normal.y + normal.z * normal.z );
	nb = cross( normal, nt );
	return make_float3(
		direction.x * nb.x + direction.y * normal.x + direction.z * nt.x,
		direction.x * nb.y + direction.y * normal.y + direction.z * nt.y,
		direction.x * nb.z + direction.y * normal.z + direction.z * nt.z );
}

float3 cosineSampleHemisphere()
{
	float u1, u2;
	do {
		u1 = randFloat();
		u2 = randFloat();
	} while ( sqrt( u1 * u1 + u2 * u2 ) > 1 );
	const float r = sqrt( u1 );
	const float theta = 2 * PI * u2;

	const float x = r * cos( theta );
	const float y = r * sin( theta );

	return make_float3( x, sqrt( max( 0.0f, 1 - u1 ) ), y );
}
} // namespace lh2core