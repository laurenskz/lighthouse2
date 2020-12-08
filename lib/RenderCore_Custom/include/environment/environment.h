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
	virtual Intersection intersect( Ray& r )  = 0;
	virtual void intersectPacket( const RayPacket& rayPacket ) = 0;
};
class TestEnvironment : public IEnvironment
{

  public:
	TestEnvironment( std::vector<Intersection> intersections,
					 std::vector<Ray> rays ) : intersections( std::move( intersections ) ), rays( std::move( rays ) ){};
	Intersection intersect( Ray& r )  override;

  private:
	std::vector<Intersection> intersections;
	std::vector<Ray> rays;
};

class Environment : public IEnvironment
{
  private:
	IGeometry* geometry;
	Intersector* intersector;

  public:
	Environment( IGeometry* geometry,
				 Intersector* intersector ) : geometry( geometry ), intersector( intersector ){};
	Intersection intersect( Ray& r ) override ;
	void intersectPacket( const RayPacket& rayPacket ) override ;
};
} // namespace lh2core