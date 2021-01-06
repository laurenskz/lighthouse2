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
} // namespace lh2core