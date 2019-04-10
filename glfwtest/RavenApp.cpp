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
#include "graphics\buffers\UniformBuffer.h"

#include "graphics/memory/GPUAllocator.h"
#include "graphics/models/Mesh.h"
#include "graphics/textures/TextureLoader.h"

#include "imgui/imgui.h"

#include "raven/model.h"


std::vector<std::function<void(int, int, int)>> RavenApp::OnMouseButton = {};
std::vector<std::function<void(double, double)>> RavenApp::OnMouseScroll = {};
std::vector<std::function<void(int, int, int, int)>> RavenApp::OnKey = {};
std::vector<std::function<void(unsigned int)>> RavenApp::OnChar = {};
std::vector<std::function<void(int, int)>> RavenApp::OnWindowResized = {};


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
	delete model;
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

	glfwSetWindowSizeCallback(window, RavenApp::WindowResizedCallback);
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
	assert(result == VK_SUCCESS);

	GraphicsDevice::Instance().Initialize(glm::u32vec2(1280, 720), vulkanSwapChain);

	GraphicsContext::GlobalAllocator.PrintStats();
	
	GraphicsContext::CommandBufferPool->Create(commandBuffers, GraphicsContext::SwapChain->GetAmountOfFrameBuffers());

	imguiVulkan = new IMGUIVulkan();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext(nullptr);	
	imguiVulkan->Init(window, true);
	
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = VK_NULL_HANDLE;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	result = vkCreateFence(GraphicsContext::LogicalDevice, &fenceInfo, GraphicsContext::GlobalAllocator.Get(), &renderFence);
	assert(result == VK_SUCCESS);

	model = new Model("cesiumman");

	renderSemaphore = new VulkanSemaphore();

	
	return true;
}

