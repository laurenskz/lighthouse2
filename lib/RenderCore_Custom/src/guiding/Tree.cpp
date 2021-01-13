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
float3 SpatialNode::replaceAxis( const float3& point, float value, AXIS axis )
{
	return axis == X ? make_float3( value, point.y, point.z ) : axis == Y ? make_float3( point.x, value, point.z )
																		  : make_float3( point.x, point.y, value );
}
float SpatialNode::midPoint( const float3& min, const float3& max, AXIS axis )
{
	return axis == X ? ( max.x - min.x ) / 2 + min.x : axis == Y ? ( max.y - min.y ) / 2 + min.y
																 : ( max.z - min.z ) / 2 + min.z;
}
void SpatialNode::splitAllAbove( int visits )
{
	if ( !left.isLeaf )
	{
		left.node->splitAllAbove( visits );
	}
	else
	{
		if ( left.leaf->visitCount >= visits )
			left = splitLeaf( left, min, replaceAxis( max, splitPane, splitAxis ) );
	}
	if ( !right.isLeaf )
	{
		right.node->splitAllAbove( visits );
	}
	else
	{
		if ( right.leaf->visitCount >= visits )
			right = splitLeaf( right, replaceAxis( min, splitPane, splitAxis ), max );
	}
}
SpatialNode::SpatialChild SpatialNode::splitLeaf( const SpatialNode::SpatialChild& child, const float3& newMin, const float3& newMax ) const
{
	AXIS newAxis = splitAxis == X ? Y : splitAxis == Y ? Z
													   : X;
	auto node = new SpatialNode( newAxis, SpatialChild( new SpatialLeaf( *child.leaf ) ), SpatialChild( new SpatialLeaf( *child.leaf ) ), newMin, newMax );
	return SpatialNode::SpatialChild( node );
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
SpatialNode::SpatialChild::SpatialChild( const SpatialNode::SpatialChild& other )
{
	if ( other.isLeaf )
	{
		isLeaf = true;
		leaf = new SpatialLeaf( *other.leaf );
	}
	else
	{
		node = new SpatialNode( *other.node );
	}
}
SpatialNode::SpatialChild SpatialNode::newLeaf()
{
	return SpatialNode::SpatialChild( new SpatialLeaf( new QuadTree( make_float2( 0, 1 ), make_float2( 1, 0 ) ) ) );
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

/**
 * PDF considers the entire hemisphere, so not only the top part.
 */
float QuadTree::pdf( float3 direction )
{
	float2 cylindrical = directionToCylindrical( direction );
	return pdf( cylindrical );
}

float QuadTree::pdf( float2 cylindrical )
{
	if ( flux < 1e-7 ) return 0;
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
QuadTree::QuadTree( const QuadTree& other )
{
	flux = other.flux;
	xPlane = other.xPlane;
	yPlane = other.yPlane;
	topLeft = other.topLeft;
	bottomRight = other.bottomRight;
	if ( !other.isLeaf() )
	{
		nw = new QuadTree( *other.nw );
		sw = new QuadTree( *other.sw );
		ne = new QuadTree( *other.ne );
		se = new QuadTree( *other.se );
	}
}
void QuadTree::splitAllAbove( float fluxThreshold )
{
	if ( flux < fluxThreshold ) return;
	if ( isLeaf() )
	{
		splitLeaf();
	}
	else
	{
		nw->splitAllAbove( fluxThreshold );
		ne->splitAllAbove( fluxThreshold );
		sw->splitAllAbove( fluxThreshold );
		se->splitAllAbove( fluxThreshold );
	}
}
void SpatialLeaf::misOptimizationStep( const float3& position, const Sample& sample, float radianceEstimate, float foreshortening, float lightTransport )
{
	float productEstimate = radianceEstimate * foreshortening * lightTransport;
	float alpha = this->brdfProb();
	float deltaAlpha = -productEstimate * ( sample.bsdfPdf - sample.combinedPdf ) / ( sample.guidingPdf * sample.combinedPdf );
	float deltaTheta = deltaAlpha * alpha * ( 1 - alpha );
	float regGradient = regularization * theta;
	adamStep( deltaTheta + regGradient );
}
void SpatialLeaf::adamStep( float deltaTheta )
{
	t++;
	float l = lr * sqrt( 1 - pow( beta1, t ) / ( 1 - pow( beta2, t ) ) ); //Calculate learning rate for iteration
	m = beta1 * m + ( 1 - beta1 ) * deltaTheta;							  //Update first moment
	v = beta2 * v + ( 1 - beta2 ) * deltaTheta * deltaTheta;			  //Update second moment
	theta = theta - ( l * m ) / ( sqrt( v ) + eps );					  //Apply moments to the learned parameter
}
float SpatialLeaf::brdfProb() const
{
	return 1.f / ( 1.f + exp( -theta ) );
}
SpatialLeaf::SpatialLeaf( const SpatialLeaf& other )
{
	this->theta = other.theta;
	this->visitCount = other.visitCount;
	this->m = other.m;
	this->v = other.v;
	this->t = other.t;
	this->beta1 = other.beta1;
	this->beta2 = other.beta2;
	this->eps = other.eps;
	this->lr = other.lr;
	this->regularization = other.regularization;
	this->directions = new QuadTree( *other.directions );
}
} // namespace lh2core