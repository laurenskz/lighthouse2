#include "acceleration/bvh.h"
#include "gtest/gtest.h"
using namespace lh2core;

#define EXPECT_FLOAT3_EQ( expected, actual )   \
	ASSERT_NEAR( expected.x, actual.x, 1e-3 ); \
	ASSERT_NEAR( expected.y, actual.y, 1e-3 ); \
	ASSERT_NEAR( expected.z, actual.z, 1e-3 );

#define EXPECT_AABB_EQ( expected, actual )        \
	EXPECT_FLOAT3_EQ( expected.min, actual.min ); \
	EXPECT_FLOAT3_EQ( expected.max, actual.max );

std::ostream& operator<<( std::ostream& os, const float3& s )
{
	return ( os << "{" << s.x << "," << s.y << "," << s.z << "}" << std::endl );
}

class BVHFixture : public ::testing::Test
{

  protected:
	void SetUp() override
	{
		primitiveCount = 3;
		primitives = new Primitive[primitiveCount];
		primitives[0] = Primitive{ TRIANGLE_BIT, make_float3( 0 ), make_float3( 1 ), make_float3( 1, 0, 1 ) };
	}

	void TearDown() override
	{
	}

	Primitive* primitives;
	int primitiveCount;
};

TEST_F( BVHFixture, BuildBasics )
{
	//	auto builder = new BaseBuilder();
	//	builder->buildBVH( primitives, primitiveCount );
}

TEST_F( BVHFixture, Centroid )
{
	auto centroid = calculateCentroid( primitives[0] );
	ASSERT_NEAR( 0.6666, centroid.x, 0.01 );
	ASSERT_NEAR( 0.3333, centroid.y, 0.01 );
	ASSERT_NEAR( 0.6666, centroid.z, 0.01 );
}

TEST_F( BVHFixture, Bounds )
{
	Primitive primitives[] = {
		Primitive{ TRIANGLE_BIT, make_float3( -1, 0, 0 ), make_float3( 1 ), make_float3( 1, 0, -10 ) },
		Primitive{ TRIANGLE_BIT, make_float3( 0 ), make_float3( 1 ), make_float3( 1, 0, 1 ) },
		Primitive{ TRIANGLE_BIT, make_float3( 0 ), make_float3( 1 ), make_float3( 1, 0, 1 ) },
		Primitive{ TRIANGLE_BIT, make_float3( 0 ), make_float3( 3, 2, 3 ), make_float3( 1, 0, 1 ) },
	};
	float3 centroids[4];
	int indices[] = { 0, 1, 2, 3 };
	auto bounds = calculateBounds( primitives, indices, centroids, 0, 4 );
	EXPECT_FLOAT3_EQ( make_float3( -1, 0, -10 ), bounds.primitiveBounds.min )
	EXPECT_FLOAT3_EQ( make_float3( 3, 2, 3 ), bounds.primitiveBounds.max )
	int indices2[] = { 0, 2, 3 };
	bounds = calculateBounds( primitives, indices2, centroids, 1, 2 );
	EXPECT_FLOAT3_EQ( make_float3( 0 ), bounds.primitiveBounds.min )
	EXPECT_FLOAT3_EQ( make_float3( 3, 2, 3 ), bounds.primitiveBounds.max )
	updateAABB( bounds.primitiveBounds, primitives[0] );
	EXPECT_FLOAT3_EQ( make_float3( -1, 0, -10 ), bounds.primitiveBounds.min )
	AABB first{ make_float3( 0 ), make_float3( 100 ) }, second{ make_float3( -20 ), make_float3( 0, 1000, 0 ) };
	const AABB& bounded = boundBoth( first, second );
	EXPECT_FLOAT3_EQ( make_float3( -20 ), bounded.min )
	EXPECT_FLOAT3_EQ( make_float3( 100, 1000, 100 ), bounded.max )
}

TEST_F( BVHFixture, SurfaceArea )
{
	float surface = surfaceArea( AABB{ make_float3( 0 ), make_float3( 1, 2, 1 ) } );
	EXPECT_NEAR( 10, surface, 0.001 );
}

