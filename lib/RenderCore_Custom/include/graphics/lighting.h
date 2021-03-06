#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/intersections.h"
namespace lh2core
{

class ILighting
{
  public:
	virtual float directIllumination( const float3& pos, float3 normal ) = 0;
};
class TestLighting : public ILighting
{
  public:
	float directIllumination( const float3& pos, float3 normal ) override;
};
class Lighting : public ILighting
{
  public:
	void SetLights( const CoreLightTri* triLights, const int triLightCount,
					const CorePointLight* pointLights, const int pointLightCount,
					const CoreSpotLight* spotLights, const int spotLightCount,
					const CoreDirectionalLight* directionalLights, const int directionalLightCount );
	float directIllumination( const float3& pos, float3 normal ) override;
	Lighting( Intersector* intersector ) : intersector( intersector ){};

  private:
	Intersector* intersector;
	const CorePointLight* pointLights;
	int pointLightCount = 0;
	const CoreDirectionalLight* directionalLights;
	int directionalLightCount = 0;
	const CoreSpotLight* spotLights;
	int spotLightCount = 0;

	float illuminationFrom( const CorePointLight& light, const float3& pos, const float3& normal );
	float illuminationFrom( const CoreSpotLight& light, const float3& pos, const float3& normal );
	float illuminationFrom( const CoreDirectionalLight& light, const float3& pos, const float3& normal );
};
} // namespace lh2core