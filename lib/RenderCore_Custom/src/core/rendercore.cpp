/* rendercore.cpp - Copyright 2019/2020 Utrecht University

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "core_settings.h"

using namespace lh2core;

//  +-----------------------------------------------------------------------------+
//  |  RenderCore::Init                                                           |
//  |  Initialization.                                                      LH2'19|
//  +-----------------------------------------------------------------------------+
void RenderCore::Init()
{
	// initialize core
	geometry = new Geometry();
	//	geometry->addSphere( make_float3( 0.5, -0.9, 1.5 ), 0.5, Material{ make_float3( 1, 0, 0 ) } );
	//	geometry->addSphere( make_float3( -3, -0.3, -2 ), 0.5, Material{ make_float3( 0 ), 0, GLASS, 1.5 } );
	//	geometry->addPlane( make_float3( 0, 1, 0 ), 1 );
	//	intersector = new BruteForceIntersector();
	intersector = new TopLevelBVH();
	environment = new Environment( geometry, intersector );
	lighting = new Lighting( intersector );
#ifdef GUIDED
	rayTracer = new PathGuidingTracer( environment, new BRDFs() );
	PixelRenderer* baseRenderer = new PathGuidingRenderer( rayTracer );
#ifdef MULTITHREADED
	renderer = new MultiThreadedRenderer( baseRenderer );
#else
	renderer = new SingleCoreRenderer( baseRenderer );
#endif
#else
	PathTracer* pTracer = new PathTracer( environment, lighting );
	renderer = new MultiThreadedRenderer( new AveragingPixelRenderer( new BasePixelRenderer( pTracer ) ) );
#endif
}

//  +-----------------------------------------------------------------------------+
//  |  RenderCore::SetTarget                                                      |
//  |  Set the OpenGL texture that serves as the render target.             LH2'19|
//  +-----------------------------------------------------------------------------+
void RenderCore::SetTarget( GLTexture* target, const uint )
{
	// synchronize OpenGL viewport
	targetTextureID = target->ID;
	if ( screen != 0 && target->width == screen->width && target->height == screen->height ) return; // nothing changed
	delete screen;
	screen = new Bitmap( target->width, target->height );
}

void RenderCore::SetInstance( const int instanceIdx, const int meshIdx, const mat4& matrix )
{
	if ( meshIdx < 0 || instanceIdx < 0 ) return;
	geometry->setInstance( instanceIdx, meshIdx, matrix );
	intersector->setInstance( instanceIdx, meshIdx, matrix );
}

void RenderCore::SetLights( const CoreLightTri* triLights, const int triLightCount,
							const CorePointLight* pointLights, const int pointLightCount,
							const CoreSpotLight* spotLights, const int spotLightCount,
							const CoreDirectionalLight* directionalLights, const int directionalLightCount )
{
	lighting->SetLights( triLights, triLightCount, pointLights, pointLightCount, spotLights, spotLightCount, directionalLights, directionalLightCount );
#ifndef WHITTED
	geometry->SetLights( triLights, triLightCount );
#endif
}
void RenderCore::FinalizeInstances()
{
	//	geometry->finalizeInstances();
	intersector->finalize();
}
//  +-----------------------------------------------------------------------------+
//  |  RenderCore::SetGeometry                                                    |
//  |  Set the geometry data for a model.                                   LH2'19|
//  +-----------------------------------------------------------------------------+
void RenderCore::SetGeometry( const int meshIdx, const float4* vertexData, const int vertexCount, const int triangleCount, const CoreTri* triangles )
{
	geometry->setGeometry( meshIdx, vertexData, vertexCount, triangleCount, triangles );
	auto mesh = geometry->getMesh( meshIdx );
	intersector->setMesh( meshIdx, mesh->primitives, mesh->triangleCount );
}

//  +-----------------------------------------------------------------------------+
//  |  RenderCore::Render                                                         |
//  |  Produce one image.                                                   LH2'19|
//  +-----------------------------------------------------------------------------+
void RenderCore::Render( const ViewPyramid& view, const Convergence converge, bool async )
{
	//	auto primitives = geometry->getPrimitives();
	//	intersector->setPrimitives( primitives.data, primitives.size );
	// render
	if ( !feq( view.pos, lastRenderPos, 1e-4 ) )
	{
		const AABB& bounds = intersector->getBounds();
		renderer->cameraChanged( bounds.min, bounds.max, screen->width, screen->height );
	}

	lastRenderPos = view.pos;
	screen->Clear();
	if ( !renderer->isDone() )
	{
		renderer->renderTo( view, screen );
		// copy pixel buffer to OpenGL render target texture
		glBindTexture( GL_TEXTURE_2D, targetTextureID );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, screen->width, screen->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, screen->pixels );
	}
	else
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
		cout << "Done rendering" << endl;
	}
}

void RenderCore::SetTextures( const CoreTexDesc* tex, const int textureCount )
{
	geometry->SetTextures( tex, textureCount );
}
void RenderCore::SetMaterials( CoreMaterial* mat, const int materialCount )
{
	geometry->SetMaterials( mat, materialCount );
}

//  +-----------------------------------------------------------------------------+
//  |  RenderCore::GetCoreStats                                                   |
//  |  Get a copy of the counters.                                          LH2'19|
//  +-----------------------------------------------------------------------------+
CoreStats RenderCore::GetCoreStats() const
{
	return coreStats;
}

void RenderCore::SetSkyData( const float3* pixels, const uint width, const uint height, const mat4& worldToLight )
{
	environment->SetSkyData( pixels, width, height );
}

//  +-----------------------------------------------------------------------------+
//  |  RenderCore::Shutdown                                                       |
//  |  Free all resources.                                                  LH2'19|
//  +-----------------------------------------------------------------------------+
void RenderCore::Shutdown()
{
	delete screen;
}

// EOF