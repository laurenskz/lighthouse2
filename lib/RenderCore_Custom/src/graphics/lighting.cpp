#include "graphics/lighting.h"
namespace lh2core
{

float Lighting::illuminationFrom( const CorePointLight& light, const float3& pos, const float3& normal )
{
	const float3& fromLightVector = pos - light.position;
	float d = length( fromLightVector );
	const float3& directionFromLight = normalize(fromLightVector);
	Ray shadowRay = Ray{ light.position, directionFromLight };
	const ShortestDistance& nearest = intersector->nearest( shadowRay );
	if ( nearest.minDistance < (d-1e-3)) return 0; //occluded
	float lightnormal = clamp( dot( (-directionFromLight),  normal  ), 0.0, 1.0 );
	return lightnormal * light.energy / ( d * d );
}
float Lighting::directIllumination( const float3& pos, float3 normal )
{
	float illumination = 0;
	for ( int i = 0; i < pointLightCount; ++i )
	{
		illumination += illuminationFrom( pointLights[i], pos, normal );
	}
	return clamp( illumination, 0.0, 1.0 );
}
void Lighting::SetLights( const CoreLightTri* triLights, const int triLightCount, const CorePointLight* pointLights, const int pointLightCount, const CoreSpotLight* spotLights, const int spotLightCount, const CoreDirectionalLight* directionalLights, const int directionalLightCount )
{
	this->pointLights = new CorePointLight[pointLightCount];
	memcpy( (void*)this->pointLights, pointLights, pointLightCount * sizeof( CorePointLight ) );
	this->pointLightCount = pointLightCount;
}
} // namespace lh2core