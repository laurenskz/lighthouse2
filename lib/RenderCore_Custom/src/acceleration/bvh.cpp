#include "acceleration/bvh.h"
namespace lh2core
{

void TopLevelBVH::setPrimitives( Primitive* primitives, int count )
{
}
void TopLevelBVH::intersect( Ray& r )
{
}
void TopLevelBVH::intersectPacket( const RayPacket& packet )
{
}
void TopLevelBVH::packetOccluded( const RayPacket& packet )
{
}
bool TopLevelBVH::isOccluded( Ray& r, float d )
{
	return false;
}
BVHTree* BaseBuilder::buildBVH( Primitive* primitives, int count )
{
	auto* tree = new BVHTree( primitives, count );

	subDivide( tree, 0 );
	return tree;
}
void BaseBuilder::subDivide( BVHTree* tree, int nodeIdx )
{
	auto node = tree->nodes[nodeIdx];
	SplitPlane plane{};
	SplitResult best{};
	if ( doSplitPlane( tree, nodeIdx, plane, best ) )
	{
		updateTree( tree, node, plane, best );
		subDivide( tree, node.leftChild() );
		subDivide( tree, node.rightChild() );
	}
}
void BaseBuilder::updateTree( BVHTree* tree, BVHNode& node, const SplitPlane& plane, const SplitResult& best ) const
{
	int leftChildPrimitivePointer = node.leftFirst;
	tree->reorder( plane, node.leftFirst, node.count );
	int left = tree->poolPtr;
	tree->poolPtr += 2;
	node.leftFirst = left;
	node.count = -plane.axis;
	tree->nodes[node.leftChild()].bounds = best.left;
	tree->nodes[node.rightChild()].bounds = best.right;
	tree->nodes[node.leftChild()].count = best.lCount;
	tree->nodes[node.rightChild()].count = best.rCount;
	tree->nodes[node.leftChild()].leftFirst = leftChildPrimitivePointer;
	tree->nodes[node.rightChild()].leftFirst = leftChildPrimitivePointer + best.lCount;
}
bool BaseBuilder::doSplitPlane( BVHTree* tree, int nodeIdx, SplitPlane& plane, SplitResult& result )
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
BVHTree::BVHTree( Primitive* primitives, int primitiveCount )
{
	this->nodeCount = 2 * primitiveCount;
	this->nodes = new BVHNode[nodeCount];
	this->primitives = primitives;
	this->primitiveCount = primitiveCount;
	this->primitiveIndices = new int[primitiveCount];
	this->centroids = new float3[primitiveCount];
	for ( int i = 0; i < primitiveCount; ++i )
	{
		primitiveIndices[i] = i;
		centroids[i] = calculateCentroid( primitives[i] );
	}
	nodes[0].bounds = calculateBounds( primitives, primitiveIndices, 0, primitiveCount );
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
			nodes[i].bounds = calculateBounds( primitives, primitiveIndices, nodes[i].leftFirst, nodes[i].count );
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
		bool toLeft = BVHTree::toLeft( plane, centroid );
		if ( toLeft )
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
AABB calculateBounds( Primitive* primitives, const int* indices, int first, int count )
{
	AABB bounds;
	for ( int i = first; i < first + count; ++i )
	{
		auto primitive = primitives[indices[i]];
		updateAABB( bounds, primitive );
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
			result.lCount++;
		}
		else
		{
			updateAABB( result.right, tree.primitives[primitiveIndex] );
			result.rCount++;
		}
	}
	return result;
}

} // namespace lh2core
