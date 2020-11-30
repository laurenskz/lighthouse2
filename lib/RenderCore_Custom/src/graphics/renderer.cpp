//
// Created by laurens on 11/23/20.
//
#include "graphics/renderer.h"
#include <thread>
namespace lh2core
{
const float2 OFFSETS[]{ make_float2( 0.1, 0.25 ), make_float2( 0.7, 0.1 ), make_float2( 0.8, 0.25 ), make_float2( 0.9, 0.7 ) };

float3 BasePixelRenderer::render( const ViewPyramid& view, float x, float y, float width, float height )
{
	Ray ray{};
	ray.start = view.pos;
	const float3& rayDirection = RayTracer::rayDirection( ( x / width ), ( y / height ), view );
	ray.direction = rayDirection;
	return rayTracer->trace( ray, 3 );
}
void SingleCoreRenderer::renderTo( const ViewPyramid& view, Bitmap* screen )
{
	Ray ray{};
	ray.start = view.pos;
	for ( int y = 0; y < screen->height; ++y )
	{
		for ( int x = 0; x < screen->width; ++x )
		{
			const float3& rayDirection = RayTracer::rayDirection( ( x / (float)screen->width ), ( y / (float)screen->height ), view );
			ray.direction = rayDirection;
			const float3& fColor = rayTracer->trace( ray, 3 );
			plotColor( screen, y, x, fColor );
		}
	}
}

void plotColor( Bitmap* screen, int y, int x, const float3& fColor )
{
	int r = clamp( (int)( fColor.x * 256 ), 0, 255 );
	int g = clamp( (int)( fColor.y * 256 ), 0, 255 );
	int b = clamp( (int)( fColor.z * 256 ), 0, 255 );
	screen->Plot( x, y, ( b << 16 ) + ( g << 8 ) + ( r ) );
}

void MultiThreadedRenderer::renderTo( const ViewPyramid& view, Bitmap* screen )
{
	uint rowsPerThread = ceil( (float)screen->height / (float)threadPool->size() );
	std::vector<std::future<void>> results( threadPool->size() );
	for ( int i = 0; i < threadPool->size(); ++i )
	{
		results[i] = threadPool->push( [this, view, screen, rowsPerThread, i]( int _ ) { renderRows( view, screen, i * rowsPerThread, ( i + 1 ) * rowsPerThread ); } );
	}
	for ( auto& result : results )
	{
		result.get();
	}
}

void MultiThreadedRenderer::renderRows( const ViewPyramid& view, Bitmap* screen, int start, uint end )
{
	end = std::min( end, screen->height );
	for ( int y = start; y < end; ++y )
	{
		for ( int x = 0; x < screen->width; ++x )
		{
			const float3& fColor = pixelRenderer->render( view, x, y, screen->width, screen->height );
			plotColor( screen, y, x, fColor );
		}
	}
}
MultiThreadedRenderer::MultiThreadedRenderer( PixelRenderer* pixelRenderer )
{
	this->pixelRenderer = pixelRenderer;
	auto const cpuCount = std::max( (uint)1, std::thread::hardware_concurrency() );
	threadPool = new ctpl::thread_pool( cpuCount );
}
float3 AntiAliasedRenderer::render( const ViewPyramid& view, float x, float y, float width, float height )
{
	auto result = make_float3( 0 );
	for ( auto offset : OFFSETS )
	{
		const float3& color = renderer->render( view, x + offset.x, y + offset.y, width, height );
		result += color;
	}
	return result / 4;
}
} // namespace lh2core
