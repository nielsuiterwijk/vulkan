#include "RavenApp.h"

#include <vector>
#include <iostream>
#include <thread>

#include "helpers/DebugAssert.h"
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

#include "imgui/imgui.h"


std::vector<std::function<void(int, int, int)>> RavenApp::OnMouseButton = {};
std::vector<std::function<void(double, double)>> RavenApp::OnMouseScroll = {};
std::vector<std::function<void(int, int, int, int)>> RavenApp::OnKey = {};
std::vector<std::function<void(unsigned int)>> RavenApp::OnChar = {};


std::mutex RavenApp::queue_mutex;
std::condition_variable RavenApp::renderThreadWait;
std::condition_variable RavenApp::updateThreadWait;

RavenApp::RavenApp() :
	window(nullptr),
	imguiVulkan(nullptr),
	updateFrameIndex(0),
	renderFrameIndex(0),
	accumelatedTime(0)
{

}

RavenApp::~RavenApp()
{
	delete renderobject;
	delete clear;
	delete renderSemaphore;
	delete imguiVulkan;

	vkDestroyFence(GraphicsContext::LogicalDevice, renderFence, GraphicsContext::GlobalAllocator.Get());

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
	glfwSetMouseButtonCallback(window, RavenApp::MouseButtonCallback);
	glfwSetScrollCallback(window, RavenApp::ScrollCallback);
	glfwSetKeyCallback(window, RavenApp::KeyCallback);
	glfwSetCharCallback(window, RavenApp::CharCallback);

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

	imguiVulkan = new IMGUIVulkan();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext(nullptr);	
	imguiVulkan->Init(window, true);
	
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = VK_NULL_HANDLE;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	vkCreateFence(GraphicsContext::LogicalDevice, &fenceInfo, GraphicsContext::GlobalAllocator.Get(), &renderFence);

	renderobject = new RenderObject();

	//chalet = new Mesh("models/chalet.obj");
	//renderobject->Load(*chalet);

	clear = new RenderObject();
	renderSemaphore = new VulkanSemaphore();
	
	return true;
}

#if MULTITHREADED_RENDERING
void RavenApp::RenderThread(RavenApp* app)
{
	GraphicsContext::GlobalAllocator.PrintStats();
	while (app->run)
	{
		Render(app);
	}
}
#endif

void RavenApp::Render(RavenApp* app)
{
	VkResult result;
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		// Wait until update thread is done with the next frame
		RavenApp::renderThreadWait.wait(lock, [=] { return app->updateFrameIndex > app->renderFrameIndex; });
	}

	if (!app->run)
		return;

	//std::cout << "Start of frame " << app->renderFrameIndex << " render " << std::endl;

	app->timer.Start();

	GraphicsDevice::Instance().Lock();



	//draw
	{
		app->acquireTimer.Start();

		result = vkWaitForFences(GraphicsContext::LogicalDevice, 1, &app->renderFence, true, 5000000000);
		if (result != VK_SUCCESS)
		{
			std::cout << "vkWaitForFences error: " << Vulkan::GetVkResultAsString(result) << std::endl;
		}

		//Prepare
		uint32_t imageIndex = -1;
		do
		{
			imageIndex = GraphicsContext::SwapChain->PrepareBackBuffer();

			if (imageIndex == -1)
			{
				GraphicsDevice::Instance().Unlock();
				GraphicsDevice::Instance().SwapchainInvalidated();
				GraphicsDevice::Instance().Lock();
			}

		} while (imageIndex == -1);

		const InstanceWrapper<VkSemaphore>& backBufferSemaphore = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex).GetLock();

		VkSemaphore signalSemaphores[] = { app->renderSemaphore->GetNative() }; //This semaphore will be signaled when done with rendering the queue

																		  //Sleep(4);
		app->acquireTimer.Stop();
		app->drawCallTimer.Start();

		std::vector<VkCommandBuffer> commandBuffersToDraw;
		{
			commandBuffersToDraw.push_back(app->clear->ClearBackbuffer(imageIndex)->GetNative());

			app->objectMutex.lock();
			//TODO: Make the prepare threadsafe by doing a copy?
			//TODO: dont pass the mesh, it should be owned / held by the renderObject
			//TODO: find something more elegant then checking nullptr
			std::shared_ptr<CommandBuffer> commandBuffer = nullptr;
			//commandBuffer = app->renderobject->PrepareDraw(imageIndex, *(app->chalet));

			if (commandBuffer != nullptr)
			{
				commandBuffersToDraw.push_back(commandBuffer->GetNative());
			}

			commandBuffer = app->imguiVulkan->Render(imageIndex);

			if (commandBuffer != nullptr)
			{
				commandBuffersToDraw.push_back(commandBuffer->GetNative());
			}

			app->objectMutex.unlock();
			//ro.PrepareDraw(imageIndex);
		}
		app->drawCallTimer.Stop();

		{
			//As soon as we're done creating our command buffers, let the update thread know it can continue processing its next frame
			app->renderFrameIndex++;
			RavenApp::updateThreadWait.notify_one();
		}

		app->renderQueuTimer.Start();

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
		vkResetFences(GraphicsContext::LogicalDevice, 1, &app->renderFence);
		GraphicsContext::TransportQueueLock.lock();
		//Draw (wait until surface is available)
		result = vkQueueSubmit(GraphicsContext::GraphicsQueue, 1, &submitInfo, app->renderFence);
		if (result != VK_SUCCESS)
		{
			std::cout << "vkQueueSubmit error: " << Vulkan::GetVkResultAsString(result) << std::endl;
			//throw std::runtime_error("failed to submit draw command buffer!");
		}
		GraphicsContext::TransportQueueLock.unlock();


		//Sleep(4);
		app->renderQueuTimer.Stop();

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;


		VkSwapchainKHR swapChains[] = { GraphicsContext::SwapChain->GetNative() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		app->presentTimer.Start();
		GraphicsContext::TransportQueueLock.lock();
		vkQueueWaitIdle(GraphicsContext::PresentQueue);
		//Present (wait until drawing is done)
		result = vkQueuePresentKHR(GraphicsContext::PresentQueue, &presentInfo);
		GraphicsContext::TransportQueueLock.unlock();
		if (result != VK_SUCCESS)
		{
			std::cout << "vkQueuePresentKHR error: " << Vulkan::GetVkResultAsString(result) << std::endl;
		}
		app->presentTimer.Stop();
	}
	GraphicsDevice::Instance().Unlock();

	//Sleep(16);
	app->timer.Stop();
	app->accumelatedTime += app->timer.GetTimeInSeconds();

	{
		if (app->accumelatedTime > 2.0f)
		{
			std::cout << "a: " << app->acquireTimer.GetTimeInSeconds() << " d: " << app->drawCallTimer.GetTimeInSeconds() << " q: " << app->renderQueuTimer.GetTimeInSeconds() << " p: " << app->presentTimer.GetTimeInSeconds() << std::endl;
			app->accumelatedTime -= 2.0f;
		}
		//std::cout << "End of frame " << app->renderFrameIndex - 1 << " render " << std::endl;
	}
}

