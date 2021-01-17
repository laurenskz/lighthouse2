#pragma once
#include "platform.h"

using namespace lighthouse2;
namespace lh2core
{
float randFloat();
float3 projectIntoWorldSpace( const float3& direction, const float3& normal );
float3 cosineSampleHemisphere();

}