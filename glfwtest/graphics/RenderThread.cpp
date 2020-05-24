#include "RenderThread.h"

#include "graphics/GraphicsContext.h"

CRenderThread* CRenderThread::pInstance = nullptr;

void CRenderThread::Start()
{
	_Thread = std::thread( &CRenderThread::ThreadRunner, this );
}

void CRenderThread::Initialize()
{
	ASSERT( CRenderThread::pInstance == nullptr );

	CRenderThread::pInstance = this;

	VkFenceCreateInfo FenceInfo = {};
	FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceInfo.pNext = VK_NULL_HANDLE;
	FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkResult result = vkCreateFence( GraphicsContext::LogicalDevice, &FenceInfo, GraphicsContext::LocalAllocator, &_RenderFence );
	assert( result == VK_SUCCESS );

	_pRenderSemaphore.reset( new VulkanSemaphore() );

#if MULTITHREADED_RENDERING == 0
	GraphicsContext::CommandBufferPool->Create( _CommandBuffers, GraphicsContext::SwapChain->GetAmountOfFrameBuffers() );
#endif
}

void CRenderThread::Stop()
{
	_ShouldRun = false;
	_RenderRunCondition.notify_one();
	_Thread.join();
}

void CRenderThread::Destroy()
{
	ASSERT( CRenderThread::pInstance != nullptr );
	CRenderThread::pInstance = nullptr;
	_pRenderSemaphore = nullptr;
	vkDestroyFence( GraphicsContext::LogicalDevice, _RenderFence, GraphicsContext::LocalAllocator );
}

void CRenderThread::ThreadRunner()
{
#if MULTITHREADED_RENDERING == 1
	GraphicsContext::CommandBufferPool->Create( _CommandBuffers, GraphicsContext::SwapChain->GetAmountOfFrameBuffers(), CommandBufferType::AutoReset );
	while ( _ShouldRun )
	{
		if ( GraphicsContext::LogicalDevice == nullptr )
			Sleep( 50 );

		DoFrame();
	}
#endif
}

