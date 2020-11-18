//
// Created by laurens on 11/16/20.
//
#include <cfloat>
#pragma once
namespace lh2core
{
#define MAX_DISTANCE FLT_MAX
struct Material
{
	float3 color;
};

struct Intersection
{
	float3 location;
	float3 normal;
	Material mat;
};

struct Ray
{
	float3 start;
	float3 direction;
};



} // namespace lh2core