#include "guiding/PathGuidingTracer.h"
#include <core_settings.h>

namespace lh2core
{

float3 PathGuidingTracer::trace( Ray& r )
{
	r.t = MAX_DISTANCE;
	//	TODO HANDLE RECURSION LIMIT VIA RUSSIAN ROULETTE
	auto intersection = environment->intersect( r );
	if ( !intersection.hitObject )
	{
		return environment->skyColor( r.direction );
	}
	if ( intersection.mat.type == LIGHT )
	{
		return intersection.mat.color;
	}
	auto brdf = brdfs->brdfForMat( intersection.mat );

	const Sample sample = module->sampleDirection( intersection, *brdf, r.direction );
	if ( sample.combinedPdf < 1e-9 )
	{
		return BLACK;
	}
	Ray newRay = Ray{ intersection.location, sample.direction };
	float foreShortening = dot( intersection.normal, sample.direction );
	if ( foreShortening <= 0 )
	{
		return BLACK;
	}
	float3 lightSample = trace( newRay );
	const float3& lightTransport = brdf->lightTransport( intersection.location, intersection.normal, r.direction, sample.direction );
	module->train( intersection.location, sample,
				   0.299 * lightSample.x + 0.587 * lightSample.y + 0.114 * lightSample.z, foreShortening,
				   0.299 * lightTransport.x + 0.5987 * lightTransport.y + 0.114 * lightTransport.z );

	const float3& result = foreShortening *
						   ( lightSample / sample.combinedPdf ) *
						   lightTransport;
	return result;
}
float3 PathGuidingTracer::performSample( Ray& r, int px, int py )
{
	const float3& sampledColor = trace( r );
	imageBuffer->recordSample( module->currentIteration, px, py, sampledColor );
	return imageBuffer->currentEstimate( px, py );
}
void PathGuidingTracer::cameraChanged( TrainModule* trainModule, ImageBuffer* buffer )
{
	imageBuffer = buffer;
	module = trainModule;
}
PathGuidingTracer::PathGuidingTracer( Environment* environment, BRDFs* brdfs ) : environment( environment ), brdfs( brdfs ) {}
void PathGuidingTracer::iterationStarted()
{
}
void PathGuidingTracer::iterationFinished()
{
	imageBuffer->increaseCount( module->currentIteration );
	module->closeIteration();
}
bool PathGuidingTracer::isDone()
{
	return module->currentIteration == ( ITERATIONS + 1 );
}
Sample TrainModule::sampleDirection( const Intersection& intersection, const BRDF& brdf, const float3& incoming )
{
	SpatialLeaf* leaf = guidingNode.lookup( intersection.location );
	float alpha = currentIteration == 0 ? 1 : leaf->brdfProb();
#ifndef USE_GUIDING
	alpha = 1;
#endif
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
	lock.lock();
	SpatialLeaf* leaf = storingNode.lookup( position );
	leaf->incrementVisits();
	leaf->directions->depositEnergy( sample.direction, radianceEstimate );
#ifdef LEARN_ALPHA
	if ( currentIteration >= 1 )
	{
		leaf->misOptimizationStep( position, sample, radianceEstimate, foreshortening, lightTransport );
	}
#endif
	lock.unlock();
}
void TrainModule::
	completeSample()
{
	guidingNode = SpatialNode( storingNode );
	int twoToK = pow( 2, currentIteration );
	storingNode.splitAllAbove( 12000.0 * sqrt( twoToK ) );
	storingNode.splitDirectionsAbove( 0.1 );
	storingNode.resetData();
	currentIteration++;
	samplesPerPixel = pow( 2, currentIteration );
	completedSamples = 0;
}
TrainModule::TrainModule( const float3& min, const float3& max, int pixelCount ) : guidingNode( SpatialNode( X, SpatialNode::newLeaf(), SpatialNode::newLeaf(), min, max ) ), storingNode( SpatialNode( X, SpatialNode::newLeaf(), SpatialNode::newLeaf(), min, max ) ), pixelCount( pixelCount )
{
	storingNode.right.leaf->directions->splitLeaf();
	storingNode.left.leaf->directions->splitLeaf();
}
void TrainModule::closeIteration()
{
	completedSamples++;
	if ( completedSamples >= samplesPerPixel )
	{
		completeSample();
	}
}
void ImageBuffer::recordSample( int iteration, int px, int py, float3 value )
{
	lock.lock();
	if ( pixels.size() <= iteration )
	{
		pixels.push_back( new float3[width * height] );
	}
	pixels[iteration][px + py * width] += value;
	lock.unlock();
}

float3 ImageBuffer::currentEstimate( int px, int py )
{
	return pixels[bestIteration][px + py * width] / static_cast<float>( fmin( pow( 2, bestIteration ), counts[bestIteration] + 1 ) );
}
ImageBuffer::ImageBuffer( int width, int height ) : width( width ), height( height )
{
	counts = new int[64];
	for ( int i = 0; i < 64; ++i )
	{
		counts[i] = 0;
	}
}
ImageBuffer::~ImageBuffer()
{
	for ( auto& pixel : pixels )
	{
		delete pixel;
	}
}
void ImageBuffer::increaseCount( int iteration )
{
	counts[iteration]++;
	if ( counts[iteration] >= counts[bestIteration] )
	{
		bestIteration = iteration;
	}
}
} // namespace lh2core