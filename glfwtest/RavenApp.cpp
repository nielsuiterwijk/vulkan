#include "RavenApp.h"


#include <vector>
#include <iostream>

#include "helpers/DebugAssert.h"
#include "helpers/Timer.h"
#include "helpers/Helpers.h"
#include "graphics/GraphicsDevice.h"
#include "graphics/VulkanInstance.h"
#include "graphics\VulkanSwapChain.h"
#include "graphics\PipelineStateObject.h"

RavenApp::RavenApp() :
	window(nullptr),
	device(nullptr)
{

}

RavenApp::~RavenApp()
{
	device = nullptr;

	glfwDestroyWindow(window);

	glfwTerminate();
}

bool RavenApp::Initialize()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(1280, 720, "Vulkan window", nullptr, nullptr);

	device = std::make_shared<GraphicsDevice>(glm::u32vec2(1280, 720));


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
	std::shared_ptr<VulkanSwapChain> vulkanSwapChain = std::make_shared<VulkanSwapChain>(vulkanInstance->Get());

	GraphicsContext::GlobalAllocator.PrintStats();

	if (glfwCreateWindowSurface(vulkanInstance->Get(), window, vulkanSwapChain->GetSurface().AllocationCallbacks(), vulkanSwapChain->GetSurface().Replace()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	device->Initialize(vulkanInstance, vulkanSwapChain);
	vulkanSwapChain = nullptr;

	GraphicsContext::GlobalAllocator.PrintStats();

	return true;
}

void RavenApp::Run()
{
	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;

	Timer timer;
		
	std::shared_ptr<Material> fixedMaterial = device->CreateMaterial("fixed");

	PipelineStateObject pso(fixedMaterial);

	GraphicsContext::GlobalAllocator.PrintStats();

	std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;
	GraphicsContext::CommandBufferPool->Create(commandBuffers, 3);

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		commandBuffers[i]->StartRecording(0);

		vkCmdBindPipeline(commandBuffers[i]->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipeLine());
		vkCmdDraw(commandBuffers[i]->GetNative(), 3, 1, 0, 0);

		commandBuffers[i]->StopRecording();
	}
	
	VulkanSemaphore renderSemaphore;

	while (!glfwWindowShouldClose(window))
	{
		timer.Start();

		//update
		{
			glfwPollEvents();
		}

		//draw
		{
			//Prepare
			uint32_t imageIndex = GraphicsContext::SwapChain->PrepareBackBuffer();
			VkSemaphore backBufferSemaphore = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex).semaphore.GetNative();

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { backBufferSemaphore }; //Wait untill this semaphore is signalled to continue with the corresponding stage below
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
		
		Sleep(500);

		timer.Stop();

		std::string windowTitle = std::string("delta time: ") + Helpers::ValueToString(timer.GetTimeInSeconds()*1000.0f);

		glfwSetWindowTitle(window, windowTitle.c_str());
	}

	vkDeviceWaitIdle(GraphicsContext::LogicalDevice);
	
	Sleep(2000);
}