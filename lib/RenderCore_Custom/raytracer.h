//
// Created by laurens on 11/12/20.
//

#pragma once
namespace lh2core
{
struct Ray
{
	float d;
};

class RayTracer
{

  public:
	static float3 rayDirection( float u, float v, const ViewPyramid& view );
	float3 trace( float3 start, float3 direction );

  private:
	inline static float3 screenPos( float u, float v, const ViewPyramid& view );
};
} // namespace lh2core