TEST_F( BVHFixture, EvaluateSplitPlane )
{
	int count = 3;
	float3 centroids[] = { make_float3( 5, 1, 6 ), make_float3( 0, 2, 3 ), make_float3( -1, 3, 10 ) };
	Primitive primitives[count];
	for ( int i = 0; i < count; ++i )
	{
		primitives[i] = Primitive{ TRIANGLE_BIT, centroids[i], centroids[i], centroids[i] };
	}
	BVHTree* tree = new BVHTree( primitives, count );
	SplitResult result = evaluateSplitPlane( SplitPlane{ AXIS_Y, 1.5 }, *tree, 0 );
	ASSERT_EQ( 1, result.lCount );
	ASSERT_EQ( 2, result.rCount );
	EXPECT_AABB_EQ( ( AABB{ make_float3( 5, 1, 6 ), make_float3( 5, 1, 6 ) } ), result.left );
	EXPECT_AABB_EQ( ( AABB{ make_float3( -1, 2, 3 ), make_float3( 0, 3, 10 ) } ), result.right );
	result = evaluateSplitPlane( SplitPlane{ AXIS_X, 1 }, *tree, 0 );
	ASSERT_EQ( 2, result.lCount );
	ASSERT_EQ( 1, result.rCount );
	EXPECT_AABB_EQ( ( AABB{ make_float3( -1, 2, 3 ), make_float3( 0, 3, 10 ) } ), result.left );
	EXPECT_AABB_EQ( ( AABB{ make_float3( 5, 1, 6 ), make_float3( 5, 1, 6 ) } ), result.right );
	result = evaluateSplitPlane( SplitPlane{ AXIS_Z, 3 }, *tree, 0 );
	ASSERT_EQ( 1, result.lCount );
	ASSERT_EQ( 2, result.rCount );
	EXPECT_AABB_EQ( ( AABB{ make_float3( 0, 2, 3 ), make_float3( 0, 2, 3 ) } ), result.left );
	EXPECT_AABB_EQ( ( AABB{ make_float3( -1, 1, 6 ), make_float3( 5, 3, 10 ) } ), result.right );
}

TEST_F( BVHFixture, Reorder )
{
	int count = 5;
	float3 centroids[] = { make_float3( 5, 1, 6 ), make_float3( 1, 2, 3 ), make_float3( -1, 3, 10 ), make_float3( -2, 0, 0 ), make_float3( -2 ) };
	Primitive primitives[count];
	for ( int i = 0; i < count; ++i )
	{
		primitives[i] = Primitive{ TRIANGLE_BIT, centroids[i], centroids[i], centroids[i] };
	}
	BVHTree* tree = new BVHTree( primitives, count );
	const SplitPlane& plane = SplitPlane{ AXIS_X, -1 };
	SplitResult result = evaluateSplitPlane( SplitPlane{ AXIS_Y, 1.5 }, *tree, 0 );
	tree->reorder( plane, 0, count );
	cout << "Left: " << result.lCount << ", right: " << result.rCount << endl;
	for ( int i = 0; i < count; ++i )
	{
		cout << "Primitive index " << i << ": " << tree->primitiveIndices[i] << endl;
	}
	ASSERT_EQ( tree->primitiveIndices[0], 2 );
}

TEST_F( BVHFixture, SplitPlaneCreator )
{
	int count = 3;
	Primitive primitive[] = {
		Primitive{ TRIANGLE_BIT, make_float3( 0 ), make_float3( 0, 1, 1 ), make_float3( 1 ) },
		Primitive{ TRIANGLE_BIT, make_float3( -2 ), make_float3( -1 ), make_float3( -1, 0, 0 ) },
		Primitive{ TRIANGLE_BIT, make_float3( -7 ), make_float3( -6 ), make_float3( -5 ) } };
	BVHTree* tree = new BVHTree( primitive, count );
	auto split = new OptimalExpensiveSplit();
	SplitPlane plane{};
	SplitResult result{};
	auto doSplit = split->doSplitPlane( tree, 0, plane, result );
	ASSERT_TRUE( doSplit );
	ASSERT_EQ( 1, result.lCount );
	EXPECT_AABB_EQ( ( AABB{ make_float3( -7 ), make_float3( -5 ) } ), result.left );
	EXPECT_AABB_EQ( ( AABB{ make_float3( -2 ), make_float3( 1 ) } ), result.right );
}

TEST_F( BVHFixture, UpdateTree )
{
	int count = 3;
	Primitive primitive[] = {
		Primitive{ TRIANGLE_BIT, make_float3( 0 ), make_float3( 0, 1, 1 ), make_float3( 1 ) },
		Primitive{ TRIANGLE_BIT, make_float3( -2 ), make_float3( -1 ), make_float3( -1, 0, 0 ) },
		Primitive{ TRIANGLE_BIT, make_float3( -7 ), make_float3( -6 ), make_float3( -5 ) } };
	auto* tree = new BVHTree( primitive, count );
	auto split = new OptimalExpensiveSplit();
	SplitPlane plane{};
	SplitResult result{};
	auto doSplit = split->doSplitPlane( tree, 0, plane, result );
	auto* builder = new BaseBuilder( split );
	BVHNode& root = tree->nodes[0];
	builder->updateTree( tree, root, plane, result );
	EXPECT_AABB_EQ( ( AABB{ make_float3( -7 ), make_float3( -5 ) } ), tree->nodes[root.leftChild()].bounds );
	EXPECT_AABB_EQ( ( AABB{ make_float3( -2 ), make_float3( 1 ) } ), tree->nodes[root.rightChild()].bounds );
	ASSERT_EQ( root.splitAxis(), AXIS_X );
	ASSERT_EQ( tree->primitiveIndices[0], 2 );
	ASSERT_EQ( 1, tree->nodes[root.leftChild()].count );
	ASSERT_EQ( 2, tree->nodes[root.rightChild()].count );
	ASSERT_EQ( 1, tree->nodes[root.rightChild()].primitiveIndex() );
	ASSERT_EQ( 0, tree->nodes[root.leftChild()].primitiveIndex() );
}