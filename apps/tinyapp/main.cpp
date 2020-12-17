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

struct Spaceman
{
	int nodeIdx;
	float scale;
	float3 start;
	float3 rotation;
	float3 velocity;
};

#define SPACEMAN_COUNT 20
static RenderAPI* renderer = 0;
static GLTexture* renderTarget = 0;
static Shader* shader = 0;
static uint scrwidth = 0, scrheight = 0, car = 0, scrspp = 1;
static bool running = true;
static Spaceman* spacemans;

static std::bitset<1024> keystates;

#include "main_tools.h"

//  +-----------------------------------------------------------------------------+
//  |  PrepareScene                                                               |
//  |  Initialize a scene.                                                  LH2'19|
//  +-----------------------------------------------------------------------------+
void PrepareScene()
{
	// initialize scene
	renderer->AddScene( "CesiumMan.gltf", "../demodata/CesiumMan/glTF" );
	int mesh = renderer->AddMesh( "../demodata/spaceman/untitled.obj" );
	int planeIdx = renderer->AddMesh( "../demodata/plane/plane.obj" );
	renderer->AddInstance( planeIdx, mat4::Scale( 4 ) );
	spacemans = new Spaceman[SPACEMAN_COUNT];
	for ( int i = 0; i < SPACEMAN_COUNT; ++i )
	{
		spacemans[i].scale = 0.1;
		spacemans[i].start = make_float3( (float)rand() * 3 / RAND_MAX, (float)rand() * 3 / RAND_MAX, (float)rand() * 3 / RAND_MAX );
		spacemans[i].velocity = make_float3( (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX ) / 1.5;
		spacemans[i].nodeIdx = renderer->AddInstance( mesh, mat4::Translate( spacemans[i].start ) * mat4::Scale( spacemans[i].scale ) );
	}
	auto sky = new HostSkyDome();
	sky->Load( "../demodata/sky_15.hdr" );
	renderer->GetScene()->SetSkyDome( sky );

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
	renderer->GetCamera()->LookAt( make_float3( 0, 3, 10 ), make_float3( 0, 2, 0 ) );
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
		for ( int i = 0; i < renderer->AnimationCount(); i++ )
		{
			renderer->UpdateAnimation( i, deltaTime );
		}
		deltaTime = timer.elapsed();
		HandleInput( 0.025f );

		for ( int i = 0; i < SPACEMAN_COUNT; ++i )
		{
			const float3& newPos = spacemans[i].start + deltaTime * spacemans[i].velocity;
			if ( fabs( newPos.x ) > 4 ) spacemans[i].velocity.x = -spacemans[i].velocity.x;
			if ( newPos.y < 0 || newPos.y > 3 ) spacemans[i].velocity.y = -spacemans[i].velocity.y;
			if ( fabs( newPos.z ) > 4 ) spacemans[i].velocity.z = -spacemans[i].velocity.z;
			spacemans[i].start += deltaTime * spacemans[i].velocity;
			renderer->SetNodeTransform( spacemans[i].nodeIdx, mat4::Translate( spacemans[i].start ) * mat4::Scale( spacemans[i].scale ) );
		}
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