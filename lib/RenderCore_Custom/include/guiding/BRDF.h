#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "guiding/utils.h"
namespace lh2core
{

class BRDF
{
  public:
	// Used to go in random direction
	[[nodiscard]] virtual float3 sampleDirection( const float3& pos, const float3& normal, const float3& incoming ) const = 0;
	// Used for importance sampling
	[[nodiscard]] virtual float probabilityOfOutgoingDirection( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const = 0;
	// Used to compute the amount of light that is passed through
	[[nodiscard]] virtual float3 lightTransport( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const = 0;
	[[nodiscard]] virtual bool isDiscrete() const = 0;
};

class DiffuseBRDF : public BRDF
{
  private:
	float3 color;

  public:
	DiffuseBRDF( const float3& color );

  public:
	[[nodiscard]] float3 sampleDirection( const float3& pos, const float3& normal, const float3& incoming ) const override;
	[[nodiscard]] float probabilityOfOutgoingDirection( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const override;
	[[nodiscard]] float3 lightTransport( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const override;
	[[nodiscard]] bool isDiscrete() const override;
};

class DualBRDF : public BRDF
{
  private:
  	BRDF* specular;
	BRDF* diffuse;
	float kspec;

  public:
	DualBRDF( BRDF* specular, BRDF* diffuse, float kspec );

  public:
	float3 sampleDirection( const float3& pos, const float3& normal, const float3& incoming ) const override;
	float probabilityOfOutgoingDirection( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const override;
	float3 lightTransport( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const override;
	bool isDiscrete() const override;
};

class MicroFacetBRDF : public BRDF
{

	float alpha;
	float3 kspec;

  public:
	[[nodiscard]] float3 sampleDirection( const float3& pos, const float3& normal, const float3& incoming ) const override;
	MicroFacetBRDF( float alpha, float3 kspec );
	[[nodiscard]] float probabilityOfOutgoingDirection( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const override;
	[[nodiscard]] float3 lightTransport( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const override;
	[[nodiscard]] bool isDiscrete() const override;

  private:
	float blinnPhong( const float3& h, const float3& n ) const;
	float geometryTerm( const float3& n, const float3& v, const float3& h, const float3& l ) const;
	float3 fresnel( float3 kspec, const float3& l, const float3& h ) const;
};

class BRDFs
{
  public:
	BRDF* brdfForMat( const Material& material );
};
} // namespace lh2core