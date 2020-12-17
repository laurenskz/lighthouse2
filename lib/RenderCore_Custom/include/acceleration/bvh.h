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
AABB operator*( const mat4& a, const AABB& bounds );

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

class TLBVHNode
{
  public:
	AABB bounds;
	int leftFirst = -1;
	int right = -1;
	[[nodiscard]] inline bool isLeaf() const { return this->right < 0; }
	[[nodiscard]] inline int leftChild() const { return this->leftFirst; }
	[[nodiscard]] inline int rightChild() const { return this->right; }
	[[nodiscard]] inline bool isUsed() const { return this->leftFirst >= 0; }
	[[nodiscard]] inline int treeIndex() const { return this->leftFirst; }
	static TLBVHNode makeParent( AABB bounds, int left, int right ) { return TLBVHNode{ bounds, left, right }; }
	static TLBVHNode makeLeaf( AABB bounds, int treeIndex ) { return TLBVHNode{ bounds, treeIndex, -1 }; }
};
struct SplitPlane
{
	int axis;
	float location;
};

float distanceTo( const Ray& ray, const AABB& box );

template <class Derived, class Node>
class BaseBVHTree
{
  public:
	void traverse( Ray& ray ) const;
	bool isOccluded( Ray& ray, float d ) const;
	Node* nodes;
	int nodeCount;
	int poolPtr;
	int depth = 0;
	[[nodiscard]] inline AABB bounds() const { return nodes[0].bounds; }
};

class BVHTree : public BaseBVHTree<BVHTree, BVHNode>
{
  public:
	void refit( Primitive* newPrimitives );
	void reorder( const SplitPlane& plane, int start, int count );
	BVHTree( Primitive* primitives, int primitiveCount );
	~BVHTree();
	int* primitiveIndices;
	Primitive* primitives;
	float3* centroids;
	AABB rootCentroidBounds;
	int primitiveCount;
	int poolPtr;
	inline bool leftIsNear( const BVHNode& node, const Ray& ray ) const;
	inline void visitLeaf( const BVHNode& node, Ray& ray ) const;
	static bool toLeft( const SplitPlane& plane, const float3& centroid );
};

struct TLInstance
{
	mat4 transform;
	mat4 inverted;
	int instanceIndex;
	BVHTree* tree;
};

class TLBVHTree final : public BaseBVHTree<TLBVHTree, TLBVHNode>
{
  private:
	const std::vector<TLInstance>& instances;

  public:
	explicit TLBVHTree( const std::vector<TLInstance>& instances );
	inline bool leftIsNear( const TLBVHNode& node, const Ray& ray ) const;
	inline void visitLeaf( const TLBVHNode& node, Ray& ray ) const;
	inline void leafOccluded( const TLBVHNode& node, Ray& ray, float d ) const;
};

class TopLevelBVH : public Intersector
{
  public:
	void setPrimitives( Primitive* primitives, int count ) override;
	void intersect( Ray& r ) override;
	bool isOccluded( Ray& r, float d ) override;
	static inline int findBestMatch( int node, int count, const int* nodes, TLBVHTree* tree );
	void setMesh( int meshIndex, Primitive* primitives, int count );
	void setInstance( int instanceIndex, int meshIndex, const mat4& transform );
	void finalize();

  private:
	bool isDirty = false;
	std::vector<TLInstance> instances{};
	std::vector<BVHTree*> trees{};
	TLBVHTree* tlBVH{};
	TLBVHTree* buildTopLevelBVH();
	void mergeNodes( const TLBVHTree* newTree, int* nodeList, int* depths, int a, int b, int& poolPtr, int& mergedCount ) const;
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

class BinningSplit : public SplitPlaneCreator
{
  private:
	int binCount;

  public:
	explicit BinningSplit( int count ) : binCount( count ){};
	bool doSplitPlane( BVHTree* tree, const AABB& centroidBounds, int nodeIdx, SplitPlane& plane, SplitResult& result ) override;
	[[nodiscard]] SplitPlane splitPlaneFromCentroid( const float3& centroid, int axis ) const;
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