#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/geometry.h"
#include "environment/intersections.h"
namespace lh2core
{
class Environment
{
  private:
	Geometry* geometry;
	Intersector* intersector;

  public:
	Environment( Geometry* geometry,
				 Intersector* intersector ) : geometry( geometry ), intersector( intersector ){};
	Intersection intersect( const Ray& r );
};
} // namespace lh2core