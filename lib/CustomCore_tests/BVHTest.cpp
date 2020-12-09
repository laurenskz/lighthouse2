#include "acceleration/bvh.h"
#include "gtest/gtest.h"
using namespace lh2core;

#define EXPECT_FLOAT3_EQ( expected, actual )   \
	ASSERT_NEAR( expected.x, actual.x, 1e-3 ); \
	ASSERT_NEAR( expected.y, actual.y, 1e-3 ); \
	ASSERT_NEAR( expected.z, actual.z, 1e-3 );

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
	auto builder = new BaseBuilder();
	builder->buildBVH( primitives, primitiveCount );
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
	int indices[] = { 0, 1, 2, 3 };
	auto bounds = calculateBounds( primitives, indices, 0, 4 );
	EXPECT_FLOAT3_EQ( make_float3( -1, 0, -10 ), bounds.min )
	EXPECT_FLOAT3_EQ( make_float3( 3, 2, 3 ), bounds.max )
	int indices2[] = { 0, 2, 3 };
	bounds = calculateBounds( primitives, indices2, 1, 2 );
	EXPECT_FLOAT3_EQ( make_float3( 0 ), bounds.min )
	EXPECT_FLOAT3_EQ( make_float3( 3, 2, 3 ), bounds.max )
	updateAABB(bounds,primitives[0]);
	EXPECT_FLOAT3_EQ( make_float3( -1, 0, -10 ), bounds.min )

}