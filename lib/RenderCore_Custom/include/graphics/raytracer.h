#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/environment.h"
#include "graphics/lighting.h"
namespace lh2core
{

class RayTracer
{

  public:
	Environment* environment;
	Lighting* lighting;
	RayTracer( Environment* environment, Lighting* lighting ) : environment( environment ), lighting( lighting ){};
	static float3 rayDirection( float u, float v, const ViewPyramid& view );
	[[nodiscard]] float3 trace( Ray r, int count ) const;


  private:
	inline static float3 screenPos( float u, float v, const ViewPyramid& view );
	inline static float3 reflect(const float3& direction,const float3& normal);
};
} // namespace lh2core
