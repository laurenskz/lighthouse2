#include "guiding/BRDF.h"
namespace lh2core
{

BRDF* BRDFs::brdfForMat( const Material& material )
{
	return new DiffuseBRDF();
}
float3 DiffuseBRDF::sampleDirection( const float3& pos, const float3& normal, const float3& incoming ) const
{
	return projectIntoWorldSpace( cosineSampleHemisphere(), normal );
}
float DiffuseBRDF::probabilityOfOutgoingDirection( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const
{
	return dot( normal, outgoing );
}
float DiffuseBRDF::lightTransport( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const
{
	return 1.f / PI;
}
bool DiffuseBRDF::isDiscrete() const
{
	return false;
}
} // namespace lh2core