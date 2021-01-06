#include "guiding/Tree.h"
#include "gtest/gtest.h"
using namespace lh2core;

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
	auto* minX = new SpatialLeaf();
	auto* posXMinY = new SpatialLeaf();
	auto* posXPosY = new SpatialLeaf();
	SpatialNode root = SpatialNode( 0, X,
									SpatialNode::SpatialChild( minX ),
									SpatialNode::SpatialChild(
										new SpatialNode( 0, Y, SpatialNode::SpatialChild( posXMinY ),
														 SpatialNode::SpatialChild( posXPosY ) ) ) );
	ASSERT_EQ( minX, root.lookup( make_float3( -1, 3, 3 ) ) );
	ASSERT_EQ( posXMinY, root.lookup( make_float3( 1, -1, 3 ) ) );
	ASSERT_EQ( posXPosY, root.lookup( make_float3( 1, 1, 3 ) ) );
}