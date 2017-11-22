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

#include "graphics/memory/GPUAllocator.h"

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

bool RavenApp::Initialize()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(1280, 720, "Vulkan window", nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, RavenApp::OnWindowResized);
	

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<std::string> allExtensionsRequired;

	std::vector<std::string> windowExtensionsNeeded(glfwExtensions, glfwExtensions + glfwExtensionCount);

	//Note: Ownership given to GraphicsDevice
	GraphicsContext::VulkanInstance = std::make_shared<VulkanInstance>();

	for (size_t i = 0; i < allExtensionsRequired.size(); i++)
	{
		if (!GraphicsContext::VulkanInstance->IsExtensionAvailable(allExtensionsRequired[i]))
		{
			std::cout << "Missing " << allExtensionsRequired[i] << " extension." << std::endl;
			return false;
		}
	}

	GraphicsContext::VulkanInstance->CreateInstance(windowExtensionsNeeded);
	GraphicsContext::VulkanInstance->HookDebugCallback();


	//Note: Ownership given to GraphicsDevice
	std::shared_ptr<VulkanSwapChain> vulkanSwapChain = std::make_shared<VulkanSwapChain>();

	GraphicsContext::GlobalAllocator.PrintStats();

	if (glfwCreateWindowSurface(GraphicsContext::VulkanInstance->GetNative(), window, vulkanSwapChain->GetSurface().AllocationCallbacks(), vulkanSwapChain->GetSurface().Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	GraphicsDevice::Instance().Initialize(glm::u32vec2(1280, 720), vulkanSwapChain);

	GraphicsContext::GlobalAllocator.PrintStats();

	GPUAllocator gpuAllocator(1024, 8);

	return true;
}


void RavenApp::UpdateThread(const RavenApp* app)
{
	Timer timer;

	while (app->run)
	{
		timer.Start();

		timer.Stop();
		Sleep(16);
	}
	
}

void RavenApp::RenderThread(const RavenApp* app)
{
	VulkanSemaphore renderSemaphore;

	GraphicsContext::GlobalAllocator.PrintStats();

	RenderObject ro;
	ro.Load();

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
			ro.PrepareDraw(imageIndex);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { backBufferSemaphore.GetNative() }; //Wait until this semaphore is signaled to continue with executing the command buffers
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &ro.commandBuffers[imageIndex]->GetNative();

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

		timer.Stop();

		std::cout << "Acquire: " << acquireTimer.GetTimeInSeconds() << " Render: " << renderQueuTimer.GetTimeInSeconds() << " Present: " << presentTimer.GetTimeInSeconds() << std::endl;

		Sleep(4);
	}

}

void RavenApp::Run()
{

	run = true;

	std::thread first(RavenApp::UpdateThread, this);
	std::thread second(RavenApp::RenderThread, this);


	Timer timer;
	while (!glfwWindowShouldClose(window))
	{
		timer.Start();

		//update
		{
			glfwPollEvents();
		}


		timer.Stop();

		std::string windowTitle = std::string("delta time: ") + Helpers::ValueToString(timer.GetTimeInSeconds()*1000.0f);

		glfwSetWindowTitle(window, windowTitle.c_str());
	}

	run = false;

	first.join();                // pauses until first finishes
	second.join();               // pauses until second finishes

	vkDeviceWaitIdle(GraphicsContext::LogicalDevice);
}