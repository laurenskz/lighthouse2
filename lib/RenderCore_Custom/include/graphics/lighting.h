#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/intersections.h"
namespace lh2core
{
class Lighting
{
  public:
	void SetLights( const CoreLightTri* triLights, const int triLightCount,
					const CorePointLight* pointLights, const int pointLightCount,
					const CoreSpotLight* spotLights, const int spotLightCount,
					const CoreDirectionalLight* directionalLights, const int directionalLightCount );
	float directIllumination( const float3& pos, float3 normal );
	Lighting( Intersector* intersector ) : intersector( intersector ){};

  private:
	Intersector* intersector;
	const CorePointLight* pointLights;
	int pointLightCount = 0;
	float illuminationFrom( const CorePointLight& light, const float3& pos, const float3& normal );
};
} // namespace lh2core