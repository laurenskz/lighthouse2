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
	geometry->addSphere( make_float3( 0.1 ), 1, Material{ make_float3( 1, 0, 0 ) } );
	geometry->addPlane( make_float3( 0, 1, 0 ), 3 );
	intersector = new BruteForceIntersector();
	auto* env = new Environment( geometry, intersector );
	lighting = new Lighting( intersector );
	rayTracer = new RayTracer( env, lighting );
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
	geometry->setInstance( instanceIdx, meshIdx, matrix );
}

void RenderCore::SetLights( const CoreLightTri* triLights, const int triLightCount,
							const CorePointLight* pointLights, const int pointLightCount,
							const CoreSpotLight* spotLights, const int spotLightCount,
							const CoreDirectionalLight* directionalLights, const int directionalLightCount )
{
	lighting->SetLights( triLights, triLightCount, pointLights, pointLightCount, spotLights, spotLightCount, directionalLights, directionalLightCount );
}
void RenderCore::FinalizeInstances()
{
	geometry->finalizeInstances();
}
//  +-----------------------------------------------------------------------------+
//  |  RenderCore::SetGeometry                                                    |
//  |  Set the geometry data for a model.                                   LH2'19|
//  +-----------------------------------------------------------------------------+
void RenderCore::SetGeometry( const int meshIdx, const float4* vertexData, const int vertexCount, const int triangleCount, const CoreTri* triangles )
{
	geometry->setGeometry( meshIdx, vertexData, vertexCount, triangleCount, triangles );
}

//  +-----------------------------------------------------------------------------+
//  |  RenderCore::Render                                                         |
//  |  Produce one image.                                                   LH2'19|
//  +-----------------------------------------------------------------------------+
void RenderCore::Render( const ViewPyramid& view, const Convergence converge, bool async )
{
	auto primitives = geometry->getPrimitives();
	intersector->setPrimitives( primitives.data, primitives.size );
	// render
	screen->Clear();
	Ray ray{};
	ray.start = view.pos;
	for ( int y = 0; y < screen->height; ++y )
	{
		for ( int x = 0; x < screen->width; ++x )
		{
			const float3& rayDirection = RayTracer::rayDirection( ( x / (float)screen->width ), ( y / (float)screen->height ), view );
			ray.direction = rayDirection;
			const float3& fColor = rayTracer->trace( ray, 3 );
			int r = clamp( (int)( fColor.x * 256 ), 0, 255 );
			int g = clamp( (int)( fColor.y * 256 ), 0, 255 );
			int b = clamp( (int)( fColor.z * 256 ), 0, 255 );
			screen->Plot( x, y, ( b << 16 ) + ( g << 8 ) + ( r ) );
		}
	}
	cout << "Done with frame" << endl;
	// copy pixel buffer to OpenGL render target texture
	glBindTexture( GL_TEXTURE_2D, targetTextureID );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, screen->width, screen->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, screen->pixels );
}

//  +-----------------------------------------------------------------------------+
//  |  RenderCore::GetCoreStats                                                   |
//  |  Get a copy of the counters.                                          LH2'19|
//  +-----------------------------------------------------------------------------+
CoreStats RenderCore::GetCoreStats() const
{
	return coreStats;
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