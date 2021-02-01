#include "guiding/BRDF.h"
namespace lh2core
{

BRDF* BRDFs::brdfForMat( const Material& material )
{
	if ( material.type == DIFFUSE )
	{
		return new DiffuseBRDF( material.color );
	}
	if ( material.type == MICROFACET )
	{
		return new DualBRDF( new MicroFacetBRDF( material.microAlpha, material.kspec ), new DiffuseBRDF( material.color ), material.specularity );
	}
}
float3 DiffuseBRDF::sampleDirection( const float3& pos, const float3& normal, const float3& incoming ) const
{
	return projectIntoWorldSpace( cosineSampleHemisphere(), normal );
}
float DiffuseBRDF::probabilityOfOutgoingDirection( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const
{
	return dot( normal, outgoing );
}
float3 DiffuseBRDF::lightTransport( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const
{
	return color / PI;
}
bool DiffuseBRDF::isDiscrete() const
{
	return false;
}
DiffuseBRDF::DiffuseBRDF( const float3& color ) : color( color ) {}
float3 MicroFacetBRDF::sampleDirection( const float3& pos, const float3& normal, const float3& incoming ) const
{
	float r1 = randFloat();
	float r2 = randFloat();
	float t = pow( r2, 2.f / ( alpha + 1 ) );
	float sqrt1minT = sqrt( 1 - t );
	float3 h = make_float3( cos( 2 * PI * r1 ) * sqrt1minT, sqrt( t ), sin( 2 * PI * r1 ) * sqrt1minT );
	const float3& v = -incoming;
	return projectIntoWorldSpace( 2 * dot( v, h ) * h - v, normal );
}
float MicroFacetBRDF::probabilityOfOutgoingDirection( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const
{
	float3 h = normalize( ( -incoming ) + outgoing );
	return blinnPhong( h, normal );
}
float3 MicroFacetBRDF::lightTransport( const float3& pos, const float3& normal, const float3& v, const float3& l ) const
{
	float3 h = normalize( ( -v ) + l );

	return (
			   fresnel( kspec, l, h ) *
			   geometryTerm( normal, -v, h, l ) *
			   blinnPhong( h, normal ) ) /
		   ( 4 * dot( normal, l ) * dot( normal, -v ) );
}
bool MicroFacetBRDF::isDiscrete() const
{
	return false;
}
float MicroFacetBRDF::blinnPhong( const float3& h, const float3& n ) const
{
	float ndoth = dot( n, h );
	return ( ( alpha + 2 ) / ( 2 * PI ) ) * pow( max( 0.f, ndoth ), alpha );
}
float MicroFacetBRDF::geometryTerm( const float3& n, const float3& v, const float3& h, const float3& l ) const
{
	float ndoth = dot( n, h );
	float vdoth = max( 0.f, dot( v, h ) );
	return min( 1.f, min( ( 2 * dot( n, v ) * ndoth ) / vdoth, ( 2 * dot( n, l ) * ndoth ) / vdoth ) );
}
float3 MicroFacetBRDF::fresnel( float3 kspec, const float3& l, const float3& h ) const
{
	float toPow = 1 - dot( l, h );
	return kspec + ( 1 - kspec ) * toPow * toPow * toPow * toPow * toPow;
}
MicroFacetBRDF::MicroFacetBRDF( float alpha, float3 kspec ) : alpha( alpha ), kspec( kspec ) {}
float3 DualBRDF::sampleDirection( const float3& pos, const float3& normal, const float3& incoming ) const
{
	if ( randFloat() < kspec )
	{
		return specular->sampleDirection( pos, normal, incoming );
	}
	return diffuse->sampleDirection( pos, normal, incoming );
}
float DualBRDF::probabilityOfOutgoingDirection( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const
{
	return kspec * specular->probabilityOfOutgoingDirection( pos, normal, incoming, outgoing ) +
		   ( 1 - kspec ) * diffuse->probabilityOfOutgoingDirection( pos, normal, incoming, outgoing );
}
float3 DualBRDF::lightTransport( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const
{
	return kspec * specular->lightTransport( pos, normal, incoming, outgoing ) + ( 1 - kspec ) * diffuse->lightTransport( pos, normal, incoming, outgoing );
}
bool DualBRDF::isDiscrete() const
{
	return false;
}
DualBRDF::DualBRDF( BRDF* specular, BRDF* diffuse, float kspec ) : specular( specular ), diffuse( diffuse ), kspec( kspec ) {}
} // namespace lh2core