void RavenApp::Run()
{

	run = true;

#if MULTITHREADED_RENDERING
	std::thread renderThread(RavenApp::RenderThread, this);
#endif

	while (!glfwWindowShouldClose(window))
	{
		//update
		glfwPollEvents();

		auto previousTime = std::chrono::high_resolution_clock::now();

		float statsTimer = 0.0f;

		bool runUpdate = false;

		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			//The condition will take the lock and will wait for to be notified and will continue
			//only if were stopping (stop == true) or if there are tasks to do, else it will keep waiting.
			//RavenApp::updateThreadWait.wait(lock, [=] { return updateFrameIndex == renderFrameIndex; });
			runUpdate = updateFrameIndex == renderFrameIndex;
		}

		float delta = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - previousTime).count();
		previousTime = std::chrono::high_resolution_clock::now();

		if (runUpdate)
		{
			//std::cout << "Start of frame " << app->updateFrameIndex << " update " << std::endl;

			objectMutex.lock();

			if (renderobject->standardMaterial != nullptr)
			{
				static auto startTime = std::chrono::high_resolution_clock::now();


				CameraUBO* ubo = renderobject->standardMaterial->GetUniformBuffers()[0]->Get<CameraUBO>();

				auto currentTime = std::chrono::high_resolution_clock::now();
				float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

				ubo->model = glm::rotate(glm::mat4(1.0f), time * glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				ubo->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				ubo->proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
				ubo->proj[1][1] *= -1;


			}
			imguiVulkan->NewFrame(delta);

			objectMutex.unlock();

			statsTimer += delta;

			if (statsTimer > 30.0f)
			{
				GraphicsContext::GlobalAllocator.PrintStats();
				statsTimer = 0;
			}

			//std::cout << "End of frame " << app->updateFrameIndex << " update " << std::endl;

			updateFrameIndex++;
			RavenApp::renderThreadWait.notify_one();
		}


#if !MULTITHREADED_RENDERING
		Render(this);
#endif

		Sleep(1);
		
		std::string windowTitle = std::string("delta time: ") + Helpers::ValueToString(delta*1000.0f);

		glfwSetWindowTitle(window, windowTitle.c_str());
	}

	run = false;

	//updateThread.join();                // pauses until first finishes
#if MULTITHREADED_RENDERING
	renderThread.join();               // pauses until second finishes
#endif

	vkDeviceWaitIdle(GraphicsContext::LogicalDevice);
}	

void RavenApp::OnWindowResized(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0)
		return;

	GraphicsContext::WindowSize = glm::ivec2(width, height);

	GraphicsDevice::Instance().SwapchainInvalidated();

	RavenApp* app = reinterpret_cast<RavenApp*>(glfwGetWindowUserPointer(window));
}

void RavenApp::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	for (size_t i = 0; i < OnMouseButton.size(); i++)
	{
		OnMouseButton[i](button, action, mods);
	}
}

void RavenApp::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	for (size_t i = 0; i < OnMouseScroll.size(); i++)
	{
		OnMouseScroll[i](xoffset, yoffset);
	}
}

void RavenApp::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	for (size_t i = 0; i < OnKey.size(); i++)
	{
		OnKey[i](key, scancode, action, mods);
	}
}

void RavenApp::CharCallback(GLFWwindow* window, unsigned int c)
{
	for (size_t i = 0; i < OnChar.size(); i++)
	{
		OnChar[i](c);
	}
}