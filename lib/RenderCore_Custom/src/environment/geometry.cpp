#include "environment/geometry.h"
namespace lh2core
{

Mesh::Mesh( int vertexCount )
{
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
	mesh->triangles = new CoreTri[triangleCount];
	memcpy( mesh->triangles, triangles, triangleCount * sizeof( CoreTri ) );
}
void Geometry::setInstance( const int instanceIdx, const int meshIdx, const mat4& transform )
{
	if ( meshIdx < 0 ) return;
	if ( instanceIdx >= instances.size() )
	{
		instances.push_back( Instance{ meshIdx, transform } );
		transforms.push_back( transform );
	}
	else
	{
		transforms[instanceIdx] = transform;
		instances[instanceIdx].meshIndex = meshIdx;
		instances[instanceIdx].transform = transform;
	}
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
	primitiveIndex = addLights( primitiveIndex );
	primitiveIndex = addTriangles( primitiveIndex );
	isDirty = false;
}
int Geometry::addLights( int primitiveIndex )
{
	for ( int i = 0; i < lightCount; ++i )
	{
		primitives[primitiveIndex++] = Primitive{ TRIANGLE_BIT | LIGHT_BIT,
												  lights[i].vertex0,
												  lights[i].vertex1,
												  lights[i].vertex2,
												  i, -1, -1 };
	}
	return primitiveIndex;
}
int Geometry::addTriangles( int primitiveIndex )
{
	for ( int instanceIndex = 0; instanceIndex < instances.size(); ++instanceIndex )
	{
		auto instance = instances[instanceIndex];
		Mesh*& mesh = meshes[instance.meshIndex];
		for ( int i = 0; i < mesh->triangleCount; ++i )
		{
			auto matId = meshes[instance.meshIndex]->triangles[i].material;
			int transparentModifier = materials[matId].pbrtMaterialType == lighthouse2::MaterialType::PBRT_GLASS ? 1 : 0;
			primitives[primitiveIndex++] = Primitive{ TRIANGLE_BIT | ( TRANSPARENT_BIT * transparentModifier ),
													  make_float3( instance.transform * mesh->positions[i * 3] ),
													  make_float3( instance.transform * mesh->positions[i * 3 + 1] ),
													  make_float3( instance.transform * mesh->positions[i * 3 + 2] ),
													  instance.meshIndex,
													  i, instanceIndex };
		}
	}
	return primitiveIndex;
}
void Geometry::addPlane( float3 normal, float d )
{
	planes.push_back( Primitive{
		PLANE_BIT, normal, make_float3( d, 0, 0 ), make_float3( 0 ), -1, -1, -1 } );
}
void Geometry::addSphere( float3 pos, float r, Material mat )
{
	int transparentModifier = mat.type == GLASS ? 1 : 0;
	spheres.push_back( Primitive{
		SPHERE_BIT | ( TRANSPARENT_BIT * transparentModifier ), pos, make_float3( r * r, r, 0 ),
		make_float3( 0 ),
		static_cast<int>( sphereMaterials.size() ),
		-1, -1 } );
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
	return planes.size() + spheres.size() + triangleCount + lightCount;
}
Intersection Geometry::triangleIntersection( const Ray& r )
{
	Intersection intersection{};
	intersection.location = intersectionLocation( r );
	if ( r.primitive->flags & LIGHT_BIT )
	{
		intersection.mat = lightMaterials[r.primitive->meshIndex];
	}
	const CoreTri& triangleData = meshes[r.primitive->meshIndex]->triangles[r.primitive->triangleNumber];
	auto normal = r.u * triangleData.vN0 + ( r.v ) * triangleData.vN1 + ( 1 - r.u - r.v ) * triangleData.vN2;
	intersection.normal = normalize( make_float3( transforms[r.primitive->instanceIndex] * ( make_float4( normal ) ) ) );
	auto mat = materials[triangleData.material];
	intersection.mat.type = DIFFUSE;
	if ( mat.pbrtMaterialType == lighthouse2::MaterialType::PBRT_GLASS )
	{
		intersection.mat.type = GLASS;
		intersection.mat.refractionIndex = mat.refraction.value;
	}
	if ( mat.color.textureID != -1 )
	{
		auto texture = textures[mat.color.textureID];
		float2 uv = make_float2( r.u, r.v );
		uv *= mat.color.uvscale;
		uv += mat.color.uvoffset;
		int x = floor( uv.x * texture.width );
		int y = floor( uv.y * texture.height );
		const uchar4& iColor = texture.idata[x + y * texture.width];
		intersection.mat.color = make_float3( (float)iColor.x / 256, (float)iColor.y / 256, (float)iColor.z / 256 );
	}
	else
	{
		intersection.mat.color = mat.color.value;
	}
	if ( mat.specular.value > (1e-4) )
	{
		intersection.mat.type = SPECULAR;
		intersection.mat.specularity = mat.specular.value;
	}
	return intersection;
}
void Geometry::SetTextures( const CoreTexDesc* tex, const int textureCount )
{
	textures = new CoreTexDesc[textureCount];
	memcpy( textures, tex, sizeof( CoreTexDesc ) * textureCount );
}
void Geometry::SetMaterials( CoreMaterial* mat, const int materialCount )
{
	materials = new CoreMaterial[materialCount];
	memcpy( materials, mat, sizeof( CoreMaterial ) * materialCount );
}
void Geometry::SetLights( const CoreLightTri* newLights, const int newLightCount )
{
	isDirty = true;
	this->lightCount = newLightCount;
	if ( newLightCount == 0 ) return;
	lightMaterials.resize( newLightCount );
	this->lights = new CoreLightTri[newLightCount];
	memcpy( (void*)this->lights, newLights, newLightCount * sizeof( CoreLightTri ) );
	for ( int i = 0; i < newLightCount; ++i )
	{
		lightMaterials[i] = Material{ newLights[i].radiance, 0, LIGHT };
	}
}
Intersection Geometry::intersectionInformation( const Ray& ray )
{
	if ( isTriangle( *ray.primitive ) )
	{
		return triangleIntersection( ray );
	}
	if ( isSphere( *ray.primitive ) )
	{
		return sphereIntersection( ray, sphereMaterials[ray.primitive->meshIndex] );
	}
	if ( isPlane( *ray.primitive ) )
	{
		return planeIntersection( ray, Material{} );
	}
}
} // namespace lh2core