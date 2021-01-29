#include "guiding/PathGuidingTracer.h"

namespace lh2core
{

float3 PathGuidingTracer::trace( Ray& r )
{
	r.t = MAX_DISTANCE;
	//	TODO HANDLE RECURSION LIMIT VIA RUSSIAN ROULETTE
	auto intersection = environment->intersect( r );
	if ( !intersection.hitObject )
		return environment->skyColor( r.direction );
	if ( intersection.mat.type == LIGHT )
	{
		return intersection.mat.color;
	}
	auto brdf = brdfs->brdfForMat( intersection.mat );
	const Sample sample = module->sampleDirection( intersection, *brdf, r.direction );
	//	if ( !brdf->directionMayResultInTransport( intersection.location, intersection.normal, r.direction, sample.direction ) )
	//	{
	//		return BLACK;
	//	}
	Ray newRay = Ray{ intersection.location, sample.direction };
	float3 lightSample = trace( newRay );
	float foreShortening = dot( intersection.normal, sample.direction );
	float lightTransport = brdf->lightTransport( intersection.location, intersection.normal, r.direction, sample.direction );
	module->train( intersection.location, sample, lightSample.x + lightSample.y + lightSample.z, foreShortening, lightTransport );
	return foreShortening *
		   ( lightSample / sample.combinedPdf ) *
		   lightTransport * intersection.mat.color;
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
	float alpha = completedIterations == 0 ? 1 : 0.2;
	float3 dir{};
	if ( randFloat() < alpha )
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
	storingNode.splitDirectionsAbove( 0.1 );
	int twoToK = pow( 2, completedIterations );
	storingNode.splitAllAbove( 12000.0 * sqrt( twoToK ) );
	completedIterations++;
	samplesPerPixel = pow( 2, completedIterations );
	completedSamples = 0;
}
inline bool TrainModule::iterationIsFinished() const
{
	return ( samplesPerPixel * pixelCount ) <= completedSamples;
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