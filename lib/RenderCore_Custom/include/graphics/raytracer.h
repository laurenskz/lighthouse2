#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/environment.h"
#include "graphics/lighting.h"
namespace lh2core
{
inline float schlick( float n1, float n2, float cosTheta );
void calculateGlass(Ray& reflected,Ray& refracted,float& reflectivityFraction,const Ray& r, const Intersection& intersection);

class IRayTracer
{
  public:
	[[nodiscard]] virtual float3 trace( Ray r, int count ) = 0;
};

class RayTracer : public IRayTracer
{

  public:
	IEnvironment* environment;
	ILighting* lighting;
	RayTracer( IEnvironment* environment, ILighting* lighting ) : environment( environment ), lighting( lighting ){};
	static float3 rayDirection( float u, float v, const ViewPyramid& view );
	[[nodiscard]] float3 trace( Ray r, int count );
	float3 computeGlassColor( const Ray& r, int count, const Intersection& intersection );
	static Ray reflect( const Intersection& intersection, const Ray& r );


  private:
	inline static float3 screenPos( float u, float v, const ViewPyramid& view );
	inline static float3 reflect( const float3& direction, const float3& normal );

	float3 computeDiffuseColor( const Intersection& intersection );
	float3 computeSpecularColor( const Ray& r, int count, const Intersection& intersection );
	float3 traceReflectedRay( const Ray& r, int count, const Intersection& intersection );
};

class PathTracer : public IRayTracer
{
  public:
	IEnvironment* environment;
	ILighting* lighting;
	PathTracer( IEnvironment* environment, ILighting* lighting ) : environment( environment ), lighting( lighting ){};
	float3 trace( Ray r, int count ) override;
	float3 randomDirectionFrom( const float3& normal );
	float3 randomHemisphereDirection();

  private:
	std::uniform_real_distribution<> dis{ 0, 1 };
	float randFloat( float min, float max );
};
} // namespace lh2core
