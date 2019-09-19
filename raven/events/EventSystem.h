#pragma once

#include <stdint.h>
#include <vector>

typedef int32_t EventToken;

class EventListener
{
public:
	virtual void OnEventCallback( void* pData, EventToken Id ) = 0;
};

class EventSystem
{
public:
	static void Start();
	static void Stop();

	//Will be handled next time when event system is polled
	static void Async( void* pData, EventToken Id );
	//Will be handled immediately
	static void Sync( void* pData, EventToken Id );

public:
	void Register( EventListener* Listener );

	void Poll();
	void PollOnce();

private:
	inline static void DoCallbacks( void* pData, EventToken Id )
	{
		for ( EventListener* Listener : EventSystem::_pInstance->_EvenListeners )
		{
			Listener->OnEventCallback( pData, Id );
		}
	}

private:
	std::vector<EventListener*> _EvenListeners;

	std::vector<std::pair<void*, EventToken>> _QueuedEvents;

	static EventSystem* _pInstance;
};
