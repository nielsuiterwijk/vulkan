#include "EventSystem.h"

EventSystem* EventSystem::_pInstance = nullptr;

void EventSystem::Start()
{
	if ( EventSystem::_pInstance != nullptr )
	{
		ASSERT( false );
		return;
	}

	EventSystem::_pInstance = new EventSystem();
}

void EventSystem::Stop()
{
	EventSystem::_pInstance->Poll();

	delete EventSystem::_pInstance;
	EventSystem::_pInstance = nullptr;
}

void EventSystem::Poll()
{
	size_t CurrentEventsQueued = _QueuedEvents.size();

	while ( CurrentEventsQueued > 0 )
	{
		PollOnce();
		--CurrentEventsQueued;
	}
}

void EventSystem::PollOnce()
{
	std::pair<void*, EventToken> Event = _QueuedEvents.back();
	_QueuedEvents.pop_back();

	DoCallbacks( Event.first, Event.second );
}

void EventSystem::Register( EventListener* Listener )
{
	_EvenListeners.emplace_back( Listener );
}

void EventSystem::Async( void* pData, EventToken Id )
{
	EventSystem::_pInstance->_QueuedEvents.emplace_back( pData, Id );
}

void EventSystem::Sync( void* pData, EventToken Id )
{
	DoCallbacks( pData, Id );
}
