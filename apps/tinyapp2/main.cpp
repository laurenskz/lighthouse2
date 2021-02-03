/* main.cpp - Copyright 2019/2020 Utrecht University

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

#include "platform.h"
#include "rendersystem.h"

#include <bitset>

static RenderAPI* renderer = 0;
static GLTexture* renderTarget = 0;
static Shader* shader = 0;
static uint scrwidth = 0, scrheight = 0, car = 0, scrspp = 1;
static bool running = true;

static std::bitset<1024> keystates;

#include "main_tools.h"

//  +-----------------------------------------------------------------------------+
//  |  PrepareScene                                                               |
//  |  Initialize a scene.                                                  LH2'19|
//  +-----------------------------------------------------------------------------+
void PrepareScene()
{
	// initialize scene
	int planeIdx = renderer->AddMesh( "../demodata/plane/plane.obj" );
	auto matId = renderer->FindMaterialID( "Texture" );
	renderer->AddInstance( planeIdx, mat4::Scale( 4.8 ) );
	HostMaterial* material = renderer->GetMaterial( matId );
	material->pbrtMaterialType = lighthouse2::MaterialType::CUSTOM_BSDF;
	material->specular = 0.4;
	material->Ks = make_float3( 1 );
	material->clearcoatGloss = 5000;

	auto tetMat = renderer->AddMaterial( make_float3( 1 ) );
	int quad = renderer->AddQuad( make_float3( 0, 0, 1 ), make_float3( 8.5, 3.5, -11 ), 1, 1, tetMat );
	renderer->AddInstance( quad );
	HostMaterial* reflMat = renderer->GetMaterial( tetMat );
	reflMat->pbrtMaterialType = lighthouse2::MaterialType::CUSTOM_BSDF;
	reflMat->specular = 1;
	reflMat->Ks = make_float3( 1 );
	reflMat->clearcoatGloss = 5;

	auto sky = new HostSkyDome();
	sky->Load( "../demodata/sky_15.hdr" );
	renderer->GetScene()->SetSkyDome( sky );
	//	For path tracer
	int lightMat = renderer->AddMaterial( make_float3( 100 ) );
	HostMaterial* mat = renderer->GetMaterial( lightMat );
	mat->pbrtMaterialType = MaterialType::PBRT_UBER;
	int lightQuad = renderer->AddQuad( make_float3( 0, -1, 0 ), make_float3( 8, 4.5f, -10 ), 0.9f, 0.9f, lightMat );
	renderer->AddInstance( lightQuad );
	int mesh = renderer->AddMesh( "../demodata/spaceman/untitled.obj" );
	renderer->AddInstance( mesh, mat4::Translate( -1.5, 0, 0 ) * mat4::Scale( 0.3 ) );
	//		Directional light
	renderer->AddDirectionalLight( normalize( make_float3( -1 ) ), make_float3( 1.0 / 2 ) );
}

//  +-----------------------------------------------------------------------------+
//  |  HandleInput                                                                |
//  |  Process user input.                                                  LH2'19|
//  +-----------------------------------------------------------------------------+
void HandleInput( float frameTime )
{
	// handle keyboard input
	float spd = ( keystates[GLFW_KEY_LEFT_SHIFT] ? 15.0f : 5.0f ) * frameTime, rot = 2.5f * frameTime;
	spd *= 5;
	Camera* camera = renderer->GetCamera();
	if ( keystates[GLFW_KEY_A] ) camera->TranslateRelative( make_float3( -spd, 0, 0 ) );
	if ( keystates[GLFW_KEY_D] ) camera->TranslateRelative( make_float3( spd, 0, 0 ) );
	if ( keystates[GLFW_KEY_W] ) camera->TranslateRelative( make_float3( 0, 0, spd ) );
	if ( keystates[GLFW_KEY_S] ) camera->TranslateRelative( make_float3( 0, 0, -spd ) );
	if ( keystates[GLFW_KEY_R] ) camera->TranslateRelative( make_float3( 0, spd, 0 ) );
	if ( keystates[GLFW_KEY_F] ) camera->TranslateRelative( make_float3( 0, -spd, 0 ) );
	if ( keystates[GLFW_KEY_UP] ) camera->TranslateTarget( make_float3( 0, -rot, 0 ) );
	if ( keystates[GLFW_KEY_DOWN] ) camera->TranslateTarget( make_float3( 0, rot, 0 ) );
	if ( keystates[GLFW_KEY_LEFT] ) camera->TranslateTarget( make_float3( -rot, 0, 0 ) );
	if ( keystates[GLFW_KEY_RIGHT] ) camera->TranslateTarget( make_float3( rot, 0, 0 ) );
}

//  +-----------------------------------------------------------------------------+
//  |  main                                                                       |
//  |  Application entry point.                                             LH2'19|
//  +-----------------------------------------------------------------------------+
int main()
{
	// initialize OpenGL
	InitGLFW();
	renderer = RenderAPI::CreateRenderAPI( "RenderCore_Custom" ); // OPTIX PRIME, best for pre-RTX CUDA devices
	//	renderer = RenderAPI::CreateRenderAPI( "RenderCore_SoftRasterizer" ); // RASTERIZER, your only option if not on NVidia
	//	 renderer = RenderAPI::CreateRenderAPI( "RenderCore_Vulkan_RT" );			// Meir's Vulkan / RTX core
	// renderer = RenderAPI::CreateRenderAPI( "RenderCore_OptixPrime_BDPT" );	// Peter's OptixPrime / BDPT core
	renderer->GetCamera()->LookAt( make_float3( 0, 2, 10 ), make_float3( 0, 2, 0 ) );
	//	renderer->DeserializeCamera( "camera.xml" );
	// initialize scene
	PrepareScene();
	// set initial window size
	ReshapeWindowCallback( 0, SCRWIDTH, SCRHEIGHT );
	Timer timer;
	timer.reset();
	float deltaTime = 0;

	// enter main loop
	while ( !glfwWindowShouldClose( window ) )
	{
		// update scene
		renderer->SynchronizeSceneData();
		// render
		renderer->Render( Restart /* alternative: converge */ );
		// handle user input
		deltaTime = timer.elapsed();
		HandleInput( 0.025f );

		timer.reset();
		// minimal rigid animation example
		shader->Bind();
		shader->SetInputTexture( 0, "color", renderTarget );
		shader->SetInputMatrix( "view", mat4::Identity() );
		DrawQuad();
		shader->Unbind();
		// finalize
		glfwSwapBuffers( window );
		glfwPollEvents();
		if ( !running ) break; // esc was pressed
	}
	// clean up
	renderer->SerializeCamera( "camera.xml" );
	renderer->Shutdown();
	glfwDestroyWindow( window );
	glfwTerminate();
	return 0;
}

// EOF