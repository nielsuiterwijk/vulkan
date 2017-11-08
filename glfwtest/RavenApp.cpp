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
	std::shared_ptr<VulkanInstance> vulkanInstance = std::make_shared<VulkanInstance>();

	for (size_t i = 0; i < allExtensionsRequired.size(); i++)
	{
		if (!vulkanInstance->IsExtensionAvailable(allExtensionsRequired[i]))
		{
			std::cout << "Missing " << allExtensionsRequired[i] << " extension." << std::endl;
			return false;
		}
	}

	vulkanInstance->CreateInstance(windowExtensionsNeeded);
	vulkanInstance->HookDebugCallback();


	//Note: Ownership given to GraphicsDevice
	std::shared_ptr<VulkanSwapChain> vulkanSwapChain = std::make_shared<VulkanSwapChain>();

	GraphicsContext::GlobalAllocator.PrintStats();

	if (glfwCreateWindowSurface(vulkanInstance->Get(), window, vulkanSwapChain->GetSurface().AllocationCallbacks(), vulkanSwapChain->GetSurface().Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	GraphicsDevice::Instance().Initialize(glm::u32vec2(1280, 720), vulkanInstance, vulkanSwapChain);

	GraphicsContext::GlobalAllocator.PrintStats();

	return true;
}


void RavenApp::UpdateThread(RavenApp& app)
{
	Timer timer;

	while (app.run)
	{
		timer.Start();

		timer.Stop();
	}
	
}

void RavenApp::RenderThread(RavenApp& app)
{
	std::shared_ptr<Material> fixedMaterial = GraphicsDevice::Instance().CreateMaterial("fixed");
	PipelineStateObject pso(fixedMaterial);

	VulkanSemaphore renderSemaphore;

	GraphicsContext::GlobalAllocator.PrintStats();

	std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;
	GraphicsContext::CommandBufferPool->Create(commandBuffers, 3);

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		commandBuffers[i]->StartRecording(i);

		vkCmdBindPipeline(commandBuffers[i]->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipeLine());
		vkCmdDraw(commandBuffers[i]->GetNative(), 3, 1, 0, 0);

		commandBuffers[i]->StopRecording();
	}

	Timer timer;

	while (app.run)
	{
		timer.Start();

		if (!GraphicsDevice::Instance().IsAvailable())
		{
			Sleep(1);
			continue;
		}

		//draw
		{
			//Prepare
			uint32_t imageIndex = GraphicsContext::SwapChain->PrepareBackBuffer();
			const VulkanSemaphore& backBufferSemaphore = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex).semaphore;

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { backBufferSemaphore.GetNative() }; //Wait untill this semaphore is signalled to continue with the corresponding stage below
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffers[imageIndex]->GetNative();

			VkSemaphore signalSemaphores[] = { renderSemaphore.GetNative() }; //This semaphore will be signalled when done with rendering the queue
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			//Draw (wait untill surface is available)
			if (vkQueueSubmit(GraphicsContext::GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { GraphicsContext::SwapChain->GetNative() };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			vkQueueWaitIdle(GraphicsContext::PresentQueue);
			//Present (wait untill drawing is done)
			vkQueuePresentKHR(GraphicsContext::PresentQueue, &presentInfo);
		}

		timer.Stop();
	}

}

void RavenApp::Run()
{		

	run = true;

	std::thread first(RavenApp::UpdateThread, *this);
	std::thread second(RavenApp::RenderThread, *this);
	

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