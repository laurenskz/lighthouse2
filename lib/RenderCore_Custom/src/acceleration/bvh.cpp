#include "acceleration/bvh.h"
namespace lh2core
{

void TopLevelBVH::setPrimitives( Primitive* primitives, int count )
{
	auto* splitter = new OptimalExpensiveSplit();
	auto* builder = new BaseBuilder( splitter );
	tree = builder->buildBVH( primitives, count );
	delete splitter;
	delete builder;
}
void TopLevelBVH::intersect( Ray& r )
{
	tree->traverse( r, mat4::Identity() );
}
void TopLevelBVH::intersectPacket( const RayPacket& packet )
{
}
void TopLevelBVH::packetOccluded( const RayPacket& packet )
{
}
bool TopLevelBVH::isOccluded( Ray& r, float d )
{
	return tree->isOccluded( r, mat4::Identity(), d );
}
BVHTree* BaseBuilder::buildBVH( Primitive* primitives, int count )
{
	auto* tree = new BVHTree( primitives, count );
	subDivide( tree, tree->rootCentroidBounds, 0, 1 );
	return tree;
}
void BaseBuilder::subDivide( BVHTree* tree, const AABB& centroidBounds, int nodeIdx, int depth )
{
	if ( tree->depth < depth ) tree->depth = depth;
	BVHNode& node = tree->nodes[nodeIdx];
	SplitPlane plane{};
	SplitResult best{};
	if ( splitPlaneCreator->doSplitPlane( tree, centroidBounds, nodeIdx, plane, best ) )
	{
		updateTree( tree, nodeIdx, plane, best );
		subDivide( tree, best.lCentroids, node.leftChild(), depth + 1 );
		subDivide( tree, best.rCentroids, node.rightChild(), depth + 1 );
	}
}
void BaseBuilder::updateTree( BVHTree* tree, int nodeIdx, const SplitPlane& plane, const SplitResult& best ) const
{
	int leftChildPrimitivePointer = tree->nodes[nodeIdx].leftFirst;
	tree->reorder( plane, tree->nodes[nodeIdx].leftFirst, tree->nodes[nodeIdx].count );
	int left = tree->poolPtr;
	tree->poolPtr += 2;
	tree->nodes[nodeIdx].leftFirst = left;
	tree->nodes[nodeIdx].count = -plane.axis;
	tree->nodes[tree->nodes[nodeIdx].leftChild()].bounds = best.left;
	tree->nodes[tree->nodes[nodeIdx].rightChild()].bounds = best.right;
	tree->nodes[tree->nodes[nodeIdx].leftChild()].count = best.lCount;
	tree->nodes[tree->nodes[nodeIdx].rightChild()].count = best.rCount;
	tree->nodes[tree->nodes[nodeIdx].leftChild()].leftFirst = leftChildPrimitivePointer;
	tree->nodes[tree->nodes[nodeIdx].rightChild()].leftFirst = leftChildPrimitivePointer + best.lCount;
}

BVHTree::BVHTree( Primitive* primitives, int primitiveCount )
{
	this->nodeCount = 2 * primitiveCount;
	this->nodes = new BVHNode[nodeCount];
	this->primitives = primitives;
	this->primitiveCount = primitiveCount;
	this->primitiveIndices = new int[primitiveCount];
	this->centroids = new float3[primitiveCount];
	poolPtr = 2;
	for ( int i = 0; i < primitiveCount; ++i )
	{
		primitiveIndices[i] = i;
		centroids[i] = calculateCentroid( primitives[i] );
	}
	const Bounds& bounds = calculateBounds( primitives, primitiveIndices, centroids, 0, primitiveCount );
	rootCentroidBounds = bounds.centroidBounds;
	nodes[0].bounds = bounds.primitiveBounds;
	nodes[0].count = primitiveCount;
	nodes[0].leftFirst = 0;
}
BVHTree::~BVHTree()
{
	delete[] this->nodes;
	delete this->primitiveIndices;
}
void BVHTree::refit( Primitive* newPrimitives )
{
	primitives = newPrimitives;
	for ( int i = nodeCount - 1; i >= 0; --i )
	{
		if ( !nodes[i].isUsed() ) continue;
		if ( nodes[i].isLeaf() )
		{
			nodes[i].bounds = calculateBounds( primitives, primitiveIndices, centroids, nodes[i].leftFirst, nodes[i].count ).primitiveBounds;
		}
		else
		{
			auto left = nodes[nodes[i].leftChild()];
			auto right = nodes[nodes[i].rightChild()];
			nodes[i].bounds = boundBoth( left.bounds, right.bounds );
		}
	}
}
void BVHTree::reorder( const SplitPlane& plane, int start, int count )
{
	int i = start;
	for ( int j = start; j < start + count; ++j )
	{
		const float3& centroid = centroids[primitiveIndices[j]];
		if ( BVHTree::toLeft( plane, centroid ) )
		{
			int temp = primitiveIndices[i];
			primitiveIndices[i] = primitiveIndices[j];
			primitiveIndices[j] = temp;
			i++;
		}
	}
}
bool BVHTree::toLeft( const SplitPlane& plane, const float3& centroid ) { return plane.axis == AXIS_X ? centroid.x <= plane.location : plane.axis == AXIS_Y ? centroid.y <= plane.location
																																							: plane.axis == AXIS_Z && centroid.z <= plane.location; }
void BVHTree::traverse( Ray& ray, mat4 transform ) const
{
	int stackPtr = 0;
	int traverselStack[depth];
	traverselStack[stackPtr] = 0;
	int evaluations = 0;
	while ( stackPtr >= 0 )
	{
		int nodeIdx = traverselStack[stackPtr--];
		auto node = nodes[nodeIdx];
		float boundDistance = distanceTo( ray, node.bounds );
		if ( !node.isUsed() || ray.t <= boundDistance )
		{
			continue;
		}
		evaluations++;
		if ( node.isLeaf() )
		{
			for ( int i = node.primitiveIndex(); i < node.primitiveIndex() + node.count; ++i )
			{
				intersectPrimitive( &primitives[primitiveIndices[i]], ray );
			}
		}
		else
		{
			bool leftIsNear = node.splitAxis() == AXIS_X ? ray.direction.x > 0 : node.splitAxis() == AXIS_Y ? ray.direction.y > 0
																											: node.splitAxis() == AXIS_Z && ray.direction.z > 0;
			int near = leftIsNear ? node.leftChild() : node.rightChild();
			int far = leftIsNear ? node.rightChild() : node.leftChild();
			traverselStack[++stackPtr] = far;
			traverselStack[++stackPtr] = near;
		}
	}
	cout << evaluations << endl;
}
bool BVHTree::isOccluded( Ray& ray, mat4 transform, float d ) const
{
	traverse( ray, transform );
	return ray.t < d;
}
Bounds calculateBounds( Primitive* primitives, const int* indices, float3* centroids, int first, int count )
{
	Bounds bounds;
	for ( int i = first; i < first + count; ++i )
	{
		updateAABB( bounds.primitiveBounds, primitives[indices[i]] );
		updateAABB( bounds.centroidBounds, centroids[indices[i]] );
	}
	return bounds;
}
AABB boundBoth( const AABB& first, const AABB& second )
{
	return AABB{ fminf( first.min, second.min ), fmaxf( first.max, second.max ) };
}
void updateAABB( AABB& bounds, const Primitive& primitive )
{
	if ( isTriangle( primitive ) )
	{
		bounds.min = fminf( primitive.v1, bounds.min );
		bounds.min = fminf( primitive.v2, bounds.min );
		bounds.min = fminf( primitive.v3, bounds.min );
		bounds.max = fmaxf( primitive.v1, bounds.max );
		bounds.max = fmaxf( primitive.v2, bounds.max );
		bounds.max = fmaxf( primitive.v3, bounds.max );
	}
	if ( isSphere( primitive ) )
	{
		bounds.min = fminf( primitive.v1 - make_float3( primitive.v2.y ), bounds.min );
		bounds.max = fmaxf( primitive.v1 + make_float3( primitive.v2.y ), bounds.max );
	}
}

void updateAABB( AABB& bounds, const float3& vertex )
{
	bounds.min = fminf( vertex, bounds.min );
	bounds.max = fmaxf( vertex, bounds.max );
}
float surfaceArea( const AABB& box )
{
	auto sizes = box.max - box.min;
	return 2 * ( sizes.x * sizes.y + sizes.x * sizes.z + sizes.y * sizes.z );
}
float3 calculateCentroid( const Primitive& primitive )
{
	if ( isTriangle( primitive ) )
	{

		return make_float3(
			( primitive.v1.x + primitive.v2.x + primitive.v3.x ) / 3,
			( primitive.v1.y + primitive.v2.y + primitive.v3.y ) / 3,
			( primitive.v1.z + primitive.v2.z + primitive.v3.z ) / 3 );
	}
	if ( isSphere( primitive ) )
	{
		return primitive.v1;
	}
	return make_float3( 0 );
}
SplitResult evaluateSplitPlane( const SplitPlane& plane, const BVHTree& tree, int nodeIdx )
{
	SplitResult result{};
	const BVHNode& node = tree.nodes[nodeIdx];
	for ( int i = node.primitiveIndex(); i < node.primitiveIndex() + node.count; ++i )
	{
		int primitiveIndex = tree.primitiveIndices[i];
		if ( BVHTree::toLeft( plane, tree.centroids[primitiveIndex] ) )
		{
			updateAABB( result.left, tree.primitives[primitiveIndex] );
			updateAABB( result.lCentroids, tree.centroids[primitiveIndex] );
			result.lCount++;
		}
		else
		{
			updateAABB( result.right, tree.primitives[primitiveIndex] );
			updateAABB( result.rCentroids, tree.centroids[primitiveIndex] );
			result.rCount++;
		}
	}
	return result;
}
float distanceTo( const Ray& ray, const AABB& box )
{
	// r.dir is unit direction vector of ray
	float3 dirfrac = 1.0f / ray.direction;
	// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
	// r.org is origin of ray
	float t1 = ( box.min.x - ray.start.x ) * dirfrac.x;
	float t2 = ( box.max.x - ray.start.x ) * dirfrac.x;
	float t3 = ( box.min.y - ray.start.y ) * dirfrac.y;
	float t4 = ( box.max.y - ray.start.y ) * dirfrac.y;
	float t5 = ( box.min.z - ray.start.z ) * dirfrac.z;
	float t6 = ( box.max.z - ray.start.z ) * dirfrac.z;

	float tmin = max( max( min( t1, t2 ), min( t3, t4 ) ), min( t5, t6 ) );
	float tmax = min( min( max( t1, t2 ), max( t3, t4 ) ), max( t5, t6 ) );

	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if ( tmax < 0 )
	{
		return MAX_DISTANCE;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if ( tmin > tmax )
	{
		return MAX_DISTANCE;
	}
	return tmin;
}

bool OptimalExpensiveSplit::doSplitPlane( BVHTree* tree, const AABB& centroidBounds, int nodeIdx, SplitPlane& plane, SplitResult& result )
{
	auto node = tree->nodes[nodeIdx];
	auto cost = surfaceArea( node.bounds ) * node.count;
	bool split = false;
	for ( int i = node.leftFirst; i < node.leftFirst + node.count; ++i )
	{
		const float3& centroid = tree->centroids[tree->primitiveIndices[i]];
		for ( int axis = 1; axis <= 3; ++axis )
		{
			auto splitPlanePosition = SplitPlane{ axis, axis == AXIS_X ? centroid.x : axis == AXIS_Y ? centroid.y
																				  : axis == AXIS_Z	 ? centroid.z
																									 : -1 };
			auto candidate = evaluateSplitPlane( splitPlanePosition, *tree, nodeIdx );
			float splitCost = sah( candidate );
			if ( splitCost < cost )
			{
				cost = splitCost;
				plane = splitPlanePosition;
				result = candidate;
				split = true;
			}
		}
	}
	return split;
}
} // namespace lh2core
