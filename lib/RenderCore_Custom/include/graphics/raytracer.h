#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/environment.h"
#include "graphics/lighting.h"
namespace lh2core
{
inline float schlick( float n1, float n2, float cosTheta );

class IRayTracer
{
  public:
	[[nodiscard]] virtual float3 trace( Ray r, int count ) const = 0;
};
class RayTracer : public IRayTracer
{

  public:
	IEnvironment* environment;
	ILighting* lighting;
	RayTracer( IEnvironment* environment, ILighting* lighting ) : environment( environment ), lighting( lighting ){};
	static float3 rayDirection( float u, float v, const ViewPyramid& view );
	[[nodiscard]] float3 trace( Ray r, int count ) const;
	float3 computeGlassColor( const Ray& r, int count, const Intersection& intersection ) const;

  private:
	inline static float3 screenPos( float u, float v, const ViewPyramid& view );
	inline static float3 reflect( const float3& direction, const float3& normal );

	float3 computeDiffuseColor( const Intersection& intersection ) const;
	float3 computeSpecularColor( const Ray& r, int count, const Intersection& intersection ) const;
	float3 traceReflectedRay( const Ray& r, int count, const Intersection& intersection ) const;
	static Ray reflect( const Intersection& intersection, const float3& direction );
};

class PathTracer
{
};
} // namespace lh2core
