#pragma once

#include "graphics/helpers/VulkanSemaphore.h"
#include "helpers/Timer.h"

#include <functional>

class CommandBuffer;
class RavenApp;

typedef std::function<void( CommandBuffer* )> RenderCallback;

class CRenderThread
{
	friend class RavenApp;

public:
	CRenderThread() {};



	void Start();
	void Stop();

	void Initialize();
	void Destroy();

	Mutex& AccessNotificationMutex() { return _NotificationMutex; }
	std::condition_variable_any& AccessRunCondition() { return _RenderRunCondition; }
	uint64_t GetRenderFrame() const { return _RenderFrame; }

	void QueueRender( RenderCallback Callback ) { _Callbacks.emplace_back( Callback ); }

private:
	void ThreadRunner();
	void DoFrame();

	std::thread _Thread;


	std::vector<CommandBuffer*> _CommandBuffers;
	std::vector<RenderCallback> _Callbacks;

	uint64_t _RenderFrame = 0;
	bool _ShouldRun = true;

	Mutex _NotificationMutex;
	std::condition_variable_any _RenderRunCondition;

	VkFence _RenderFence = {};
	std::unique_ptr<VulkanSemaphore> _pRenderSemaphore;

	float _AccumelatedTime = 0;
	Timer _TotalTimer;
	Timer _AcquireTimer;
	Timer _DrawCallTimer;
	Timer _RenderQueuTimer;
	Timer _PresentTimer;
};