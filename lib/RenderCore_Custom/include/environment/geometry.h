#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/primitives.h"
namespace lh2core
{
class Mesh
{
  public:
	explicit Mesh( int vertexCount );
	float4* positions;
	float3* normals;
	mat4 transform = mat4::Identity();
	int vertexCount;
	int triangleCount;
	CoreTri* triangles = nullptr; // 'fat' triangle data
};

struct Instance
{
	int meshIndex{};
	mat4 transform;
};

class Geometry
{
  private:
	std::vector<Mesh*> meshes = std::vector<Mesh*>();
	std::vector<Instance> instances = std::vector<Instance>();
	std::vector<Primitive> planes = std::vector<Primitive>();
	std::vector<Primitive> spheres = std::vector<Primitive>();
	std::vector<Material> sphereMaterials = std::vector<Material>();
	bool isDirty = true;
	Primitive* primitives;
	int count;
	uint addPrimitives(int startIndex,const std::vector<Primitive>& toAdd );
	uint computePrimitiveCount();
	void addTriangles( int primitiveIndex );
	Intersection triangleIntersection(const Primitive& primitive,float t,Ray r);

  public:
	void setGeometry( const int meshIdx, const float4* vertexData, const int vertexCount, const int triangleCount, const CoreTri* triangles );
	void setInstance( const int instanceIdx, const int modelIdx, const mat4& transform = mat4::Identity() );
	void addPlane( float3 normal, float d );
	void finalizeInstances();
	Primitives getPrimitives();
	void addSphere( float3 pos, float r, Material mat );
	Intersection intersectionInformation( const Primitive& primitive, float t, Ray r );
};
} // namespace lh2core