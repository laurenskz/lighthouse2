﻿/* rendercore.cpp - Copyright 2019/2020 Utrecht University

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
	rayTracer = RayTracer();
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
	if ( meshIdx < 0 ) return;
	rayTracer.scene.mesh->transform = matrix;
}
//  +-----------------------------------------------------------------------------+
//  |  RenderCore::SetGeometry                                                    |
//  |  Set the geometry data for a model.                                   LH2'19|
//  +-----------------------------------------------------------------------------+
void RenderCore::SetGeometry( const int meshIdx, const float4* vertexData, const int vertexCount, const int triangleCount, const CoreTri* triangles )
{
	Mesh* newMesh = new Mesh( vertexCount );
	for ( int i = 0; i < vertexCount; ++i )
	{
		newMesh->positions[i] =vertexData[i] ;
	}
	for ( int i = 0; i < triangleCount; i++ )
	{
		newMesh->normals[i * 3 + 0] = triangles[i].vN0;
		newMesh->normals[i * 3 + 1] = triangles[i].vN1;
		newMesh->normals[i * 3 + 2] = triangles[i].vN2;
	}
	rayTracer.scene.mesh = newMesh;
}

//  +-----------------------------------------------------------------------------+
//  |  RenderCore::Render                                                         |
//  |  Produce one image.                                                   LH2'19|
//  +-----------------------------------------------------------------------------+
void RenderCore::Render( const ViewPyramid& view, const Convergence converge, bool async )
{
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
			const float3& fColor = rayTracer.trace( ray );
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