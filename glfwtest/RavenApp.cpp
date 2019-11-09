#include "RavenApp.h"

#include <iostream>
#include <thread>
#include <vector>

#include "graphics/GraphicsDevice.h"
#include "graphics/VulkanInstance.h"
#include "graphics\PipelineStateObject.h"
#include "graphics\VulkanSwapChain.h"
#include "graphics\buffers\UniformBuffer.h"
#include "helpers/DebugAssert.h"
#include "helpers/Helpers.h"

#include "graphics/memory/GPUAllocator.h"
#include "graphics/models/Mesh.h"
#include "graphics/textures/TextureLoader.h"

#include "imgui/imgui.h"

#include "raven/DebugUI.h"
#include "raven/model.h"

std::vector<std::function<void( int, int, int )>> RavenApp::OnMouseButton = {};
std::vector<std::function<void( double, double )>> RavenApp::OnMouseScroll = {};
std::vector<std::function<void( int, int, int, int )>> RavenApp::OnKey = {};
std::vector<std::function<void( unsigned int )>> RavenApp::OnChar = {};
std::vector<std::function<void( int, int )>> RavenApp::OnWindowResized = {};

RavenApp::RavenApp()
	: window( nullptr )
	, imguiVulkan( nullptr )
	, updateFrameIndex( 0 )
	, _RenderThread( )
{
}

RavenApp::~RavenApp()
{
	models.clear();

	delete imguiVulkan;

	_RenderThread.Destroy();

	GraphicsDevice::Instance().Finalize();

	glfwDestroyWindow( window );

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

		window = glfwCreateWindow( 1280, 720, "Vulkan window", nullptr, nullptr );
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

	glfwSetWindowUserPointer( window, this );

	glfwSetWindowSizeCallback( window, RavenApp::WindowResizedCallback );
	glfwSetMouseButtonCallback( window, RavenApp::MouseButtonCallback );
	glfwSetScrollCallback( window, RavenApp::ScrollCallback );
	glfwSetKeyCallback( window, RavenApp::KeyCallback );
	glfwSetCharCallback( window, RavenApp::CharCallback );

	GraphicsContext::VulkanInstance->CreateInstance( windowExtensionsNeeded );
	GraphicsContext::VulkanInstance->HookDebugCallback();

	//Note: Ownership given to GraphicsDevice
	std::shared_ptr<VulkanSwapChain> vulkanSwapChain = std::make_shared<VulkanSwapChain>();

	GraphicsContext::GlobalAllocator.PrintStats();
	VkResult result = glfwCreateWindowSurface( GraphicsContext::VulkanInstance->GetNative(),
											   window,
											   vulkanSwapChain->GetSurface().AllocationCallbacks(),
											   vulkanSwapChain->GetSurface().Replace() );
	assert( result == VK_SUCCESS );

	GraphicsDevice::Instance().Initialize( glm::u32vec2( 1280, 720 ), vulkanSwapChain );

	GraphicsContext::GlobalAllocator.PrintStats();


	_RenderThread.Initialize();


	imguiVulkan = new IMGUIVulkan();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext( nullptr );
	imguiVulkan->Init( window, true );


	std::shared_ptr<Model> model = std::make_shared<Model>( "cesiumman" );
	models.push_back( model );

	return true;
}

