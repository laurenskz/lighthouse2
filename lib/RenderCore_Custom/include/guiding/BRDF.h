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
	[[nodiscard]] virtual float lightTransport( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const = 0;
	[[nodiscard]] virtual bool isDiscrete() const = 0;
};

class DiffuseBRDF : public BRDF
{
  public:
	[[nodiscard]] float3 sampleDirection( const float3& pos, const float3& normal, const float3& incoming ) const override;
	[[nodiscard]] float probabilityOfOutgoingDirection( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const override;
	[[nodiscard]] float lightTransport( const float3& pos, const float3& normal, const float3& incoming, const float3& outgoing ) const override;
	[[nodiscard]] bool isDiscrete() const override;
};

class BRDFs
{
  public:
	BRDF* brdfForMat( const Material& material );
};
} // namespace lh2core