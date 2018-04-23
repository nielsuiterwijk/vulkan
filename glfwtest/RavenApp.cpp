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
#include "graphics\buffers\UniformBuffer.h"

#include "graphics/memory/GPUAllocator.h"
#include "graphics/models/Mesh.h"
#include "graphics/textures/TextureLoader.h"

RavenApp::RavenApp() :
	window(nullptr)
{

}

RavenApp::~RavenApp()
{
	delete renderobject;

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

	std::cout << glfwExtensionCount << " extensions needed:" << std::endl;
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

	renderobject = new RenderObject();


	
	return true;
}


void RavenApp::UpdateThread(RavenApp* app)
{
	//CameraUBO ubo = {};

	auto previousTime = std::chrono::high_resolution_clock::now();
	
	float statsTimer = 0.0f;

	while (app->run)
	{
		float delta = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - previousTime).count();
		previousTime = std::chrono::high_resolution_clock::now();

		if (app->renderobject->standardMaterial != nullptr)
		{
			static auto startTime = std::chrono::high_resolution_clock::now();

			app->objectMutex.lock();

				CameraUBO* ubo = app->renderobject->standardMaterial->GetUniformBuffers()[0]->Get<CameraUBO>();

				auto currentTime = std::chrono::high_resolution_clock::now();
				float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

				ubo->model = glm::rotate(glm::mat4(1.0f), time * glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				ubo->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				ubo->proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
				ubo->proj[1][1] *= -1;

			app->objectMutex.unlock();
		}

		statsTimer += delta;

		if (statsTimer > 30.0f)
		{
			GraphicsContext::GlobalAllocator.PrintStats();
			statsTimer = 0;
		}


		Sleep(16);
	}
	
}

void RavenApp::RenderThread(RavenApp* app)
{
	VulkanSemaphore renderSemaphore;

	GraphicsContext::GlobalAllocator.PrintStats();

	Mesh quad;
	Mesh chalet("models/chalet.obj");
	app->renderobject->Load(chalet);


	GraphicsContext::GlobalAllocator.PrintStats();

	VkFence renderFence;
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = VK_NULL_HANDLE;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	RenderObject clear;


	vkCreateFence(GraphicsContext::LogicalDevice, &fenceInfo, GraphicsContext::GlobalAllocator.Get(), &renderFence);

	Timer timer;
	Timer acquireTimer;
	Timer drawCallTimer;
	Timer renderQueuTimer;
	Timer presentTimer;

	float accumelatedTime = 0; 
	
	VkResult result;

	while (app->run)
	{
		timer.Start();

		GraphicsDevice::Instance().Lock();

		

		//draw
		{
			acquireTimer.Start();
			//Prepare
			uint32_t imageIndex = GraphicsContext::SwapChain->PrepareBackBuffer();
			const InstanceWrapper<VkSemaphore>& backBufferSemaphore = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex).GetLock();

			VkSemaphore signalSemaphores[] = { renderSemaphore.GetNative() }; //This semaphore will be signaled when done with rendering the queue

			//Sleep(4);
			acquireTimer.Stop();			
			drawCallTimer.Start();

			std::vector<VkCommandBuffer> commandBuffersToDraw;
			{
				commandBuffersToDraw.push_back(clear.ClearBackbuffer(imageIndex)->GetNative());

				app->objectMutex.lock();
				//TODO: let it return a command buffer to add to a list.
				//TODO: Make the prepare threadsafe by doing a copy?
				//TODO: dont pass the mesh, it should be owned / held by the renderObject
				std::shared_ptr<CommandBuffer> commandBuffer = app->renderobject->PrepareDraw(imageIndex, chalet);

				if (commandBuffer != nullptr)
				{
					commandBuffersToDraw.push_back(commandBuffer->GetNative());
				}
				
				app->objectMutex.unlock();
				//ro.PrepareDraw(imageIndex);
			}
			drawCallTimer.Stop();

			renderQueuTimer.Start();

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { backBufferSemaphore }; //Wait until this semaphore is signaled to continue with executing the command buffers
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffersToDraw.size());
			submitInfo.pCommandBuffers = commandBuffersToDraw.data(); 

			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
			vkResetFences(GraphicsContext::LogicalDevice, 1, &renderFence);
			GraphicsContext::TransportQueueLock.lock();
			//Draw (wait untill surface is available)
			result = vkQueueSubmit(GraphicsContext::GraphicsQueue, 1, &submitInfo, renderFence);
			if (result != VK_SUCCESS)
			{
				std::cout << "vkQueueSubmit error: " << Vulkan::GetVkResultAsString(result) << std::endl;
				//throw std::runtime_error("failed to submit draw command buffer!");
			}
			GraphicsContext::TransportQueueLock.unlock();

			result = vkWaitForFences(GraphicsContext::LogicalDevice, 1, &renderFence, true, 5000000000);
			if (result != VK_SUCCESS)
			{
				std::cout << "vkWaitForFences error: " << Vulkan::GetVkResultAsString(result) << std::endl;
			}

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
			result = vkQueuePresentKHR(GraphicsContext::PresentQueue, &presentInfo);
			if (result != VK_SUCCESS)
			{
				std::cout << "vkQueuePresentKHR error: " << Vulkan::GetVkResultAsString(result) << std::endl;
			}
			presentTimer.Stop();
		}
		GraphicsDevice::Instance().Unlock();

		Sleep(16);
		timer.Stop();
		accumelatedTime += timer.GetTimeInSeconds();

		if (accumelatedTime > 2.0f)
		{
			std::cout << "a: " << acquireTimer.GetTimeInSeconds() << " d: " << drawCallTimer.GetTimeInSeconds() << " q: " << renderQueuTimer.GetTimeInSeconds() << " p: " << presentTimer.GetTimeInSeconds() << std::endl;
			accumelatedTime -= 2.0f;
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