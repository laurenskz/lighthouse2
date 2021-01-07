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
	SpatialNode root = SpatialNode( 0, X,
									SpatialNode::SpatialChild( minX ),
									SpatialNode::SpatialChild(
										new SpatialNode( 0, Y, SpatialNode::SpatialChild( posXMinY ),
														 SpatialNode::SpatialChild( posXPosY ) ) ) );
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
	//	cout << root->sample() << endl;
	//	cout << root->sample() << endl;
	//	cout << root->sample() << endl;
	//	cout << root->sample() << endl;
	//	cout << root->sample() << endl;
}
