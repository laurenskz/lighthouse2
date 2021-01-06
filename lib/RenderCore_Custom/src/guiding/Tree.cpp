#include "guiding/Tree.h"
namespace lh2core
{

SpatialLeaf* SpatialNode::lookup( float3 pos )
{
	SpatialNode* current = this;
	while ( true )
	{
		float axisPos = current->splitAxis == X ? pos.x : current->splitAxis == Y ? pos.y
																				  : pos.z;
		if ( axisPos < current->splitPane )
		{
			if ( current->left.isLeaf ) return current->left.leaf;
			current = current->left.node;
		}
		else
		{
			if ( current->right.isLeaf ) return current->right.leaf;
			current = current->right.node;
		}
	}
}
SpatialNode::SpatialChild& SpatialNode::SpatialChild::operator=( const SpatialNode::SpatialChild& other ) noexcept
{
	this->isLeaf = other.isLeaf;
	if ( this->isLeaf )
	{
		this->leaf = other.leaf;
	}
	else
	{
		this->node = other.node;
	}
	return *this;
}
QuadTree* QuadTree::traverse( float2 pos )
{
	QuadTree* current = this;
	while ( !current->isLeaf() )
	{
		current = current->getChild( pos );
	}
	return current;
}
float3 QuadTree::sample()
{
	QuadTree* current = this;
	while ( !current->isLeaf() )
	{
		current = current->sampleChildByEnergy();
	}
	const float2& cylindrical = current->uniformRandomPosition();
	return cylindricalToDirection( cylindrical );
}
float3 QuadTree::cylindricalToDirection( float2 cylindrical )
{
	float cosTheta = 2 * cylindrical.x - 1;
	float phi = 2 * PI * cylindrical.y;
	float sinTheta = sqrt( 1 - cosTheta * cosTheta );
	float sinPhi = sin( phi );
	float cosPhi = cos( phi );
	return make_float3( sinTheta * cosPhi, sinTheta * sinPhi, cosTheta );
}
float2 QuadTree::directionToCylindrical( float3 direction )
{
	float cosTheta = clamp( direction.z, -1.f, 1.f );
	float phi = atan2( direction.y, direction.x );
	while ( phi < 0 ) phi += 2 * PI;
	return make_float2( ( cosTheta + 1 ) / 2, phi / ( 2 * PI ) );
}
QuadTree::QuadTree( QuadTree* nw, QuadTree* ne, QuadTree* sw, QuadTree* se,
					const float2& topLeft, const float2& bottomRight, float xPlane, float yPlane )
	: nw( nw ), ne( ne ), sw( sw ), se( se ), topLeft( topLeft ), bottomRight( bottomRight ), xPlane( xPlane ), yPlane( yPlane ) {}
QuadTree::QuadTree( const float2& topLeft, const float2& bottomRight ) : topLeft( topLeft ), bottomRight( bottomRight ) {}

float randFloat()
{
	return static_cast<float>( random() ) / static_cast<float>( RAND_MAX );
}

float2 QuadTree::uniformRandomPosition() const
{
	float x = abs( topLeft.x - bottomRight.x );
	float y = abs( topLeft.y - bottomRight.y );
	return make_float2( randFloat() * x + topLeft.x, bottomRight.y + randFloat() * y );
}
QuadTree* QuadTree::sampleChildByEnergy()
{
	float total = ne->flux + nw->flux + se->flux + sw->flux;
	float dice = randFloat() * total;
	if ( ne->flux > dice ) return ne;
	dice -= ne->flux;
	if ( nw->flux > dice ) return nw;
	dice -= nw->flux;
	if ( se->flux > dice ) return se;
	return sw;
}

QuadTree* QuadTree::getChild( const float2& cylindrical )
{
	bool west = cylindrical.x<xPlane, north = cylindrical.y> yPlane;
	return north ? ( west ? nw : ne ) : ( west ? sw : se );
}
void QuadTree::depositEnergy( const float3& direction, float receivedEnergy )
{
	float2 cylindrical = directionToCylindrical( direction );
	QuadTree* current = this;
	while ( true )
	{
		current->flux += receivedEnergy;
		if ( current->isLeaf() ) break;
		current = current->getChild( cylindrical );
	}
}
float QuadTree::pdf( float3 direction )
{
	float2 cylindrical = directionToCylindrical( direction );
	return pdf( cylindrical );
}
float QuadTree::pdf( float2 cylindrical )
{
	if ( isLeaf() ) return 1 / ( 4 * PI );
	auto child = getChild( cylindrical );
	float beta = 4 * child->flux / flux;
	return beta * child->pdf( cylindrical );
}
void QuadTree::splitLeaf()
{
	float2 middle{ topLeft.x + ( bottomRight.x - topLeft.x ) / 2, bottomRight.y + ( topLeft.y - bottomRight.y ) / 2 };
	xPlane = middle.x;
	yPlane = middle.y;
	nw = new QuadTree( topLeft, middle );
	ne = new QuadTree( make_float2( middle.x, topLeft.y ), make_float2( bottomRight.x, middle.y ) );
	se = new QuadTree( middle, bottomRight );
	sw = new QuadTree( make_float2( topLeft.x, middle.y ), make_float2( middle.x, bottomRight.y ) );
}
} // namespace lh2core