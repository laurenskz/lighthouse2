#include "guiding/PathGuidingTracer.h"

namespace lh2core
{

float3 PathGuidingTracer::trace( Ray& r )
{
	r.t = MAX_DISTANCE;
	//	TODO HANDLE RECURSION LIMIT VIA RUSSIAN ROULETTE
	auto intersection = environment->intersect( r );
	if ( !intersection.hitObject )
	{
		return make_float3( 0.529, 0.808, 0.929 );
		return environment->skyColor( r.direction );
	}
	if ( intersection.mat.type == LIGHT )
	{
		return intersection.mat.color;
	}
	auto brdf = brdfs->brdfForMat( intersection.mat );
	const Sample sample = module->sampleDirection( intersection, *brdf, r.direction );
	if ( sample.combinedPdf < 1e-7 )
	{
		return BLACK;
	}
	//	if ( !brdf->directionMayResultInTransport( intersection.location, intersection.normal, r.direction, sample.direction ) )
	//	{
	//		return BLACK;
	//	}
	Ray newRay = Ray{ intersection.location, sample.direction };
	float3 lightSample = trace( newRay );
	if ( isnan( lightSample.x ) )
	{
		trace( newRay );
	}
	float foreShortening = dot( intersection.normal, sample.direction );
	const float3& lightTransport = brdf->lightTransport( intersection.location, intersection.normal, r.direction, sample.direction );
	module->train( intersection.location, sample,
				   0.299 * lightSample.x + 0.587 * lightSample.y + 0.114 * lightSample.z, foreShortening,
				   0.299 * lightTransport.x + 0.5987 * lightTransport.y + 0.114 * lightTransport.z );
	return foreShortening *
		   ( lightSample / sample.combinedPdf ) *
		   lightTransport;
}
float3 PathGuidingTracer::performSample( Ray& r, int px, int py )
{
	const float3& sampledColor = trace( r );
	module->increaseSamples();
	imageBuffer->recordSample( module->completedIterations, px, py, sampledColor );
	return imageBuffer->currentEstimate( px, py );
}
void PathGuidingTracer::cameraChanged( TrainModule* trainModule, ImageBuffer* buffer )
{
	imageBuffer = buffer;
	module = trainModule;
}
PathGuidingTracer::PathGuidingTracer( Environment* environment, BRDFs* brdfs ) : environment( environment ), brdfs( brdfs ) {}
Sample TrainModule::sampleDirection( const Intersection& intersection, const BRDF& brdf, const float3& incoming )
{
	SpatialLeaf* leaf = guidingNode.lookup( intersection.location );
	//	TODO: fix this
	//	float alpha = completedIterations == 0 ? 1 : leaf->brdfProb();
	float alpha = completedIterations == 0 ? 1 : 0.5;
	alpha = 1;
	float3 dir{};
	if ( randFloat() <= alpha )
	{
		dir = brdf.sampleDirection( intersection.location, intersection.normal, incoming );
	}
	else
	{
		dir = leaf->directions->sample();
	}
	float bsdfPdf = brdf.probabilityOfOutgoingDirection(
		intersection.location,
		intersection.normal,
		incoming,
		dir );
	float guidingPdf = leaf->directions->pdf( dir );
	float directionProb = alpha * bsdfPdf +
						  ( 1 - alpha ) * guidingPdf;
	return Sample{ dir, directionProb, bsdfPdf, guidingPdf };
}

void TrainModule::train( const float3& position, const Sample& sample, float radianceEstimate, float foreshortening, float lightTransport )
{
	SpatialLeaf* leaf = storingNode.lookup( position );
	leaf->incrementVisits();
	leaf->directions->depositEnergy( sample.direction, radianceEstimate );
	leaf->misOptimizationStep( position, sample, radianceEstimate, foreshortening, lightTransport );
	if ( iterationIsFinished() )
	{
		completeSample();
	}
}
void TrainModule::
	completeSample()
{
	guidingNode = SpatialNode( storingNode );
	int twoToK = pow( 2, completedIterations );
	storingNode.splitAllAbove( 12000.0 * sqrt( twoToK ) );
	storingNode.splitDirectionsAbove( 0.1 );
	storingNode.resetData();
	completedIterations++;
	samplesPerPixel = pow( 2, completedIterations );
	completedSamples = 0;
}
inline bool TrainModule::iterationIsFinished() const
{
	return ( samplesPerPixel * pixelCount ) <= completedSamples;
}
TrainModule::TrainModule( const float3& min, const float3& max, int pixelCount ) : guidingNode( SpatialNode( X, SpatialNode::newLeaf(), SpatialNode::newLeaf(), min, max ) ), storingNode( SpatialNode( X, SpatialNode::newLeaf(), SpatialNode::newLeaf(), min, max ) ), pixelCount( pixelCount )
{
	storingNode.right.leaf->directions->splitLeaf();
	storingNode.left.leaf->directions->splitLeaf();
}
void ImageBuffer::recordSample( int iteration, int px, int py, float3 value )
{
	if ( pixels.size() <= iteration )
	{
		pixels.push_back( new float3[width * height] );
	}
	pixels[iteration][px + py * width] += value;
	if ( px >= width - 1 && py >= height - 1 )
	{
		counts[iteration]++;
		if ( counts[iteration] >= counts[bestIteration] )
		{
			bestIteration = iteration;
		}
	}
}
float3 ImageBuffer::currentEstimate( int px, int py )
{
	return pixels[bestIteration][px + py * width] / static_cast<float>( counts[bestIteration] );
}
ImageBuffer::ImageBuffer( int width, int height ) : width( width ), height( height )
{
	counts = new int[64];
	for ( int i = 0; i < 64; ++i )
	{
		counts[i] = 1;
	}
}
ImageBuffer::~ImageBuffer()
{
	for ( auto& pixel : pixels )
	{
		delete pixel;
	}
}
} // namespace lh2core