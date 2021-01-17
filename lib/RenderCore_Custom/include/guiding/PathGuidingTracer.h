#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/environment.h"
#include "graphics/lighting.h"
#include "guiding/BRDF.h"
#include "guiding/Tree.h"
#include "guiding/utils.h"
namespace lh2core
{

class TrainModule
{
  private:
	SpatialNode guidingNode;
	SpatialNode storingNode;
	int completedIterations = 0;
	int completedSamples = 0;
	int desiredSamples = 1;

  public:
	Sample sampleDirection( const Intersection& intersection, const BRDF& brdf, const float3& incoming );
	void train( const float3& position, const Sample& sample, float flux, float foreshortening, float lightTransport );
	void completeSample();
	TrainModule( const float3& min, const float3& max ) : guidingNode( SpatialNode( X, SpatialNode::newLeaf(), SpatialNode::newLeaf(), min, max ) ), storingNode( SpatialNode( X, SpatialNode::newLeaf(), SpatialNode::newLeaf(), min, max ) ){};
};
class PathGuidingTracer
{
  private:
	TrainModule* module;
	Environment* environment;
	BRDFs* brdfs;
	float3 trace( Ray& r );
};
} // namespace lh2core