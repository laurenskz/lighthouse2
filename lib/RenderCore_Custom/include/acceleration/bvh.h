#pragma once

#include "environment/intersections.h"
#include "environment/primitives.h"
#include "platform.h"

using namespace lighthouse2;
namespace lh2core
{
#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 3
struct AABB
{
	float3 min = make_float3( MAXFLOAT );
	float3 max = make_float3( -MAXFLOAT );
};
class BVHNode
{
  public:
	AABB bounds;
	int leftFirst = -1;
	int count = -1;
	[[nodiscard]] inline bool isLeaf() const { return this->count > 0; }
	[[nodiscard]] inline int leftChild() const { return this->leftFirst; }
	[[nodiscard]] inline bool isUsed() const { return this->leftFirst >= 0; }
	[[nodiscard]] inline int rightChild() const { return this->leftFirst + 1; }
	[[nodiscard]] inline int primitiveIndex() const { return this->leftFirst; }
	[[nodiscard]] inline int splitAxis() const { return -this->count; }
};
struct SplitPlane
{
	int axis;
	float location;
};
class TopLevelBVH : public Intersector
{
  public:
	void setPrimitives( Primitive* primitives, int count ) override;
	void intersect( Ray& r ) override;
	void intersectPacket( const RayPacket& packet ) override;
	void packetOccluded( const RayPacket& packet ) override;
	bool isOccluded( Ray& r, float d ) override;
};

class BVHTree
{
  public:
	void traverse( Ray& ray, mat4 transform );
	void traverse( const RayPacket& packet, mat4 transform );
	void isOccluded( Ray& ray, mat4 transform, float d );
	void isOccluded( const RayPacket& packet, mat4 transform, float d );
	void refit( Primitive* newPrimitives );
	void reorder( const SplitPlane& plane, int start, int count );
	BVHTree( Primitive* primitives, int primitiveCount );
	~BVHTree();
	BVHNode* nodes;
	int nodeCount;
	int* primitiveIndices;
	Primitive* primitives;
	float3* centroids;
	int primitiveCount;
	int poolPtr;
	static bool toLeft( const SplitPlane& plane, const float3& centroid );
};

struct SplitResult
{
	AABB left{};
	AABB right{};
	int lCount = 0;
	int rCount = 0;
};
// Bounding boxes
AABB calculateBounds( Primitive* primitives, const int* indices, int first, int count );
AABB boundBoth( const AABB& first, const AABB& second );
void updateAABB( AABB& bounds, const Primitive& primitive );

//Misc
float3 calculateCentroid( const Primitive& primitive );
float surfaceArea( const AABB& box );



SplitResult evaluateSplitPlane( const SplitPlane& plane, const BVHTree& tree, int nodeIdx );
inline float sah( const SplitResult& splitResult ) { return surfaceArea( splitResult.left ) * splitResult.lCount + surfaceArea( splitResult.right ) * splitResult.rCount; }
class Heuristic
{
	//	Cost if node is used as leaf
	virtual float leafCost( const BVHTree& tree, int nodeIndex ) = 0;
};

class Partitioner
{
	//
	virtual float splitPosition( const BVHNode& node ) = 0;
};
class BVHBuilder
{
  public:
	virtual BVHTree* buildBVH( Primitive* primitives, int count ) = 0;
	static int nodeCount( int primitiveCount ) { return 2 * primitiveCount; }
	static BVHNode* allocateFor( int primitiveCount ) { return new BVHNode[nodeCount( primitiveCount )]; }
};

class BaseBuilder : public BVHBuilder
{
  public:
	BVHTree* buildBVH( Primitive* primitives, int count ) override;
	void subDivide( BVHTree* tree, int node );
	bool doSplitPlane( BVHTree* tree, int nodeIdx, SplitPlane& plane, SplitResult& result );
	void updateTree( BVHTree* tree, BVHNode& node, const SplitPlane& plane, const SplitResult& best ) const;
};
} // namespace lh2core