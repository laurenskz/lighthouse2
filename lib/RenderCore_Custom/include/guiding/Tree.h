#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/environment.h"
#include "graphics/lighting.h"
#include "guiding/BRDF.h"
#include "guiding/utils.h"
namespace lh2core
{
enum AXIS
{
	X,
	Y,
	Z
};

struct Sample
{
	float3 direction;
	float combinedPdf;
	float bsdfPdf;
	float guidingPdf;
};

class QuadTree
{
  public:
	QuadTree *nw = nullptr, *ne = nullptr, *sw = nullptr, *se = nullptr;
	float flux = 0;
	float2 topLeft, bottomRight;
	float xPlane{}, yPlane{};

	static float3 cylindricalToDirection( float2 cylindrical );
	QuadTree( const QuadTree& other );
	QuadTree( const float2& topLeft, const float2& bottomRight );
	QuadTree( QuadTree* nw, QuadTree* ne, QuadTree* sw, QuadTree* se, const float2& topLeft, const float2& bottomRight, float xPlane, float yPlane );
	[[nodiscard]] float2 uniformRandomPosition() const;
	QuadTree* sampleChildByEnergy();
	QuadTree* getChild( const float2& cylindrical );
	void depositEnergy( const float3& direction, float receivedEnergy );
	static float2 directionToCylindrical( float3 direction );
	float3 sample();
	float pdf( float3 direction );
	QuadTree* traverse( float2 pos );
	void splitLeaf();
	void splitAllAbove( float fluxThreshold );

  private:
	[[nodiscard]] inline bool isLeaf() const { return nw == nullptr; }
	float pdf( float2 cylindrical );
};

class SpatialLeaf
{
	int visitCount = 0;
	float theta = 0, m = 0, v = 0;
	int t = 0;
	float beta1 = 0.9, beta2 = 0.999, eps = 1e-8, lr = 0.01, regularization = 0.01;

  public:
	QuadTree* directions;

	explicit SpatialLeaf( QuadTree* directions ) : directions( directions ) {}
	SpatialLeaf( const SpatialLeaf& other );
	void incrementVisits() { visitCount++; };
	void misOptimizationStep( const float3& position, const Sample& sample, float radianceEstimate, float foreshortening, float lightTransport );
	void adamStep( float deltaTheta );
	//Probability of choosing the brdf
	float brdfProb() const;
};

class SpatialNode
{
  public:
	class SpatialChild
	{
	  public:
		SpatialChild() = default;
		SpatialChild( const SpatialChild& other );
		explicit SpatialChild( SpatialNode* node ) : isLeaf( false ), node( node ){};
		explicit SpatialChild( SpatialLeaf* leaf ) : isLeaf( true ), leaf( leaf ){};
		bool isLeaf{};
		union
		{
		  public:
			SpatialNode* node{};
			SpatialLeaf* leaf;
		};
		SpatialChild& operator=( const SpatialChild& other ) noexcept;
	};
	SpatialNode( const SpatialNode& other ) : SpatialNode( other.splitPane, other.splitAxis, SpatialChild( other.left ), SpatialChild( other.right ) ){};
	SpatialNode( float splitPane, AXIS splitAxis ) : splitPane( splitPane ), splitAxis( splitAxis ){};
	SpatialNode( float splitPane, AXIS splitAxis, SpatialChild left, SpatialChild right ) : splitPane( splitPane ), splitAxis( splitAxis ), left( left ), right( right ){};
	SpatialLeaf* lookup( float3 pos );
	void splitLeaf();
	void splitAllAbove( int visits );
	void splitDirectionsAbove( float flux );

	float splitPane;
	AXIS splitAxis;
	SpatialChild left, right;
};
} // namespace lh2core