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
};

class SpatialLeaf
{
	int visitCount = 0;
	QuadTree directions{};

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