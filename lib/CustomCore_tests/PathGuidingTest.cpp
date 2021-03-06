#include "guiding/PathGuidingTracer.h"
#include "guiding/Tree.h"
#include "gtest/gtest.h"
using namespace lh2core;

#define EXPECT_VEC3_EQ( expected, actual )     \
	ASSERT_NEAR( expected.x, actual.x, 1e-3 ); \
	ASSERT_NEAR( expected.y, actual.y, 1e-3 ); \
	ASSERT_NEAR( expected.z, actual.z, 1e-3 );

#define EXPECT_VEC2_EQ( expected, actual )     \
	ASSERT_NEAR( expected.x, actual.x, 1e-3 ); \
	ASSERT_NEAR( expected.y, actual.y, 1e-3 );

std::ostream& operator<<( std::ostream& os, const float3& s )
{
	return ( os << "{" << s.x << "," << s.y << "," << s.z << "}" );
}

class GuidingFixture : public ::testing::Test
{

  protected:
	void SetUp() override
	{
	}

	void TearDown() override
	{
	}
};

TEST_F( GuidingFixture, TraverseTree )
{
	auto* minX = new SpatialLeaf( new QuadTree( make_float2( 0 ), make_float2( 1 ) ) );
	auto* posXMinY = new SpatialLeaf( new QuadTree( make_float2( 0 ), make_float2( 1 ) ) );
	auto* posXPosY = new SpatialLeaf( new QuadTree( make_float2( 0 ), make_float2( 1 ) ) );
	SpatialNode root = SpatialNode( X,
									SpatialNode::SpatialChild( minX ),
									SpatialNode::SpatialChild(
										new SpatialNode( Y, SpatialNode::SpatialChild( posXMinY ),
														 SpatialNode::SpatialChild( posXPosY ), make_float3( -1 ), make_float3( 1 ) ) ),
									make_float3( -1 ), make_float3( 1 ) );
	ASSERT_EQ( minX, root.lookup( make_float3( -1, 3, 3 ) ) );
	ASSERT_EQ( posXMinY, root.lookup( make_float3( 1, -1, 3 ) ) );
	ASSERT_EQ( posXPosY, root.lookup( make_float3( 1, 1, 3 ) ) );
}

TEST_F( GuidingFixture, ConvertCoordinates )
{
	const float3& input = normalize( make_float3( 1, 0.25, -1 ) );
	const float2& cylindrical = QuadTree::directionToCylindrical( input );
	EXPECT_VEC2_EQ( make_float2( 0.151845, 0.0389896 ), cylindrical )
	const float3& convertedBack = QuadTree::cylindricalToDirection( cylindrical );
	EXPECT_VEC3_EQ( input, convertedBack )
}

void splitAll( int times, QuadTree* tree )
{
	tree->splitLeaf();
	if ( times == 0 ) return;
	splitAll( times - 1, tree->ne );
	splitAll( times - 1, tree->nw );
	splitAll( times - 1, tree->sw );
	splitAll( times - 1, tree->se );
}

TEST_F( GuidingFixture, Traverse )
{
	auto root = new QuadTree( make_float2( 0, 1 ), make_float2( 1, 0 ) );
	root->splitLeaf();
	splitAll( 1, root );
	const float3& direction = normalize( make_float3( 0.5, 0.2, 0.3 ) );
	const float3& direction2 = normalize( make_float3( 0.3, 0.2, 0.3 ) );
	const float2& cylindrical = QuadTree::directionToCylindrical( direction );
	root->depositEnergy( direction, 4 );
	root->depositEnergy( direction2, 1 );
	cout << root->pdf( direction ) * 4 * PI << endl;
	cout << root->pdf( direction2 ) * 4 * PI << endl;
	auto sampled = root->sample();
	cout << root->pdf( sampled ) * 4 * PI << endl;
	cout << root->pdf( normalize( make_float3( 0.3, 0.2, 0.3 ) ) ) << endl;
}

TEST_F( GuidingFixture, TestTree )
{
	const SpatialNode::SpatialChild left = SpatialNode::newLeaf();
	const SpatialNode::SpatialChild right = SpatialNode::newLeaf();
	auto root = new SpatialNode( X, left, right, make_float3( -5 ), make_float3( 5 ) );
	auto result = root->lookup( make_float3( -1, 1, 1 ) );
	result->incrementVisits();
	auto other = root->lookup( make_float3( 1, 1, 1 ) );
	ASSERT_EQ( 0, other->visitCount );
	ASSERT_EQ( 1, result->visitCount );
	root->splitAllAbove( 1 );
	auto minXMinY = root->lookup( make_float3( -1, -1, 0 ) );
	auto minXPosY = root->lookup( make_float3( -1, 1, 0 ) );
	ASSERT_NE( minXMinY, minXPosY );
	ASSERT_EQ( root->lookup( make_float3( 1, -1, 0 ) ), root->lookup( make_float3( 1, 1, 0 ) ) );
	root->splitAllAbove( 0 );
	ASSERT_NE( root->lookup( make_float3( 1, -1, 0 ) ), root->lookup( make_float3( 1, 1, 0 ) ) );
}

TEST_F( GuidingFixture, TestTrainModule )
{
	auto* trainModule = new TrainModule( make_float3( 0 ), make_float3( 1 ), 100 );
	auto brdf = DiffuseBRDF();
	srand( time( NULL ) );
	const Intersection& intersection = Intersection{ make_float3( 0.2 ), make_float3( 0, 1, 0 ), Material{
																									 make_float3( 0, 1, 0 ),
																								 } };
	auto direction = trainModule->sampleDirection(
		intersection,
		brdf, make_float3( 0, 1, 0 ) );
	trainModule->train( make_float3( 0.2 ), direction, 5, 0.7, 0.3 );
	trainModule->completeSample();
	direction = trainModule->sampleDirection(
		intersection,
		brdf, make_float3( 0, 1, 0 ) );
	float totalFlux = 5;
	for ( int i = 0; i < 1; ++i )
	{
		totalFlux = totalFlux * ( 100 * ( i + 1 ) );
		trainModule->train( make_float3( 0.2 ), direction, totalFlux, 0.7, 0.3 );
		trainModule->completeSample();
	}
	double total = 0;
	int iterations = 10000;
	for ( int i = 0; i < iterations; ++i )
	{
		auto nDirection = trainModule->sampleDirection(
			intersection,
			brdf, make_float3( 0, 1, 0 ) );
		float d = dot( nDirection.direction, direction.direction );
		total += d;
	}
	cout << "Avg" << endl;
	cout << ( total / iterations ) << endl;
}