void RavenApp::Run()
{
	run = true;

#if MULTITHREADED_RENDERING
	GraphicsContext::GlobalAllocator.PrintStats();
	_RenderThread.Start();
#endif

	static float rotation = 310;
	float translationY = -1;
	//float scale = 0.025;
	float scale = 1;

	while ( !glfwWindowShouldClose( window ) )
	{
		//update
		glfwPollEvents();

		static auto previousTime = std::chrono::high_resolution_clock::now();

		float statsTimer = 0.0f;

		bool DoUpdate = true;

		{
			//std::unique_lock<std::mutex> lock( _RenderThread.AccessNotificationMutex() );

			//The condition will take the lock and will wait for to be notified and will continue
			//only if were stopping (stop == true) or if there are tasks to do, else it will keep waiting.
			//_UpdateThreadWait.wait(lock);
			//ASSERT( updateFrameIndex == renderFrameIndex );
			while ( updateFrameIndex > _RenderThread.GetRenderFrame() )
			{
				_RenderThread.AccessRunCondition().notify_one();
				Sleep( 0 );
			}
		}

		float delta = std::chrono::duration<float, std::chrono::seconds::period>( std::chrono::high_resolution_clock::now() - previousTime ).count();

		if ( DoUpdate )
		{
			previousTime = std::chrono::high_resolution_clock::now();
			//std::cout << "Start of frame " << app->updateFrameIndex << " update " << std::endl;

			//if (renderobject->standardMaterial != nullptr)
			{
				static auto startTime = std::chrono::high_resolution_clock::now();

				auto currentTime = std::chrono::high_resolution_clock::now();
				float time = std::chrono::duration<float, std::chrono::seconds::period>( currentTime - startTime ).count();

				rotation -= delta * 45.0f;

				if ( rotation < 0 )
					rotation = 360.0f;

				for ( std::shared_ptr<Model>& pModel : models )
				{
					pModel->Update( delta );

					CameraUBO& camera = pModel->AccessUBO();

					camera.model = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, translationY, 0.0f ) );
					camera.model = glm::rotate( camera.model, glm::radians( rotation ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
					camera.model = glm::scale( camera.model, glm::vec3( scale, scale, scale ) );

					camera.view = glm::lookAt( glm::vec3( 2, 1, 2 ), glm::vec3( 0.0f, 0, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

					camera.proj = glm::perspective( glm::radians( 45.0f ), 16.0f / 9.0f, 0.01f, 100.0f );
					camera.proj[ 1 ][ 1 ] *= -1;
				}
			}

			imguiVulkan->NewFrame( delta );

			if ( imguiVulkan->IsReady() )
			{
				ImGui::Begin( "Model" );

				ImGui::DragFloat( "Object Rotation", &rotation, 0.1f );
				ImGui::DragFloat( "Object Translation Y", &translationY, 0.01f );
				ImGui::DragFloat( "Scale", &scale, 0.0001f );

				ImGui::End();

				DebugUI UI;
				UI.ListGameObjects();
			}

			statsTimer += delta;

			if ( statsTimer > 30.0f )
			{
				GraphicsContext::GlobalAllocator.PrintStats();
				statsTimer = 0;
			}

			//std::cout << "End of frame " << app->updateFrameIndex << " update " << std::endl;

			++updateFrameIndex;

			_RenderThread.AccessRunCondition().notify_one();
		}

#if !MULTITHREADED_RENDERING
		_RenderThread.DoFrame();
#endif

		Sleep( 1 );

		std::string windowTitle = std::string( "delta time: " ) + Helpers::ValueToString( delta * 1000.0f );

		glfwSetWindowTitle( window, windowTitle.c_str() );
	}

	run = false;

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

void RavenApp::MouseButtonCallback( GLFWwindow* window, int button, int action, int mods )
{
	for ( size_t i = 0; i < OnMouseButton.size(); i++ )
	{
		OnMouseButton[ i ]( button, action, mods );
	}
}

void RavenApp::ScrollCallback( GLFWwindow* window, double xoffset, double yoffset )
{
	for ( size_t i = 0; i < OnMouseScroll.size(); i++ )
	{
		OnMouseScroll[ i ]( xoffset, yoffset );
	}
}

void RavenApp::KeyCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	for ( size_t i = 0; i < OnKey.size(); i++ )
	{
		OnKey[ i ]( key, scancode, action, mods );
	}
}

void RavenApp::CharCallback( GLFWwindow* window, unsigned int c )
{
	for ( size_t i = 0; i < OnChar.size(); i++ )
	{
		OnChar[ i ]( c );
	}
}
