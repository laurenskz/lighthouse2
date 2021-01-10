
#include "guiding/utils.h"
namespace lh2core
{

float randFloat()
{
	return static_cast<float>( random() ) / static_cast<float>( RAND_MAX );
}
}