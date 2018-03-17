#include "RavenApp.h"

#include <vector>
#include <iostream>
#include <thread>

#include "helpers/DebugAssert.h"
#include "helpers/Timer.h"
#include "helpers/Helpers.h"
#include "graphics/GraphicsDevice.h"
#include "graphics/VulkanInstance.h"
#include "graphics\VulkanSwapChain.h"
#include "graphics\PipelineStateObject.h"
#include "graphics\RenderObject.h"
#include "graphics\shaders\UniformBuffer.h"

#include "graphics/memory/GPUAllocator.h"
#include "graphics/models/Mesh.h"

RavenApp::RavenApp() :
	window(nullptr)
{

}

RavenApp::~RavenApp()
{
	GraphicsDevice::Instance().Finalize();

	glfwDestroyWindow(window);

	glfwTerminate();
}

void error_callback(int error, const char* description)
{
	puts(description);
}

bool RavenApp::Initialize()
{
	{
		if (!glfwInit()) //Initialize GLFW
		{
			std::cout << "GLFW not initialized." << std::endl;
			return false;
		}
		
		glfwSetErrorCallback(error_callback);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(1280, 720, "Vulkan window", nullptr, nullptr);
	}

	if (glfwVulkanSupported() == GLFW_TRUE)
	{
		std::cout << "Vulkan is supported." << std::endl;
	}
	else 
	{
		std::cout << "Vulkan is NOT supported. Falling back to DirectX11" << std::endl;
	}

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	std::vector<std::string> windowExtensionsNeeded(glfwExtensions, glfwExtensions + glfwExtensionCount);

	//Note: Ownership given to GraphicsDevice
	GraphicsContext::VulkanInstance = std::make_shared<VulkanInstance>();

	std::cout << glfwExtensionCount << " extensions available." << std::endl;
	for (size_t i = 0; i < windowExtensionsNeeded.size(); i++)
	{
		if (!GraphicsContext::VulkanInstance->IsExtensionAvailable(windowExtensionsNeeded[i]))
		{
			std::cout << "Missing " << windowExtensionsNeeded[i] << " extension." << std::endl;
			return false;
		}

		std::cout << windowExtensionsNeeded[i] << " extension is available." << std::endl;
	}


	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, RavenApp::OnWindowResized);

	GraphicsContext::VulkanInstance->CreateInstance(windowExtensionsNeeded);
	GraphicsContext::VulkanInstance->HookDebugCallback();


	//Note: Ownership given to GraphicsDevice
	std::shared_ptr<VulkanSwapChain> vulkanSwapChain = std::make_shared<VulkanSwapChain>();

	GraphicsContext::GlobalAllocator.PrintStats();
	VkResult result = glfwCreateWindowSurface(GraphicsContext::VulkanInstance->GetNative(), window, vulkanSwapChain->GetSurface().AllocationCallbacks(), vulkanSwapChain->GetSurface().Replace());

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	GraphicsDevice::Instance().Initialize(glm::u32vec2(1280, 720), vulkanSwapChain);

	GraphicsContext::GlobalAllocator.PrintStats();
	
	return true;
}


void RavenApp::UpdateThread(RavenApp* app)
{
	Timer timer;
	//CameraUBO ubo = {};
	

	while (app->run)
	{
		timer.Start();

		if (app->renderobject.basicMaterial != nullptr)
		{			
			CameraUBO* ubo = app->renderobject.basicMaterial->GetUniformBuffers()[0]->Get<CameraUBO>();

			app->objectMutex.lock();

				static auto startTime = std::chrono::high_resolution_clock::now();

				auto currentTime = std::chrono::high_resolution_clock::now();
				float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

				ubo->model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				ubo->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				ubo->proj = glm::perspective(glm::radians(45.0f), GraphicsContext::WindowSize.x / (float)GraphicsContext::WindowSize.y, 0.1f, 10.0f);
				ubo->proj[1][1] *= -1;

			app->objectMutex.unlock();
		}

		timer.Stop();
		Sleep(16);
	}
	
}