void CRenderThread::DoFrame()
{
	VkResult result;
#if MULTITHREADED_RENDERING == 1
	{
		std::unique_lock<Mutex> Lock( AccessNotificationMutex() );
		// Wait until update thread is done with the next frame
		//RavenApp::renderThreadWait.wait( lock, [=] { return app->updateFrameIndex > app->renderFrameIndex || !app->run; } );
		_RenderRunCondition.wait( Lock );
	}
#endif
	if ( !_ShouldRun )
		return;

	//std::cout << "Start of frame " << app->renderFrameIndex << " render " << std::endl;

	_TotalTimer.Start();

	bool RecreateSwapChain = false;

	GraphicsDevice& GraphicsDevice = GraphicsDevice::Instance();

	GraphicsDevice.Lock();

	//draw
	{
		_AcquireTimer.Start();

		//Prepare
		uint32_t imageIndex = -1;
		do
		{
			imageIndex = GraphicsContext::SwapChain->PrepareBackBuffer();

			if ( imageIndex == -1 )
			{
				GraphicsDevice.Unlock();
				GraphicsDevice.SwapchainInvalidated();
				GraphicsDevice.Lock();
			}

		} while ( imageIndex == -1 );

		const FrameBuffer& frameBuffer = GraphicsContext::SwapChain->GetFrameBuffer( imageIndex );
		const InstanceWrapper<VkSemaphore>& backBufferSemaphore = frameBuffer.GetLock();

		VkSemaphore RenderSemaphore[] = { _pRenderSemaphore->GetNative() }; //This semaphore will be signaled when done with rendering the queue

		//Sleep(4);
		_AcquireTimer.Stop();
		_DrawCallTimer.Start();

		CommandBuffer* pCommandBuffer = _CommandBuffers[ imageIndex ];
		{

			//std::cout << "current index: " << GraphicsContext::DescriptorPool->GetCurrentIndex() << std::endl;
			//vkResetCommandBuffer( pCommandBuffer->GetNative(), 0 );

			{
				pCommandBuffer->StartRecording( VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT );

				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = GraphicsContext::RenderPass->GetNative();
				renderPassInfo.framebuffer = frameBuffer.GetNative();
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = GraphicsContext::SwapChain->GetExtent();

				std::array<VkClearValue, 2> clearValues = {};
				clearValues[ 0 ].color = { 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f }; // = cornflower blue :)
				clearValues[ 1 ].depthStencil = { 1.0f, 0 }; //1.0 means pixel is furthest away, so stuff can be rendered on top of it.

				renderPassInfo.clearValueCount = static_cast<uint32_t>( clearValues.size() );
				renderPassInfo.pClearValues = clearValues.data();

				vkCmdBeginRenderPass( pCommandBuffer->GetNative(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
			}

			//TODO: Make the prepare threadsafe by doing a copy?

			for ( RawRenderCallback& Callback : _RawCallbacks )
			{
				Callback( pCommandBuffer );
			}

			//for ( RenderCallback& Callback : _Callbacks )
			//{
			//	Callback( (Ecs::Entity)0, pCommandBuffer );
			//}

			_RawCallbacks.clear();

			{
				vkCmdEndRenderPass( pCommandBuffer->GetNative() );
				pCommandBuffer->StopRecording();
			}
		}
		_DrawCallTimer.Stop();

		{
			//As soon as we're done creating our command buffers, let the update thread know it can continue processing its next frame
			++_RenderFrame;
		}

		//At this point, make sure to wait that the previous render submit was done.
		{
			do
			{
				result = vkWaitForFences( GraphicsContext::LogicalDevice, 1, &_RenderFence, true, UINT64_MAX );

				if ( result != VK_SUCCESS )
				{
					std::cout << "vkWaitForFences error: " << Vulkan::GetVkResultAsString( result ) << std::endl;
				}

			} while ( result != VK_SUCCESS );

			vkResetFences( GraphicsContext::LogicalDevice, 1, &_RenderFence );
		}

		_RenderQueuTimer.Start();

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &backBufferSemaphore; //Wait until this semaphore is signaled to continue with executing the command buffers
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &pCommandBuffer->GetNative();

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = RenderSemaphore;
		GraphicsContext::QueueLock.lock();
		{
			//Draw (wait until surface is available)
			result = vkQueueSubmit( GraphicsContext::GraphicsQueue, 1, &submitInfo, _RenderFence );
			if ( result != VK_SUCCESS )
			{
				std::cout << "vkQueueSubmit error: " << Vulkan::GetVkResultAsString( result ) << std::endl;
				assert( false && "failed to submit draw command buffer!" );
			}
		}
		GraphicsContext::QueueLock.unlock();

		//Sleep(4);
		_RenderQueuTimer.Stop();
		_PresentTimer.Start();

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = RenderSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &GraphicsContext::SwapChain->GetNative();
		presentInfo.pImageIndices = &imageIndex;

		{
			std::scoped_lock<Mutex> Lock( GraphicsContext::QueueLock );
			//Present (wait until drawing is done)
			result = vkQueuePresentKHR( GraphicsContext::PresentQueue, &presentInfo );

			if ( result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR )
			{
				std::cout << "vkQueuePresentKHR error: " << Vulkan::GetVkResultAsString( result ) << std::endl;
				RecreateSwapChain = true;
			}
			else if ( result != VK_SUCCESS )
			{
				std::cout << "vkQueuePresentKHR error: " << Vulkan::GetVkResultAsString( result ) << std::endl;
				assert( false && "failed to present!" );
			}
		}
		_PresentTimer.Stop();
	}
	GraphicsDevice.Unlock();

	if ( RecreateSwapChain )
	{
		//RavenApp::WindowResizedCallback( app->window, GraphicsContext::WindowSize.x, GraphicsContext::WindowSize.y );
	}

	//Sleep(16);
	_TotalTimer.Stop();
	_AccumelatedTime += _TotalTimer.GetTimeInSeconds();

	{
		if ( _AccumelatedTime > 2.0f )
		{
			std::cout << "a: " << _AcquireTimer.GetTimeInSeconds() << " d: " << _DrawCallTimer.GetTimeInSeconds() << " q: " << _RenderQueuTimer.GetTimeInSeconds() << " p: " << _PresentTimer.GetTimeInSeconds()
					  << std::endl;
			_AccumelatedTime -= 2.0f;
		}
		//std::cout << "End of frame " << app->renderFrameIndex - 1 << " render " << std::endl;
	}
}