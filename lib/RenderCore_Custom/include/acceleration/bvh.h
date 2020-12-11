#pragma once

#include "environment/intersections.h"
#include "environment/primitives.h"
#include "platform.h"
#include <stack>
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

float distanceTo( const Ray& ray, const AABB& box );
class BVHTree
{
  public:
	void traverse( Ray& ray, mat4 transform ) const;
	void traverse( const RayPacket& packet, mat4 transform );
	bool isOccluded( Ray& ray, mat4 transform, float d ) const;
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
	AABB rootCentroidBounds;
	int primitiveCount;
	int poolPtr;
	int depth = 0;
	static bool toLeft( const SplitPlane& plane, const float3& centroid );
};

class TopLevelBVH : public Intersector
{
  public:
	void setPrimitives( Primitive* primitives, int count ) override;
	void intersect( Ray& r ) override;
	void intersectPacket( const RayPacket& packet ) override;
	void packetOccluded( const RayPacket& packet ) override;
	bool isOccluded( Ray& r, float d ) override;

  private:
	BVHTree* tree{};
};

struct SplitResult
{
	AABB left{};
	AABB right{};
	AABB lCentroids{};
	AABB rCentroids{};
	int lCount = 0;
	int rCount = 0;
};

struct Bounds
{
	AABB primitiveBounds;
	AABB centroidBounds;
};
// Bounding boxes
Bounds calculateBounds( Primitive* primitives, const int* indices, float3* centroids, int first, int count );
AABB boundBoth( const AABB& first, const AABB& second );
void updateAABB( AABB& bounds, const Primitive& primitive );
void updateAABB( AABB& bounds, const float3& vertex );

//Misc
float3 calculateCentroid( const Primitive& primitive );
float surfaceArea( const AABB& box );

//Splitting
SplitResult evaluateSplitPlane( const SplitPlane& plane, const BVHTree& tree, int nodeIdx );
inline float sah( const SplitResult& splitResult ) { return surfaceArea( splitResult.left ) * splitResult.lCount + surfaceArea( splitResult.right ) * splitResult.rCount; }

class Heuristic
{
	//	Cost if node is used as leaf
	virtual float leafCost( const BVHTree& tree, int nodeIndex ) = 0;
};

class BVHBuilder
{
  public:
	virtual BVHTree* buildBVH( Primitive* primitives, int count ) = 0;
	static int nodeCount( int primitiveCount ) { return 2 * primitiveCount; }
	static BVHNode* allocateFor( int primitiveCount ) { return new BVHNode[nodeCount( primitiveCount )]; }
};

class SplitPlaneCreator
{
  public:
	virtual bool doSplitPlane( BVHTree* tree, const AABB& centroidBounds, int nodeIdx, SplitPlane& plane, SplitResult& result ) = 0;
};

class OptimalExpensiveSplit : public SplitPlaneCreator
{
  public:
	bool doSplitPlane( BVHTree* tree, const AABB& centroidBounds, int nodeIdx, SplitPlane& plane, SplitResult& result ) override;
};

class BaseBuilder : public BVHBuilder
{
  private:
	SplitPlaneCreator* splitPlaneCreator;

  public:
	explicit BaseBuilder( SplitPlaneCreator* splitPlaneCreator ) : splitPlaneCreator( splitPlaneCreator ){};
	BVHTree* buildBVH( Primitive* primitives, int count ) override;
	void subDivide( BVHTree* tree, const AABB& centroidBounds, int node, int depth );
	void updateTree( BVHTree* tree, int nodeIdx, const SplitPlane& plane, const SplitResult& best ) const;
};
} // namespace lh2core