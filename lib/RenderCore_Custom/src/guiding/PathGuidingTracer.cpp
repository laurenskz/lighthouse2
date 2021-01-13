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
		   lightTransport;
}
Sample TrainModule::sampleDirection( const Intersection& intersection, const BRDF& brdf, const float3& incoming )
{
	SpatialLeaf* leaf = guidingNode.lookup( intersection.location );
	float alpha = leaf->brdfProb();
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
	leaf->directions->depositEnergy( sample.direction, radianceEstimate / sample.combinedPdf );
	leaf->misOptimizationStep( position, sample, radianceEstimate, foreshortening, lightTransport );
}
void TrainModule::completeSample()
{
	guidingNode = SpatialNode( storingNode );
	//	TODO: split on visitcount and collected
}
} // namespace lh2core