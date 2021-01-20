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
	int samplesPerPixel = 1;
	int pixelCount;

  public:
	Sample sampleDirection( const Intersection& intersection, const BRDF& brdf, const float3& incoming );
	void train( const float3& position, const Sample& sample, float flux, float foreshortening, float lightTransport );
	void completeSample();
	void reset();
	[[nodiscard]] inline bool iterationIsFinished() const;
	TrainModule( const float3& min, const float3& max, int pixelCount ) : guidingNode( SpatialNode( X, SpatialNode::newLeaf(), SpatialNode::newLeaf(), min, max ) ), storingNode( SpatialNode( X, SpatialNode::newLeaf(), SpatialNode::newLeaf(), min, max ) ), pixelCount( pixelCount ){};
};

class ImageBuffer
{
  private:
	int width, height;

  public:
	void recordSample( int iteration, int px, int py, float3 value );
};
class PathGuidingTracer
{
  private:
	TrainModule* module;
	Environment* environment;
	BRDFs* brdfs;

  public:
	float3 trace( Ray& r );
	float3 performSample( Ray& r, int px, int py );
	void cameraChanged();
};
} // namespace lh2core