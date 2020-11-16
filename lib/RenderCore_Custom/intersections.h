//
// Created by laurens on 11/16/20.
//
#pragma once
namespace lh2core
{
struct ShortestDistance
{
	float minDistance;
	int index;
};

class Intersectable
{
  public:
	virtual float distanceTo( Ray r ) = 0;
	virtual Intersection intersectionAt( float3, Material* materials ) = 0;
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

class Plane : public Intersectable
{
  public:
	float3 normal{};
	float d{};
	int material{};
	Plane( float3 normal, float d, int material ) : normal( normal ), d( d ), material( material ) {}
	Plane() = default;
	float distanceTo( Ray r ) override;
	Intersection intersectionAt( float3 intersectionPoint, Material* materials ) override;
};

class Spheres
{
  public:
	Sphere* spheres{};
	Material* materials{};
	int count{};
	ShortestDistance minDistanceTo( Ray r );
	Intersection intersectionWith( int index, float3 origin, float3 intersection );
};
class Planes
{
  public:
	Plane* planes{};
	Material* materials{};
	int count{};
	ShortestDistance minDistanceTo( Ray r );
	Intersection intersectionWith( int index, float3 origin, float3 intersection );
};

class Environment
{
  public:
	Spheres spheres;
	Planes planes;
};

} // namespace lh2core