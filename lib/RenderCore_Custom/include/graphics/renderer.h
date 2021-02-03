//
// Created by laurens on 11/23/20.
//

#pragma once
#include "platform.h"

using namespace lighthouse2;
#include "core/base_definitions.h"
#include "environment/intersections.h"
#include "graphics/raytracer.h"
#include "guiding/PathGuidingTracer.h"
#include "threading/ctpl_stl.h"

namespace lh2core
{
class PixelRenderer
{
  public:
	virtual float3 render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection ) = 0;
	virtual void beforeRender( const ViewPyramid& view, int width, int height ){};
	virtual void afterRender(){};
	virtual void cameraChanged( const float3& geometryMin, const float3& geometryMax, int width, int height ){};
	virtual bool isDone(){return false;}
};

class PathGuidingRenderer : public PixelRenderer
{
  private:
	PathGuidingTracer* tracer;

  public:
	explicit PathGuidingRenderer( PathGuidingTracer* tracer );

  public:
	float3 render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection ) override;
	void beforeRender( const ViewPyramid& view, int width, int height ) override;
	void afterRender() override;
	void cameraChanged( const float3& geometryMin, const float3& geometryMax, int width, int height ) override;
	bool isDone() override;
};

class TestPixelRenderer : public PixelRenderer
{
  public:
	void beforeRender( const ViewPyramid& view, int width, int height ) override;
	float3 render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection ) override;
	float count = 0;
};
class Renderer
{
  public:
	virtual void renderTo( const ViewPyramid& view, Bitmap* screen ) = 0;
	virtual void cameraChanged( const float3& geometryMin, const float3& geometryMax, int width, int height ){};
	virtual bool isDone(){return false;}
};

class BasePixelRenderer : public PixelRenderer
{
  private:
	IRayTracer* rayTracer;

  public:
	BasePixelRenderer( IRayTracer* tracer ) : rayTracer( tracer ){};
	float3 render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection ) override;
};

class AveragingPixelRenderer : public PixelRenderer
{
  public:
	float3 render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection ) override;
	void beforeRender( const ViewPyramid& view, int width, int height ) override;
	AveragingPixelRenderer( PixelRenderer* renderer ) : renderer( renderer ){};

  private:
	PixelRenderer* renderer;
	int numPixels = 0;
	int numFrames = 0;
	float3* pixelData{};
	bool dirty = true;
	float3 lastRenderPos{};
};

class AntiAliasedRenderer : public PixelRenderer
{
  public:
	AntiAliasedRenderer( PixelRenderer* renderer ) : renderer( renderer ){};
	float3 render( const ViewPyramid& view, float x, float y, float width, float height, Ray& ray, Intersection& intersection ) override;

  private:
	PixelRenderer* renderer;
};

void plotColor( Bitmap* screen, int y, int x, const float3& fColor );
class SingleCoreRenderer : public Renderer
{
  public:
	explicit SingleCoreRenderer( PixelRenderer* pixelRenderer ) : pixelRenderer( pixelRenderer ){};
	void renderTo( const ViewPyramid& view, Bitmap* screen ) override;
	void cameraChanged( const float3& geometryMin, const float3& geometryMax, int width, int height ) override;
	bool isDone() override;

  private:
	PixelRenderer* pixelRenderer;
};

class MultiThreadedRenderer : public Renderer
{
  public:
	explicit MultiThreadedRenderer( PixelRenderer* pixelRenderer );
	void renderTo( const ViewPyramid& view, Bitmap* screen ) override;
	void cameraChanged( const float3& geometryMin, const float3& geometryMax, int width, int height ) override;
	bool isDone() override;

  private:
	ctpl::thread_pool* threadPool;
	PixelRenderer* pixelRenderer;
	void renderRows( const ViewPyramid& view, Bitmap* screen, int start, uint end );
};

} // namespace lh2core