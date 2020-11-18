#include "environment/environment.h"
namespace lh2core
{

Intersection Environment::intersect( const Ray& r )
{
	const ShortestDistance& nearest = intersector->nearest( r );
	if ( nearest.minDistance == MAX_DISTANCE )
	{
		return Intersection(); //We hit nothing
	}
	return geometry->intersectionInformation( *nearest.primitive, nearest.minDistance, r );
}
} // namespace lh2core
