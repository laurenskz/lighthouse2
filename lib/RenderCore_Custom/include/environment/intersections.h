//
// Created by laurens on 11/16/20.
//
#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/primitives.h"
namespace lh2core
{
struct ShortestDistance
{
	float minDistance;
	Primitive* primitive;
};

class Intersector
{
  public:
	virtual void setPrimitives( Primitive* primitives, int count ) = 0;
	virtual ShortestDistance nearest( const Ray& r ) = 0;
};

class BruteForceIntersector:public Intersector{
  private:
	Primitive* primitives{};
	int count{};

  public:
	void setPrimitives( Primitive* primitives, int count ) override;
	ShortestDistance nearest( const Ray& r ) override;
};

} // namespace lh2core