#if MULTITHREADED_RENDERING
void RavenApp::RenderThread(RavenApp* app)
{
	GraphicsContext::GlobalAllocator.PrintStats();
	while (app->run)
	{
		if (GraphicsContext::LogicalDevice == nullptr)
			Sleep(50);

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
		RavenApp::renderThreadWait.wait(lock, [=] { return app->updateFrameIndex > app->renderFrameIndex || !app->run; });
	}

	if (!app->run)
		return;

	//std::cout << "Start of frame " << app->renderFrameIndex << " render " << std::endl;

	app->timer.Start();

	bool recreateSwapChain = false;

	GraphicsDevice::Instance().Lock();



	//draw
	{
		app->acquireTimer.Start();

		

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

		const FrameBuffer& frameBuffer = GraphicsContext::SwapChain->GetFrameBuffer(imageIndex);
		const InstanceWrapper<VkSemaphore>& backBufferSemaphore = frameBuffer.GetLock();

		VkSemaphore renderSemaphore[] = { app->renderSemaphore->GetNative() }; //This semaphore will be signaled when done with rendering the queue

																		  //Sleep(4);
		app->acquireTimer.Stop();
		app->drawCallTimer.Start();

		std::shared_ptr<CommandBuffer> commandBuffer = app->commandBuffers[imageIndex];
		{
			
			//std::cout << "current index: " << GraphicsContext::DescriptorPool->GetCurrentIndex() << std::endl;

			{
				commandBuffer->StartRecording(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = GraphicsContext::RenderPass->GetNative();
				renderPassInfo.framebuffer = frameBuffer.GetNative();
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = GraphicsContext::SwapChain->GetExtent();

				std::array<VkClearValue, 2> clearValues = {};
				clearValues[0].color = { 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f }; // = cornflower blue :)
				clearValues[1].depthStencil = { 1.0f, 0 }; //1.0 means pixel is furthest away, so stuff can be rendered on top of it.

				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();

				vkCmdBeginRenderPass(commandBuffer->GetNative(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			}

			//TODO: Make the prepare threadsafe by doing a copy?
			//TODO: dont pass the mesh, it should be owned / held by the renderObject
			//app->renderobject->PrepareDraw(commandBuffer);
			app->model->Draw(commandBuffer);

			//app->imguiVulkan->Render(commandBuffer);

			{
				vkCmdEndRenderPass(commandBuffer->GetNative());
				commandBuffer->StopRecording();
			}

		}
		app->drawCallTimer.Stop();

		{
			//As soon as we're done creating our command buffers, let the update thread know it can continue processing its next frame
			app->renderFrameIndex++;
			RavenApp::updateThreadWait.notify_one();
		}

		//At this point, make sure to wait that the previous render submit was done.
		{
			result = vkWaitForFences(GraphicsContext::LogicalDevice, 1, &app->renderFence, true, UINT64_MAX);
			if (result != VK_SUCCESS)
			{
				std::cout << "vkWaitForFences error: " << Vulkan::GetVkResultAsString(result) << std::endl;
				//assert(false && "failed to wait for fences!");
				goto crap;
			}

			vkResetFences(GraphicsContext::LogicalDevice, 1, &app->renderFence);
		}

		app->renderQueuTimer.Start();

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &backBufferSemaphore; //Wait until this semaphore is signaled to continue with executing the command buffers
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer->GetNative();

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = renderSemaphore;
		GraphicsContext::QueueLock.lock();
		{
			//Draw (wait until surface is available)
			result = vkQueueSubmit(GraphicsContext::GraphicsQueue, 1, &submitInfo, app->renderFence);
			if (result != VK_SUCCESS)
			{
				std::cout << "vkQueueSubmit error: " << Vulkan::GetVkResultAsString(result) << std::endl;
				assert(false && "failed to submit draw command buffer!");
			}
		}
		GraphicsContext::QueueLock.unlock();

crap:

		//Sleep(4);
		app->renderQueuTimer.Stop();
		app->presentTimer.Start();

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = renderSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &GraphicsContext::SwapChain->GetNative();
		presentInfo.pImageIndices = &imageIndex;

		GraphicsContext::QueueLock.lock();
		{
			//Present (wait until drawing is done)
			result = vkQueuePresentKHR(GraphicsContext::PresentQueue, &presentInfo);

			if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				std::cout << "vkQueuePresentKHR error: " << Vulkan::GetVkResultAsString(result) << std::endl;
				recreateSwapChain = true;
			}
			else if (result != VK_SUCCESS)
			{
				std::cout << "vkQueuePresentKHR error: " << Vulkan::GetVkResultAsString(result) << std::endl;
				assert(false && "failed to present!");
			}
		}
		GraphicsContext::QueueLock.unlock();
		app->presentTimer.Stop();
	}
	GraphicsDevice::Instance().Unlock();


	if (recreateSwapChain)
	{
		RavenApp::WindowResizedCallback(app->window, GraphicsContext::WindowSize.x, GraphicsContext::WindowSize.y);
	}

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

	float rotation = 310-70;
	float translationY = -1;
	//float scale = 0.25;
	float scale = 1;

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
			
			//if (renderobject->standardMaterial != nullptr)
			{
				static auto startTime = std::chrono::high_resolution_clock::now();

				CameraUBO& camera = model->AccessUBO();
				
				auto currentTime = std::chrono::high_resolution_clock::now();
				float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

				//renderobject->camera->model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				//renderobject->camera->model = glm::rotate(renderobject->camera->model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));		

				camera.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, translationY, 0.0f));
				camera.model = glm::rotate(camera.model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
				camera.model = glm::scale(camera.model, glm::vec3(scale, scale, scale));

				camera.view = glm::lookAt(glm::vec3(2, 1, 2), glm::vec3(0.0f, 0, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				//renderobject->camera->view = glm::lookAt(glm::vec3(40.0f, 40.0f, 40.0f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				
				camera.proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.01f, 100.0f);
				camera.proj[1][1] *= -1;
			}

			/*imguiVulkan->NewFrame(delta);

			if (imguiVulkan->IsReady())
			{
				ImGui::Begin("Model");

				ImGui::DragFloat("Object Rotation", &rotation, 0.1f);
				ImGui::DragFloat("Object Translation Y", &translationY, 0.01f);
				ImGui::DragFloat("Scale", &scale, 0.0001f);

				ImGui::End();
			}*/
				
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
	RavenApp::renderThreadWait.notify_one();
	renderThread.join();               // pauses until second finishes
#endif

	for (auto& x : commandBuffers) 
	{ 
		x = nullptr;
	}

	vkDeviceWaitIdle(GraphicsContext::LogicalDevice);
}	

void RavenApp::WindowResizedCallback(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0)
		return;

	std::cout << "WindowResizedCallback:  " << width << "x" << height << std::endl;

	GraphicsContext::WindowSize = glm::ivec2(width, height);

	GraphicsDevice::Instance().SwapchainInvalidated();

	for (size_t i = 0; i < OnWindowResized.size(); i++)
	{
		OnWindowResized[i](width, height);
	}

	RavenApp* app = reinterpret_cast<RavenApp*>(glfwGetWindowUserPointer(window));

	app->model->WindowResized(width, height);
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