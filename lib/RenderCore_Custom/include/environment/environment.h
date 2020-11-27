#pragma once
#include <utility>

#include <utility>

#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/geometry.h"
#include "environment/intersections.h"
namespace lh2core
{
class IEnvironment
{
  public:
	virtual Intersection intersect( const Ray& r ) = 0;
};
class TestEnvironment : public IEnvironment
{

  public:
	TestEnvironment( std::vector<Intersection> intersections,
					 std::vector<Ray> rays ) : intersections( std::move( intersections ) ), rays( std::move( rays ) ){};
	Intersection intersect( const Ray& r ) override;

  private:
	std::vector<Intersection> intersections;
	std::vector<Ray> rays;
};

class Environment : public IEnvironment
{
  private:
	Geometry* geometry;
	Intersector* intersector;

  public:
	Environment( Geometry* geometry,
				 Intersector* intersector ) : geometry( geometry ), intersector( intersector ){};
	Intersection intersect( const Ray& r ) override;
};
} // namespace lh2core