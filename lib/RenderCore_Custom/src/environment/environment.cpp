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
	Intersection intersection = geometry->intersectionInformation( *nearest.primitive, nearest.minDistance, r );
	intersection.hitObject = true;
	return intersection;
}
} // namespace lh2core
