#include "acceleration/bvh.h"
namespace lh2core
{

void TopLevelBVH::setPrimitives( Primitive* primitives, int count )
{
}
void TopLevelBVH::intersect( Ray& r )
{
	tlBVH->traverse( r );
}
void TopLevelBVH::intersectPacket( const RayPacket& packet )
{
}
void TopLevelBVH::packetOccluded( const RayPacket& packet )
{
}
bool TopLevelBVH::isOccluded( Ray& r, float d )
{
	return tlBVH->isOccluded( r, d );
}
TLBVHTree* TopLevelBVH::buildTopLevelBVH()
{
	auto newTree = new TLBVHTree( instances );
	int poolPtr = newTree->nodeCount - 1;
	int nodeList[instances.size()];
	int depths[newTree->nodeCount];
	int mergedCount = instances.size();
	for ( int i = 0; i < instances.size(); ++i )
	{
		const TLInstance& instance = instances[i];
		int nodeIndex = poolPtr--;
		depths[nodeIndex] = 1;
		newTree->nodes[nodeIndex] = TLBVHNode::makeLeaf( instance.transform * instance.tree->bounds(), i );
		nodeList[i] = nodeIndex;
	}
	int a = 0;
	int b = findBestMatch( a, mergedCount, nodeList, newTree );
	while ( mergedCount > 1 )
	{
		int c = findBestMatch( b, mergedCount, nodeList, newTree );
		if ( a == c )
		{
			mergeNodes( newTree, nodeList, depths, a, b, poolPtr, mergedCount );
			a = 0;
			b = findBestMatch( a, mergedCount, nodeList, newTree );
		}
		else
		{
			a = b;
			b = c;
		}
	}
	newTree->nodes[0] = newTree->nodes[poolPtr + 1];
	newTree->depth = depths[poolPtr + 1];
	return newTree;
}
void TopLevelBVH::mergeNodes( const TLBVHTree* newTree, int* nodeList, int* depths, int a, int b, int& poolPtr, int& mergedCount ) const
{
	int leftChild = nodeList[a];
	int rightChild = nodeList[b];
	int parentIndex = poolPtr--;
	newTree->nodes[parentIndex] = TLBVHNode::makeParent( boundBoth( newTree->nodes[leftChild].bounds, newTree->nodes[rightChild].bounds ), leftChild, rightChild );
	mergedCount--;
	nodeList[a] = parentIndex;
	nodeList[b] = nodeList[mergedCount];
	depths[parentIndex] = max( depths[leftChild], depths[rightChild] ) + 1;
}
int TopLevelBVH::findBestMatch( int nodeIdx, int count, const int* nodes, TLBVHTree* currentTree )
{
	int bestMatch = abs( nodeIdx - 1 );
	float bestCost = surfaceArea( boundBoth( currentTree->nodes[nodes[nodeIdx]].bounds, currentTree->nodes[nodes[bestMatch]].bounds ) );
	for ( int i = 0; i < count; ++i )
	{
		if ( i == nodeIdx || i == bestMatch ) continue;
		float cost = surfaceArea( boundBoth( currentTree->nodes[nodes[nodeIdx]].bounds, currentTree->nodes[nodes[i]].bounds ) );
		if ( cost < bestCost )
		{
			bestCost = cost;
			bestMatch = i;
		}
	}
	return bestMatch;
}
void TopLevelBVH::setMesh( int meshIndex, Primitive* primitives, int count )
{
	if ( meshIndex >= trees.size() )
	{
		auto* splitter = new BinningSplit( 32 );
		auto* builder = new BaseBuilder( splitter );
		trees.push_back( builder->buildBVH( primitives, count ) );
	}
	else
	{
		trees[meshIndex]->refit( primitives );
	}
}
void TopLevelBVH::setInstance( int instanceIndex, int meshIndex, const mat4& transform )
{
	isDirty = true;
	if ( instanceIndex >= instances.size() )
	{
		instances.push_back( TLInstance{ transform, transform.Inverted(), instanceIndex, trees[meshIndex] } );
	}
	else
	{
		instances[instanceIndex].transform = transform;
		instances[instanceIndex].inverted = transform.Inverted();
	}
}
void TopLevelBVH::finalize()
{
	if ( isDirty )
	{
		tlBVH = buildTopLevelBVH();
		isDirty = false;
	}
}
BVHTree* BaseBuilder::buildBVH( Primitive* primitives, int count )
{
	BVHTree* tree = new BVHTree( primitives, count );
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
bool BVHTree::leftIsNear( const BVHNode& node, const Ray& ray ) const
{
	return node.splitAxis() == AXIS_X ? ray.direction.x > 0 : node.splitAxis() == AXIS_Y ? ray.direction.y > 0
																						 : node.splitAxis() == AXIS_Z && ray.direction.z > 0;
}
void BVHTree::visitLeaf( const BVHNode& node, Ray& ray ) const
{
	for ( int i = node.primitiveIndex(); i < node.primitiveIndex() + node.count; ++i )
	{
		intersectPrimitive( &primitives[primitiveIndices[i]], ray );
	}
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
bool BinningSplit::doSplitPlane( BVHTree* tree, const AABB& centroidBounds, int nodeIdx, SplitPlane& plane, SplitResult& result )
{
	auto node = tree->nodes[nodeIdx];
	auto cost = surfaceArea( node.bounds ) * node.count;
	bool split = false;
	auto axisLengths = centroidBounds.max - centroidBounds.min;
	int axis = axisLengths.x >= axisLengths.y && axisLengths.x >= axisLengths.z
				   ? AXIS_X
			   : axisLengths.y >= axisLengths.x && axisLengths.y >= axisLengths.z
				   ? AXIS_Y
				   : AXIS_Z;
	float2 bounds = axis == AXIS_X
						? make_float2( centroidBounds.min.x, centroidBounds.max.x )
					: axis == AXIS_Y
						? make_float2( centroidBounds.min.y, centroidBounds.max.y )
						: make_float2( centroidBounds.min.z, centroidBounds.max.z );
	float binLength = ( bounds.y - bounds.x ) / (float)binCount;
	for ( int bin = 0; bin < binCount; ++bin )
	{
		SplitPlane splitPlanePosition;
		if ( node.count <= binCount )
		{
			if ( bin >= node.count ) break;
			splitPlanePosition = splitPlaneFromCentroid( tree->centroids[tree->primitiveIndices[node.primitiveIndex() + bin]], axis );
		}
		else
		{
			splitPlanePosition = SplitPlane{ axis, bounds.x + (float)bin * binLength };
		}
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
	return split;
}
SplitPlane BinningSplit::splitPlaneFromCentroid( const float3& centroid, int axis ) const
{
	auto splitPlanePosition = SplitPlane{ axis, axis == AXIS_X ? centroid.x : axis == AXIS_Y ? centroid.y
																		  : axis == AXIS_Z	 ? centroid.z
																							 : -1 };
	return splitPlanePosition;
}
template <class Derived, class Node>
void BaseBVHTree<Derived, Node>::traverse( Ray& ray ) const
{
	int stackPtr = 0;
	int traverselStack[depth];
	traverselStack[stackPtr] = 0;
	while ( stackPtr >= 0 )
	{
		int nodeIdx = traverselStack[stackPtr--];
		auto node = nodes[nodeIdx];
		float boundDistance = distanceTo( ray, node.bounds );
		if ( !node.isUsed() || boundDistance < 0 || ray.t <= boundDistance )
		{
			continue;
		}
		if ( node.isLeaf() )
		{
			static_cast<const Derived*>( this )->visitLeaf( node, ray );
		}
		else
		{
			bool leftFirst = static_cast<const Derived*>( this )->leftIsNear( node, ray );
			int near = leftFirst ? node.leftChild() : node.rightChild();
			int far = leftFirst ? node.rightChild() : node.leftChild();
			traverselStack[++stackPtr] = far;
			traverselStack[++stackPtr] = near;
		}
	}
}
template <class Derived, class Node>
bool BaseBVHTree<Derived, Node>::isOccluded( Ray& ray, float d ) const
{
	traverse( ray );
	return ray.t < d;
}
bool TLBVHTree::leftIsNear( const TLBVHNode& node, const Ray& ray ) const
{
	return distanceTo( ray, nodes[node.leftChild()].bounds ) < distanceTo( ray, nodes[node.rightChild()].bounds );
}
void TLBVHTree::visitLeaf( const TLBVHNode& node, Ray& ray ) const
{
	auto tree = instances[node.treeIndex()];
	float t = ray.t;
	float3 pos = ray.start;
	float3 dir = ray.direction;
	ray.start = make_float3( tree.inverted * make_float4( pos, 1 ) );
	ray.direction = normalize( make_float3( tree.inverted * make_float4( dir, 0 ) ) );
	tree.tree->traverse( ray );
	if ( ray.t < t )
	{
		ray.instanceIndex = tree.instanceIndex;
	}
	ray.start = pos;
	ray.direction = dir;
}
AABB operator*( const mat4& mat, const AABB& bounds )
{
	//	We need to apply the transformation to all 8 corners
	AABB newBounds{};
	updateAABB( newBounds, make_float3( mat * make_float4( bounds.min.x, bounds.min.y, bounds.min.z, 1 ) ) );
	updateAABB( newBounds, make_float3( mat * make_float4( bounds.min.x, bounds.min.y, bounds.max.z, 1 ) ) );
	updateAABB( newBounds, make_float3( mat * make_float4( bounds.min.x, bounds.max.y, bounds.min.z, 1 ) ) );
	updateAABB( newBounds, make_float3( mat * make_float4( bounds.min.x, bounds.max.y, bounds.max.z, 1 ) ) );
	updateAABB( newBounds, make_float3( mat * make_float4( bounds.max.x, bounds.min.y, bounds.min.z, 1 ) ) );
	updateAABB( newBounds, make_float3( mat * make_float4( bounds.max.x, bounds.min.y, bounds.max.z, 1 ) ) );
	updateAABB( newBounds, make_float3( mat * make_float4( bounds.max.x, bounds.max.y, bounds.min.z, 1 ) ) );
	updateAABB( newBounds, make_float3( mat * make_float4( bounds.max.x, bounds.max.y, bounds.max.z, 1 ) ) );
	return newBounds;
}

TLBVHTree::TLBVHTree( const vector<TLInstance>& instances ) : instances( instances )
{
	nodeCount = (int)instances.size() * 2;
	nodes = new TLBVHNode[nodeCount];
}
} // namespace lh2core
