#include "RavenApp.h"

#include "core/Frame.h"
#include "graphics/GraphicsDevice.h"
#include "graphics/VulkanInstance.h"
#include "graphics/memory/GPUAllocator.h"
#include "graphics/model.h"
#include "graphics/models/Mesh.h"
#include "graphics/textures/TextureLoader.h"
#include "graphics\PipelineStateObject.h"
#include "graphics\VulkanSwapChain.h"
#include "graphics\buffers\UniformBuffer.h"
#include "helpers/DebugAssert.h"
#include "helpers/Helpers.h"
#include "imgui/imgui.h"
#include "raven/DebugUI.h"
#include "core/FirstPersonCamera.h"

#include <iostream>
#include <thread>
#include <vector>

std::vector<std::function<void( int, int )>> RavenApp::OnWindowResized = {};

RavenApp::RavenApp()
	: _pWindow( nullptr )
	, _pImguiVulkan( nullptr )
	, updateFrameIndex( 0 )
	, _RenderThread()
{
}

RavenApp::~RavenApp()
{
	models.clear();

	delete _pImguiVulkan;

	_RenderThread.Destroy();

	GraphicsDevice::Instance().Finalize();

	glfwDestroyWindow( _pWindow );

	glfwTerminate();
}

void error_callback( int error, const char* description )
{
	puts( description );
}

bool RavenApp::Initialize()
{
	{
		if ( !glfwInit() ) //Initialize GLFW
		{
			std::cout << "GLFW not initialized." << std::endl;
			return false;
		}

		glfwSetErrorCallback( error_callback );

		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
		glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

		_pWindow = glfwCreateWindow( 1280, 720, "Vulkan window", nullptr, nullptr );
	}

	if ( glfwVulkanSupported() == GLFW_TRUE )
	{
		std::cout << "Vulkan is supported." << std::endl;
	}
	else
	{
		std::cout << "Vulkan is NOT supported. Falling back to DirectX11" << std::endl;
	}

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

	std::vector<std::string> windowExtensionsNeeded( glfwExtensions, glfwExtensions + glfwExtensionCount );

	//Note: Ownership given to GraphicsDevice
	GraphicsContext::VulkanInstance = std::make_shared<VulkanInstance>();

	std::cout << glfwExtensionCount << " extensions needed:" << std::endl;
	for ( size_t i = 0; i < windowExtensionsNeeded.size(); i++ )
	{
		if ( !GraphicsContext::VulkanInstance->IsExtensionAvailable( windowExtensionsNeeded[ i ] ) )
		{
			std::cout << "Missing " << windowExtensionsNeeded[ i ] << " extension." << std::endl;
			return false;
		}

		std::cout << windowExtensionsNeeded[ i ] << " extension is available." << std::endl;
	}

	glfwSetWindowUserPointer( _pWindow, this );

	glfwSetWindowSizeCallback( _pWindow, RavenApp::WindowResizedCallback );

	glfwSetMouseButtonCallback( _pWindow, InputEvent::MouseButtonCallback );
	glfwSetScrollCallback( _pWindow, InputEvent::ScrollCallback );
	glfwSetKeyCallback( _pWindow, InputEvent::KeyCallback );
	glfwSetCharCallback( _pWindow, InputEvent::CharCallback );

	GraphicsContext::VulkanInstance->CreateInstance( windowExtensionsNeeded );
	GraphicsContext::VulkanInstance->HookDebugCallback();

	//Note: Ownership given to GraphicsDevice
	std::shared_ptr<VulkanSwapChain> vulkanSwapChain = std::make_shared<VulkanSwapChain>();

	GraphicsContext::GlobalAllocator.PrintStats();
	VkResult result = glfwCreateWindowSurface( GraphicsContext::VulkanInstance->GetNative(),
											   _pWindow,
											   vulkanSwapChain->GetSurface().AllocationCallbacks(),
											   vulkanSwapChain->GetSurface().Replace() );
	assert( result == VK_SUCCESS );

	GraphicsDevice::Instance().Initialize( glm::u32vec2( 1280, 720 ), vulkanSwapChain );

	GraphicsContext::GlobalAllocator.PrintStats();

	_RenderThread.Initialize();

	_pImguiVulkan = new IMGUIVulkan( _pWindow );
	IMGUI_CHECKVERSION();
	ImGui::CreateContext( nullptr );
	_pImguiVulkan->Initialize( );

	std::shared_ptr<Model> model = std::make_shared<Model>( "cesiumman" ); //"boy"
	models.push_back( model );

	return true;
}

