//
// Created by laurens on 11/15/20.
//
#include "core_settings.h"
#include "gtest/gtest.h"

#define EXPECT_VEC_EQ( expected, actual )      \
	ASSERT_NEAR( expected.x, actual.x, 1e-3 ); \
	ASSERT_NEAR( expected.y, actual.y, 1e-3 ); \
	ASSERT_NEAR( expected.z, actual.z, 1e-3 );

ostream& operator<<( ostream& os, const float3& s )
{
	return ( os << "{" << s.x << "," << s.y << "," << s.z << "}" << std::endl );
}

class RayFixture : public ::testing::Test
{

  protected:
	void SetUp() override
	{
		rayTracer = new RayTracer();
		scene = new Scene();
		materials = new Material[1];
		materials[0].color = make_float3( 1, 0, 0 );
	}

	void TearDown() override
	{
		delete rayTracer;
		delete scene;
	}

	RayTracer* rayTracer;
	Scene* scene;
	Material* materials;
};

TEST_F( RayFixture, RayHasRightDirection )
{
	auto vp = ViewPyramid{};
	EXPECT_VEC_EQ( rayTracer->rayDirection( 0, 0, vp ), normalize( vp.p1 ) );
	EXPECT_VEC_EQ( rayTracer->rayDirection( 1, 0, vp ), normalize( vp.p2 ) );
}

TEST_F( RayFixture, SphereIntersects )
{
	Sphere sphere = Sphere( make_float3( 0, 0, 0 ), 1, 0 );
	ASSERT_EQ( 1, sphere.distanceTo( Ray{ make_float3( 0, 0, -2 ), make_float3( 0, 0, 1 ) } ) );
	const float3& rayDirection = normalize( make_float3( 0.2, 0.1, 1 ) );
	const Ray& ray2 = Ray{ make_float3( 0, 0, -3 ), rayDirection };
	float t = sphere.distanceTo( ray2 );
	ASSERT_NEAR( 2.171, t, 1e-3 );
	auto intersectionPoint = locationAt( t, ray2 );
	ASSERT_NEAR( 1, length( intersectionPoint ), 1e-3 );
	auto intersection = sphere.intersectionAt( intersectionPoint, materials );
	EXPECT_VEC_EQ( intersection.location, intersectionPoint );
	EXPECT_VEC_EQ( intersectionPoint, intersection.location );
}

TEST_F( RayFixture, Illumination )
{
	const PointLight& light = PointLight{ make_float3( 0, 1, 0 ), 1 };
	ASSERT_NEAR( 1, scene->illuminationFrom( light, make_float3( 0 ), make_float3( 0, 1, 0 ) ), 1e-3 );
	ASSERT_NEAR( 0.196, scene->illuminationFrom( light, make_float3( 0 ), normalize( make_float3( 5, 1, 0 ) ) ), 1e-3 );
	ASSERT_NEAR( 0, scene->illuminationFrom( light, make_float3( 0 ), normalize( make_float3( 1, 0, 0 ) ) ), 1e-3 );
}

TEST_F( RayFixture, PlaneIntersects )
{
	Plane plane = Plane( make_float3( 0, 1, 0 ), 2, 0 );
	const Ray& r = Ray{ make_float3( 0, 2, 0 ), normalize( make_float3( -2, -1, 0 ) ) };
	auto t = plane.distanceTo( r );
	auto intersection = locationAt( t, r );
	cout << intersection;
}

TEST_F( RayFixture, TriangleIntersects )
{
	Ray r{ make_float3( 0 ), make_float3( 0, -1, 0 ) };
	ASSERT_NEAR(1,Mesh::distanceTo( r,
					  make_float3( -1, -1, -1 ),
					  make_float3( -1, -1, 1 ),
					  make_float3( 1, -1, 0 ) )
						,1e-5);
	ASSERT_NEAR(0.5,Mesh::distanceTo( r,
					  make_float3( -1, -0.5, -1 ),
					  make_float3( -1, -0.5, 1 ),
					  make_float3( 1, -0.5, 0 ) )
						,1e-5);
	ASSERT_NEAR(0.25,Mesh::distanceTo( r,
					  make_float3( -1, 0, -1 ),
					  make_float3( -1, 0, 1 ),
					  make_float3( 1, -0.5, 0 ) )
						,1e-5);
	ASSERT_NEAR(-1,Mesh::distanceTo( r,
					  make_float3( -1, -1, -1 ),
					  make_float3( -1, -1, 1 ),
					  make_float3( 1, 2.1, 0 ) )
						,1e-5);
}