void RavenApp::RenderThread(RavenApp* app)
{
	VulkanSemaphore renderSemaphore;

	GraphicsContext::GlobalAllocator.PrintStats();

	Mesh quad;

	app->renderobject.Load(quad);


	GraphicsContext::GlobalAllocator.PrintStats();

	VkFence renderFence;
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = VK_NULL_HANDLE;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	vkCreateFence(GraphicsContext::LogicalDevice, &fenceInfo, GraphicsContext::GlobalAllocator.Get(), &renderFence);

	Timer timer;
	Timer acquireTimer;
	Timer renderQueuTimer;
	Timer presentTimer;

	float accumelatedTime = 0;

	while (app->run)
	{
		timer.Start();

		GraphicsDevice::Instance().Lock();

		//draw
		{
			acquireTimer.Start();
			//Prepare
			uint32_t imageIndex = GraphicsContext::SwapChain->PrepareBackBuffer();
			const VulkanSemaphore& backBufferSemaphore = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex).semaphore;
			Sleep(4);
			acquireTimer.Stop();			

			renderQueuTimer.Start();

			{
				app->objectMutex.lock();
				app->renderobject.PrepareDraw(imageIndex, quad);//TODO: let it return a command buffer to add to a list.
				app->objectMutex.unlock();
				//ro.PrepareDraw(imageIndex);
			}

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { backBufferSemaphore.GetNative() }; //Wait until this semaphore is signaled to continue with executing the command buffers
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &app->renderobject.commandBuffers[imageIndex]->GetNative(); //TODO: make use of local vector of commandBuffers

			VkSemaphore signalSemaphores[] = { renderSemaphore.GetNative() }; //This semaphore will be signaled when done with rendering the queue
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
			vkResetFences(GraphicsContext::LogicalDevice, 1, &renderFence);
			//Draw (wait untill surface is available)
			if (vkQueueSubmit(GraphicsContext::GraphicsQueue, 1, &submitInfo, renderFence) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to submit draw command buffer!");
			}
			vkWaitForFences(GraphicsContext::LogicalDevice, 1, &renderFence, true, 1000000);
			//Sleep(4);
			renderQueuTimer.Stop();

			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { GraphicsContext::SwapChain->GetNative() };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			presentTimer.Start();
			vkQueueWaitIdle(GraphicsContext::PresentQueue);
			//Present (wait until drawing is done)
			vkQueuePresentKHR(GraphicsContext::PresentQueue, &presentInfo);
			presentTimer.Stop();
		}
		GraphicsDevice::Instance().Unlock();

		Sleep(16);
		timer.Stop();
		accumelatedTime += timer.GetTimeInSeconds();

		if (accumelatedTime > 1.0f)
		{
			std::cout << "Acquire: " << acquireTimer.GetTimeInSeconds() << " Render: " << renderQueuTimer.GetTimeInSeconds() << " Present: " << presentTimer.GetTimeInSeconds() << std::endl;
			accumelatedTime -= 1.0f;
		}

	}

	vkDestroyFence(GraphicsContext::LogicalDevice, renderFence, GraphicsContext::GlobalAllocator.Get());

}

void RavenApp::Run()
{

	run = true;

	std::thread updateThread(RavenApp::UpdateThread, this);
	std::thread renderThread(RavenApp::RenderThread, this);


	Timer timer;
	while (!glfwWindowShouldClose(window))
	{
		timer.Start();

		//update
		{
			glfwPollEvents();
		}

		Sleep(1);

		timer.Stop();

		std::string windowTitle = std::string("delta time: ") + Helpers::ValueToString(timer.GetTimeInSeconds()*1000.0f);

		glfwSetWindowTitle(window, windowTitle.c_str());
	}

	run = false;

	updateThread.join();                // pauses until first finishes
	renderThread.join();               // pauses until second finishes

	vkDeviceWaitIdle(GraphicsContext::LogicalDevice);
}