#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/environment.h"
#include "graphics/lighting.h"
namespace lh2core
{
enum AXIS
{
	X,
	Y,
	Z
};
class QuadTree
{
  public:
	QuadTree *nw = nullptr, *ne = nullptr, *sw = nullptr, *se = nullptr;
	float flux = 0;
	float2 topLeft, bottomRight;
	float xPlane{}, yPlane{};
	static float3 cylindricalToDirection( float2 cylindrical );
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

  private:
	inline bool isLeaf() { return nw == nullptr; }
	float pdf( float2 cylindrical );

};

class SpatialLeaf
{
	int visitCount = 0;
	QuadTree* directions;

  public:
	explicit SpatialLeaf( QuadTree* directions ) : directions( directions ) {}

  public:
	void incrementVisits() { visitCount++; };
};

class SpatialNode
{
  public:
	class SpatialChild
	{
	  public:
		SpatialChild() = default;
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

	SpatialNode( float splitPane, AXIS splitAxis ) : splitPane( splitPane ), splitAxis( splitAxis ){};
	SpatialNode( float splitPane, AXIS splitAxis, SpatialChild left, SpatialChild right ) : splitPane( splitPane ), splitAxis( splitAxis ), left( left ), right( right ){};
	SpatialLeaf* lookup( float3 pos );

	float splitPane;
	AXIS splitAxis;
	SpatialChild left, right;
};
} // namespace lh2core