//
// Created by laurens on 11/23/20.
//
#include "graphics/renderer.h"
#include <thread>
namespace lh2core
{
const float2 OFFSETS[]{ make_float2( 0.1, 0.25 ), make_float2( 0.7, 0.1 ), make_float2( 0.8, 0.25 ), make_float2( 0.9, 0.7 ) };

float3 BasePixelRenderer::render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection )
{
	ray.start = view.pos;
	const float3& rayDirection = RayTracer::rayDirection( ( x / width ), ( y / height ), view );
	ray.direction = rayDirection;
	return rayTracer->trace( ray, 5 );
}
void SingleCoreRenderer::renderTo( const ViewPyramid& view, Bitmap* screen )
{
	pixelRenderer->beforeRender( view, screen->width, screen->height );
	Ray ray{};
	Intersection intersection{};
	for ( int y = 0; y < screen->height; ++y )
	{
		for ( int x = 0; x < screen->width; ++x )
		{
			const float3& fColor = pixelRenderer->render( view, x, y, screen->width, screen->height, ray, intersection );
			plotColor( screen, y, x, fColor );
		}
	}
	pixelRenderer->afterRender();
}
void SingleCoreRenderer::cameraChanged( const float3& geometryMin, const float3& geometryMax, int width, int height )
{
	pixelRenderer->cameraChanged( geometryMin, geometryMax, width, height );
}
bool SingleCoreRenderer::isDone()
{
	return pixelRenderer->isDone();
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
	pixelRenderer->beforeRender( view, screen->width, screen->height );
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
	pixelRenderer->afterRender();
}

void MultiThreadedRenderer::renderRows( const ViewPyramid& view, Bitmap* screen, int start, uint end )
{
	end = std::min( end, screen->height );
	Ray ray{};
	Intersection intersection{};
	for ( int y = start; y < end; ++y )
	{
		for ( int x = 0; x < screen->width; ++x )
		{
			const float3& fColor = pixelRenderer->render( view, x, y, screen->width, screen->height, ray, intersection );
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
void MultiThreadedRenderer::cameraChanged( const float3& geometryMin, const float3& geometryMax, int width, int height )
{
	pixelRenderer->cameraChanged( geometryMin, geometryMax, width, height );
}
bool MultiThreadedRenderer::isDone()
{
	return pixelRenderer->isDone();
}
float3 AntiAliasedRenderer::render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection )
{
	auto result = make_float3( 0 );
	for ( auto offset : OFFSETS )
	{
		const float3& color = renderer->render( view, x + offset.x, y + offset.y, width, height, ray, intersection );
		result += color;
	}
	return result / 4;
}
float3 AveragingPixelRenderer::render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection )
{
	int pixelIndex = floor( y * round( width ) + x );
	pixelData[pixelIndex] += renderer->render( view, x, y, width, height, ray, intersection );
	return pixelData[pixelIndex] / numFrames;
}

void AveragingPixelRenderer::beforeRender( const ViewPyramid& view, int width, int height )
{
	renderer->beforeRender( view, width, height );
	dirty = dirty || !feq( view.pos, lastRenderPos, 1e-4 );
	if ( ( width * height ) != this->numPixels || dirty )
	{
		dirty = false;
		numPixels = width * height;
		numFrames = 0;
		pixelData = new float3[numPixels];
	}
	lastRenderPos = view.pos;
	numFrames += 1;
}
float3 TestPixelRenderer::render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection )
{
	return make_float3( 1 ) * count / 10.0;
}
void TestPixelRenderer::beforeRender( const ViewPyramid& view, int width, int height )
{
	count++;
	sleep( 1 );
}
float3 PathGuidingRenderer::render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection )
{
	ray.start = view.pos;
	const float3& rayDirection = RayTracer::rayDirection( ( x / width ), ( y / height ), view );
	ray.direction = rayDirection;
	return tracer->performSample( ray, round( x ), round( y ) );
}
void PathGuidingRenderer::beforeRender( const ViewPyramid& view, int width, int height )
{
	PixelRenderer::beforeRender( view, width, height );
	tracer->iterationStarted();
}
void PathGuidingRenderer::cameraChanged( const float3& geometryMin, const float3& geometryMax, int width, int height )
{
	tracer->cameraChanged( new TrainModule( geometryMin, geometryMax, width * height ), new ImageBuffer( width, height ) );
}
PathGuidingRenderer::PathGuidingRenderer( PathGuidingTracer* tracer ) : tracer( tracer ) {}
void PathGuidingRenderer::afterRender()
{
	PixelRenderer::afterRender();
	tracer->iterationFinished();
}
bool PathGuidingRenderer::isDone()
{
	return tracer->isDone();
}
} // namespace lh2core
