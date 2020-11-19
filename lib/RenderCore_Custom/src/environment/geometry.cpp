#include "environment/geometry.h"
namespace lh2core
{

Mesh::Mesh( int vertexCount )
{
	normals = new float3[vertexCount];
	positions = new float4[vertexCount];
	triangleCount = vertexCount / 3;
	this->vertexCount = vertexCount;
}
void Geometry::setGeometry( const int meshIdx, const float4* vertexData, const int vertexCount, const int triangleCount, const CoreTri* triangles )
{
	Mesh* mesh;
	if ( meshIdx >= meshes.size() )
		meshes.push_back( mesh = new Mesh( vertexCount ) );
	else
		mesh = meshes[meshIdx];
	for ( int i = 0; i < vertexCount; ++i )
	{
		mesh->positions[i] = vertexData[i];
	}
	for ( int i = 0; i < triangleCount; i++ )
	{
		mesh->normals[i * 3 + 0] = triangles[i].vN0;
		mesh->normals[i * 3 + 1] = triangles[i].vN1;
		mesh->normals[i * 3 + 2] = triangles[i].vN2;
	}
}
void Geometry::setInstance( const int instanceIdx, const int meshIdx, const mat4& transform )
{
	if ( meshIdx < 0 ) return;
	if ( instanceIdx >= instances.size() )
	{
		instances.push_back( Instance{ meshIdx, transform } );
	}
	else
	{
		instances[instanceIdx].meshIndex = meshIdx;
		instances[instanceIdx].transform = transform;
	}
}
Intersection Geometry::intersectionInformation( const Primitive& primitive, float t, Ray r )
{
	if ( isSphere( primitive ) )
	{
		return sphereIntersection( primitive, sphereMaterials[primitive.meshIndex], r, t );
	}
	if ( isPlane( primitive ) )
	{
		return planeIntersection( primitive, Material{}, r, t );
	}
	if ( isTriangle( primitive ) )
	{
		return triangleIntersection( primitive, t, r );
	}
	return Intersection();
}
Primitives Geometry::getPrimitives()
{
	if ( isDirty )
	{
		finalizeInstances();
	}
	return Primitives{ primitives, count };
}

void Geometry::finalizeInstances()
{
	count = computePrimitiveCount();
	primitives = new Primitive[count];
	int primitiveIndex = 0;
	primitiveIndex = addPrimitives( primitiveIndex, spheres );
	primitiveIndex = addPrimitives( primitiveIndex, planes );
	addTriangles( primitiveIndex );
	isDirty = false;
}
void Geometry::addTriangles( int primitiveIndex )
{
	for ( auto instance : instances )
	{
		Mesh*& mesh = meshes[instance.meshIndex];
		for ( int i = 0; i < mesh->triangleCount; ++i )
		{
			primitives[primitiveIndex++] = Primitive{ TRIANGLE_BIT,
													  make_float3( instance.transform * mesh->positions[i * 3] ),
													  make_float3( instance.transform * mesh->positions[i * 3 + 1] ),
													  make_float3( instance.transform * mesh->positions[i * 3 + 2] ),
													  instance.meshIndex,
													  i };
		}
	}
}
void Geometry::addPlane( float3 normal, float d )
{
	planes.push_back( Primitive{
		PLANE_BIT, normal, make_float3( d, 0, 0 ), make_float3( 0 ), -1, -1 } );
}
void Geometry::addSphere( float3 pos, float r, Material mat )
{
	spheres.push_back( Primitive{
		SPHERE_BIT, pos, make_float3( r * r, 0, 0 ),
		make_float3( 0 ),
		static_cast<int>( sphereMaterials.size() ),
		-1 } );
	sphereMaterials.push_back( mat );
}
uint Geometry::addPrimitives( int startIndex, const std::vector<Primitive>& toAdd )
{
	for ( auto primitive : toAdd )
	{
		primitives[startIndex++] = primitive;
	}
	return startIndex;
}
uint Geometry::computePrimitiveCount()
{
	int triangleCount = 0;
	for ( auto instance : instances )
	{
		triangleCount += meshes[instance.meshIndex]->triangleCount;
	}
	return planes.size() + spheres.size() + triangleCount;
}
Intersection Geometry::triangleIntersection( const Primitive& primitive, float t, Ray r )
{
	float3 v0v1 = primitive.v2 - primitive.v1;
	float3 v0v2 = primitive.v3 - primitive.v1;
	float3 triangleNormal = cross( v0v1, v0v2 ); // N

	return Intersection{ locationAt( t, r ), normalize( triangleNormal ), Material{ make_float3( 0, 1, 0 ), 0.5 } };
}
} // namespace lh2core