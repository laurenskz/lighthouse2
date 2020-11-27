#include "graphics/lighting.h"
namespace lh2core
{

float Lighting::illuminationFrom( const CorePointLight& light, const float3& pos, const float3& normal )
{
	const float3& fromLightVector = pos - light.position;
	float d = length( fromLightVector );
	const float3& directionFromLight = normalize( fromLightVector );
	Ray shadowRay = Ray{ light.position, directionFromLight };
	if ( intersector->isOccluded( shadowRay, d - 1e-3 ) ) return 0; //occluded
	float lightnormal = clamp( dot( ( -directionFromLight ), normal ), 0.0, 1.0 );
	return lightnormal * light.energy / ( d * d );
}
float Lighting::directIllumination( const float3& pos, float3 normal )
{
	float illumination = 0;
	for ( int i = 0; i < pointLightCount; ++i )
	{
		illumination += illuminationFrom( pointLights[i], pos, normal );
	}
	for ( int i = 0; i < directionalLightCount; ++i )
	{
		illumination += illuminationFrom( directionalLights[i], pos, normal );
	}
	return clamp( illumination, 0.0, 1.0 );
}
void Lighting::SetLights( const CoreLightTri* triLights, const int triLightCount, const CorePointLight* pointLights, const int pointLightCount, const CoreSpotLight* spotLights, const int spotLightCount, const CoreDirectionalLight* directionalLights, const int directionalLightCount )
{
	this->pointLights = new CorePointLight[pointLightCount];
	memcpy( (void*)this->pointLights, pointLights, pointLightCount * sizeof( CorePointLight ) );
	this->pointLightCount = pointLightCount;

	this->directionalLights = new CoreDirectionalLight[directionalLightCount];
	memcpy( (void*)this->directionalLights, directionalLights, directionalLightCount * sizeof( CoreDirectionalLight ) );
	this->directionalLightCount = directionalLightCount;
}
float Lighting::illuminationFrom( const CoreDirectionalLight& light, const float3& pos, const float3& normal )
{
	auto directionToLight = -light.direction;
	auto shadowRay = Ray{ pos + directionToLight * ( 1e-4 ), directionToLight };
	if ( !intersector->isOccluded( shadowRay, MAX_DISTANCE ) )
	{
		return light.energy * dot( directionToLight, normal );
	}
	return 0;
}
float TestLighting::directIllumination( const float3& pos, float3 normal )
{
	return 1; // Everything is always lit for testing purposes
}
} // namespace lh2core