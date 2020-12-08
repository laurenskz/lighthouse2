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
	Distance minDistance;
	Primitive* primitive{};
};
struct RayPacket
{
	//	Data about rays and which to trace
	int rayCount;
	Ray* rays;
	//	Result intersections
	Intersection* intersections;
	//	Result colors after tracing this packet
	float3* results;
	//	For occlusions
	bool* occlusions;
	float* occlusionDistances;
};
class Intersector
{
  public:
	virtual void setPrimitives( Primitive* primitives, int count ) = 0;
	virtual void intersect( Ray& r ) = 0;
	virtual void intersectPacket( const RayPacket& packet ) = 0;
	virtual void packetOccluded( const RayPacket& packet ) = 0;
	virtual bool isOccluded( Ray& r, float d ) = 0;
};

class BruteForceIntersector : public Intersector
{
  private:
	Primitive* primitives{};
	int count{};

  public:
	void setPrimitives( Primitive* newPrimitives, int newCount ) override;
	bool isOccluded( Ray& r, float d ) override;
	void intersect( Ray& r ) override;
	void intersectPacket( const RayPacket& packet ) override;
	void packetOccluded( const RayPacket& packet ) override;
};



} // namespace lh2core