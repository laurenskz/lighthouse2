//
// Created by laurens on 11/23/20.
//

#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/intersections.h"
#include "graphics/raytracer.h"
#include "threading/ctpl_stl.h"

namespace lh2core
{
class PixelRenderer
{
	virtual float3 render( const ViewPyramid& view, int x, int y, int width, int height ) = 0;
};
class Renderer
{
  public:
	virtual void renderTo( const ViewPyramid& view, Bitmap* screen ) = 0;
};

class BasePixelRenderer : public PixelRenderer
{
	float3 render( const ViewPyramid& view, int x, int y, int width, int height ) override;
};

float3 renderAntialised( const ViewPyramid& view, Bitmap* screen, RayTracer* rayTracer, int x, int y );
void plotColor( Bitmap* screen, int y, int x, const float3& fColor );
class SingleCoreRenderer : public Renderer
{
  public:
	explicit SingleCoreRenderer( RayTracer* rayTracer ) : rayTracer( rayTracer ){};
	void renderTo( const ViewPyramid& view, Bitmap* screen ) override;

  private:
	RayTracer* rayTracer;
};

class MultiThreadedRenderer : public Renderer
{
  public:
	explicit MultiThreadedRenderer( RayTracer* rayTracer );
	void renderTo( const ViewPyramid& view, Bitmap* screen ) override;

  private:
	ctpl::thread_pool* threadPool;
	RayTracer* rayTracer;
	void renderRows( const ViewPyramid& view, Bitmap* screen, int start, uint end );
};

} // namespace lh2core