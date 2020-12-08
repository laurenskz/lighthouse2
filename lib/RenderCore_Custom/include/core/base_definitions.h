//
// Created by laurens on 11/16/20.
//
#include <cfloat>
#pragma once
namespace lh2core
{
#define MAX_DISTANCE FLT_MAX
enum MaterialKind
{
	DIFFUSE,
	SPECULAR,
	GLASS,
	LIGHT
};
struct Material
{
	float3 color;
	float specularity = 0;
	MaterialKind type = DIFFUSE;
	float refractionIndex = 1;
};

struct Intersection
{
	float3 location;
	float3 normal;
	Material mat;
	bool hitObject = false;
};

struct Primitive
{
	uint flags;
	float3 v1;
	float3 v2;
	float3 v3;
	int meshIndex;		// The index of the mesh
	int triangleNumber; //Number of this triangle inside the mesh
	int instanceIndex;	//The transformation
};

struct Ray
{
	float3 start;
	float3 direction;
	float t = MAX_DISTANCE, u{}, v{};
	bool alive = true;
	const Primitive* primitive = nullptr;
};

} // namespace lh2core