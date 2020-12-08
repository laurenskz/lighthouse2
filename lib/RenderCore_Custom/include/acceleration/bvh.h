#pragma once

#include "platform.h"
#include "environment/primitives.h"
#include "environment/intersections.h"

using namespace lighthouse2;
namespace lh2core
{
struct AABB
{
	float3 min;
	float3 max;
};
class BVHNode
{
	AABB bounds;
	int leftFirst;
	int count;
	[[nodiscard]] inline bool isLeaf() const { return this->count > 0; }
	[[nodiscard]] inline int leftChild() const { return this->leftFirst; }
	[[nodiscard]] inline int rightChild() const { return this->leftFirst + 1; }
	[[nodiscard]] inline int primitiveIndex() const { return this->leftFirst; }
	[[nodiscard]] inline int splitAxis() const { return -this->count; }
};

class TopLevelBVH : public Intersector{

};

class BVHTree : public Intersector
{
  public:
	ShortestDistance traverse(Ray* rays, mat4 transform);
	void rebuild(Primitive* newPrimitives);
  private:
	BVHNode* nodes;
	int nodeCount;
	int* primitiveIndices;
	Primitive* primitives;
	int primitiveCount;

};
} // namespace lh2core