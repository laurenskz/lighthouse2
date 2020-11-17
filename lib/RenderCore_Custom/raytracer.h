//
// Created by laurens on 11/12/20.
//

#pragma once
namespace lh2core
{

struct PointLight
{
	float3 location;
	float intensity;
};

class Scene
{
  public:
	Scene();
	Spheres spheres;
	Planes planes;
	Mesh* mesh = nullptr;
	Intersection nearestIntersection( Ray r );
	float directIllumination( const float3& pos, float3 normal );
	float illuminationFrom( const PointLight& light, const float3& pos, const float3& normal );
};

class RayTracer
{

  public:
	Scene scene;
	static float3 rayDirection( float u, float v, const ViewPyramid& view );
	float3 trace( Ray r );

  private:
	inline static float3 screenPos( float u, float v, const ViewPyramid& view );
};
} // namespace lh2core