void RavenApp::Run()
{
	_Run = true;

#if MULTITHREADED_RENDERING
	GraphicsContext::GlobalAllocator.PrintStats();
	_RenderThread.Start();
#endif

	static bool autoRotate = true;
	static float rotation = 310;
	float translationY = -1;
	//float scale = 0.025;
	float scale = 1;

	float StatsTimer = 29.0f;
	bool DoUpdate = true;
	bool DoQuit = false;

	FirstPersonCamera FPVCamera;
	Timer UpdateTimer;
	UpdateTimer.Start();

	std::string MemoryStats1 = {};
	std::string MemoryStats2 = {};

	while ( !DoQuit )
	{
		//update		
		glfwPollEvents();
		DoQuit = glfwWindowShouldClose( _pWindow );
			   

		while ( updateFrameIndex > _RenderThread.GetRenderFrame() )
		{
			_RenderThread.AccessRunCondition().notify_one();
			Sleep( 0 );
		}

		UpdateTimer.Stop();
		Frame::DeltaTime = UpdateTimer.GetTimeInSeconds();
		UpdateTimer.Start();
		

		StatsTimer += Frame::DeltaTime;

		if ( StatsTimer > 30.0f )
		{
			MemoryStats1 = "CPU Memory: " + Helpers::MemorySizeToString( GraphicsContext::GlobalAllocator.BytesAllocated() ); ;
			MemoryStats2 = "GPU Memory: " + Helpers::MemorySizeToString( GraphicsContext::DeviceAllocator->BytesAllocated() );
			StatsTimer = 0;
		}

		//std::cout << "End of frame " << app->updateFrameIndex << " update " << std::endl;

		if ( DoUpdate )
		{

			_InputEvent.Update( _pWindow, Frame::DeltaTime );

			//std::cout << "Start of frame " << app->updateFrameIndex << " update " << std::endl;

			//if (renderobject->standardMaterial != nullptr)
			{
				FPVCamera.Update();

				if ( autoRotate )
					rotation -= Frame::DeltaTime * 45.0f;

				if ( rotation < 0 )
					rotation = 360.0f;

				for ( std::shared_ptr<Model>& pModel : models )
				{
					pModel->Update();

					auto RenderCallback = [&]( CommandBuffer* pBuffer ) {
						pModel->Render( pBuffer );
					};

					_RenderThread.QueueRender( RenderCallback );

					Camera::Buffer& camera = pModel->AccessUBO();

					camera.model = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, translationY, 0.0f ) );
					camera.model = glm::rotate( camera.model, glm::radians( rotation ), Camera::Up );
					camera.model = glm::scale( camera.model, glm::vec3( scale, scale, scale ) );

					//camera.view = glm::lookAt( glm::vec3( 2, 1, 2 ), glm::vec3( 0.0f, 0, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
					camera.view = FPVCamera.Data().view;

					camera.proj = FPVCamera.Data().proj;
					camera.proj[ 1 ][ 1 ] *= -1;
				}
			}

			_pImguiVulkan->NewFrame( Frame::DeltaTime );

			if ( _pImguiVulkan->IsReady() )
			{
				ImGui::Begin( "Model" );

				ImGui::Checkbox( "Auto Rotate", &autoRotate );
				ImGui::DragFloat( "Object Rotation", &rotation, 0.1f );
				ImGui::DragFloat( "Object Translation Y", &translationY, 0.01f );
				ImGui::DragFloat( "Scale", &scale, 0.0001f );

				ImGui::End();

				DebugUI UI;
				UI.ListGameObjects();
				UI.TimingGraph( Frame::DeltaTime, _RenderThread._TotalTimer.GetTimeInSeconds() );

				ImGui::Begin( "MemoryStats" );
				
				ImGui::TextWrapped( MemoryStats1.c_str() );
				ImGui::TextWrapped( MemoryStats2.c_str() );

				ImGui::End();

				auto RenderCallback = [&]( CommandBuffer* pBuffer ) {
					_pImguiVulkan->Render( pBuffer );
				};

				_RenderThread.QueueRender( RenderCallback );
			}

			++updateFrameIndex;

			_RenderThread.AccessRunCondition().notify_one();
		}

#if !MULTITHREADED_RENDERING
		_RenderThread.DoFrame();
#endif
		std::string windowTitle = std::string( "delta time: " ) + Helpers::ValueToString( Frame::DeltaTime * 1000.0f );

		glfwSetWindowTitle( _pWindow, windowTitle.c_str() );
	}

	_Run = false;

	//updateThread.join();                // pauses until first finishes
#if MULTITHREADED_RENDERING
	_RenderThread.Stop();
#endif

	vkDeviceWaitIdle( GraphicsContext::LogicalDevice );
}

void RavenApp::WindowResizedCallback( GLFWwindow* window, int width, int height )
{
	if ( width == 0 || height == 0 )
		return;

	std::cout << "WindowResizedCallback:  " << width << "x" << height << std::endl;

	GraphicsContext::WindowSize = glm::ivec2( width, height );

	GraphicsDevice::Instance().SwapchainInvalidated();

	for ( size_t i = 0; i < OnWindowResized.size(); i++ )
	{
		OnWindowResized[ i ]( width, height );
	}

	RavenApp* app = reinterpret_cast<RavenApp*>( glfwGetWindowUserPointer( window ) );

	for ( std::shared_ptr<Model> pModel : app->models )
	{
		pModel->WindowResized( width, height );
	}
}
