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
	int completedSamples = 0;
	int samplesPerPixel = 1;
	int pixelCount;

  public:
	int completedIterations = 0;
	Sample sampleDirection( const Intersection& intersection, const BRDF& brdf, const float3& incoming );
	void train( const float3& position, const Sample& sample, float flux, float foreshortening, float lightTransport );
	void increaseSamples() { completedSamples++; }
	void completeSample();
	[[nodiscard]] inline bool iterationIsFinished() const;
	TrainModule( const float3& min, const float3& max, int pixelCount ) : guidingNode( SpatialNode( X, SpatialNode::newLeaf(), SpatialNode::newLeaf(), min, max ) ), storingNode( SpatialNode( X, SpatialNode::newLeaf(), SpatialNode::newLeaf(), min, max ) ), pixelCount( pixelCount ){};
};

class ImageBuffer
{
  private:
	int width, height;
	std::vector<float3*> pixels{};
	int* counts;
	int bestIteration = 0;

  public:
	ImageBuffer( int width, int height );
	void recordSample( int iteration, int px, int py, float3 value );
	float3 currentEstimate( int px, int py );
	virtual ~ImageBuffer();
};
class PathGuidingTracer
{
  private:
	TrainModule* module;
	Environment* environment;
	ImageBuffer* imageBuffer;
	BRDFs* brdfs;

  public:
	PathGuidingTracer( Environment* environment, BRDFs* brdfs );

  public:
	float3 trace( Ray& r );
	float3 performSample( Ray& r, int px, int py );
	void cameraChanged( TrainModule* trainModule, ImageBuffer* buffer );
};
} // namespace lh2core