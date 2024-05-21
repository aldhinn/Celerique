/*

File: ./core/src/events.cpp
Author: Aldhinn Espinas
Description: This file contains implementations of the event system.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/events.h>

#include <utility>

/// @brief Pure virtual destructor.
::celerique::Event::~Event() {}

/// @brief Pure virtual destructor.
::celerique::IEventListener::~IEventListener() {}

/// @brief Pure virtual destructor.
::celerique::EventBroadcaster::~EventBroadcaster() {}

/// @brief Add an event listener callback to `_vecListeners`.
/// @param listener The function pointer to the event listener.
void ::celerique::EventBroadcaster::addEventListener(EventHandler&& listener) {
    _vecListeners.emplace_back(::std::move(listener));
}

/// @brief Add the `onEvent` method of an `IEventListener` instance.
/// @param ptrListener The pointer to the `IEventListener` instance.
void ::celerique::EventBroadcaster::addEventListener(IEventListener* ptrListener) {
    _vecListeners.emplace_back(::std::bind(
        &IEventListener::onEvent, ptrListener, ::std::placeholders::_1
    ));
}

/// @brief Dispatch event to the listeners based on the event object passed.
/// @param ptrEvent The pointer to the event to be dispatched.
/// @param strategy The dispatch strategy (blocking by default.)
void ::celerique::EventBroadcaster::broadcast(
    const ::std::shared_ptr<Event>& ptrEvent, EventHandlingStrategy strategy
) {
    EventDispatcher dispatcher(ptrEvent);
    // Dispatch to all listeners.
    for (const EventHandler& listener : _vecListeners) {
        dispatcher.dispatch<Event>(listener, strategy);
    }
}