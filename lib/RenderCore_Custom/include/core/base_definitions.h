//
// Created by laurens on 11/16/20.
//

#pragma once
namespace lh2core
{


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

inline float3 locationAt( float t, Ray r ) { return r.start + t * r.direction; };

}