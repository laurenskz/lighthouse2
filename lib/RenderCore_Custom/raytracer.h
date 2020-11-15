//
// Created by laurens on 11/12/20.
//

#pragma once
namespace lh2core
{

struct Ray
{
	float3 start;
	float3 direction;
};

inline float3 locationAt( float t, Ray r ) { return r.start + t * r.direction; };

struct Material
{
	float3 color;
};

struct Intersection
{
	float3 location;
	float3 normal;
	Material mat;
};

class Intersectable
{
  public:
	virtual float distanceTo( Ray r ) = 0;
	virtual Intersection intersectionAt( float3, Material* materials ) = 0;
};

class Plane : public Intersectable
{
	float3 direction = { 0, 1, 0 };
	float d = 0;
	float distanceTo( Ray r ) override;
	Intersection intersectionAt( float3 intersectionPoint, Material* materials ) override;
};

class Sphere : public Intersectable
{

  public:
	float3 pos{};
	float r2{};
	int material{}; //Material index
	Sphere( float3 p, float r, int material ) : pos( p ), r2( r * r ), material( material ) {}
	Sphere() = default;
	float distanceTo( Ray r ) override;
	Intersection intersectionAt( float3 intersectionPoint, Material* materials ) override;
};

class Scene
{
  public:
	Scene();
	Sphere* spheres;
	Plane* planes;
	Material* materials;
	Intersection nearestIntersection( Ray r );
	float directIllumination( const float3& pos, float3 normal );

  private:
	float illuminationFrom( const float3& lightSource, float lightIntensity, const float3& pos, const float3& normal );
	Intersection nearestIntersectionWith( Ray r, Intersectable* objects[], int count, const int counts[] );
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
