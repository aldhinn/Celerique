/*

File: ./include/celerique/events.h
Author: Aldhinn Espinas
Description: This header file contains interfaces to the event system.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_EVENTS_HEADER_FILE)
#define CELERIQUE_EVENTS_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/types.h>

/// @brief A category enum of which an event could be classified.
typedef uint8_t CeleriqueEventCategory;

/// @brief The event does not belong to any category.
#define CELERIQUE_EVENT_CATEGORY_NONE                                           0
/// @brief A specific input event relating to keyboard inputs.
#define CELERIQUE_EVENT_CATEGORY_KEYBOARD                                       CELERIQUE_LEFT_BIT_SHIFT_1(0)
/// @brief A specific input event relating to mouse inputs.
#define CELERIQUE_EVENT_CATEGORY_MOUSE                                          CELERIQUE_LEFT_BIT_SHIFT_1(1)
/// @brief A specific input event relating to touch inputs.
#define CELERIQUE_EVENT_CATEGORY_TOUCH                                          CELERIQUE_LEFT_BIT_SHIFT_1(2)
/// @brief Input events relating to mouse or touch.
#define CELERIQUE_EVENT_CATEGORY_CURSOR                                         CELERIQUE_EVENT_CATEGORY_MOUSE | CELERIQUE_EVENT_CATEGORY_TOUCH
/// @brief Engine type events.
#define CELERIQUE_EVENT_CATEGORY_ENGINE                                         CELERIQUE_LEFT_BIT_SHIFT_1(3)
/// @brief Window related events.
#define CELERIQUE_EVENT_CATEGORY_WINDOW                                         CELERIQUE_LEFT_BIT_SHIFT_1(4)
/// @brief The category for events related to any user input.
#define CELERIQUE_EVENT_CATEGORY_INPUT                                          CELERIQUE_EVENT_CATEGORY_KEYBOARD | CELERIQUE_EVENT_CATEGORY_CURSOR

/// @brief The way of handling events.
typedef uint8_t CeleriqueEventHandlingStrategy;

/// @brief A null value for `CeleriqueEventHandlingStrategy`.
#define CELERIQUE_EVENT_HANDLING_STRATEGY_NULL                                  0x00
/// @brief The current thread will prioritize handling the event.
#define CELERIQUE_EVENT_HANDLING_STRATEGY_BLOCKING                              0x01
/// @brief Spawn a separate thread to handle the event.
#define CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC                                 0x02

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <functional>
#include <thread>
#include <typeindex>
#include <memory>
#include <list>
#include <atomic>

namespace celerique {
    /// @brief A category enum of which an event could be classified.
    typedef CeleriqueEventCategory EventCategory;

    /// @brief The way of handling events.
    typedef CeleriqueEventHandlingStrategy EventHandlingStrategy;

    /// @brief Base Event type.
    class Event {
    public:
        /// @brief Determines whether this event should propagate or not.
        /// @return `_shouldPropagate` value.
        inline bool shouldPropagate() const { return _atomicShouldPropagate.load(); }
        /// @brief The propagation will now set to completion and will no longer propagate.
        virtual void completePropagation() { _atomicShouldPropagate.store(false, ::std::memory_order_release); }

        /// @brief The category this event belongs to.
        /// @return The event category value.
        virtual EventCategory category() const = 0;
        /// @brief A specific type identifier for the event. Make
        /// this uniform for each derived event class. The purpose for this
        /// is to avoid dynamic casting when checking for event types.
        /// @return The type of the event.
        virtual ::std::type_index typeID() const = 0;

    protected:
        /// @brief Atomic container for the state that determines
        /// whether or not this event should propagate.
        ::std::atomic<bool> _atomicShouldPropagate = true;

    public:
        /// @brief Pure virtual destructor.
        virtual ~Event() = 0;
    };

    /// @brief The type of an event handler.
    using EventHandler = ::std::function<void(::std::shared_ptr<Event>)>;

    /// @brief A template class for an event dispatcher.
    class EventDispatcher final {
    public:
        /// @brief Copy init constructor.
        /// @param ptrEvent The shared pointer to the event to be dispatched.
        inline EventDispatcher(const ::std::shared_ptr<Event>& ptrEvent) :
        _ptrEvent(ptrEvent) {}
        /// @brief Move init constructor.
        /// @param ptrEvent The shared pointer to the event to be dispatched.
        /// In this case, this dispatcher object will own the event pointer.
        inline EventDispatcher(::std::shared_ptr<Event>&& ptrEvent) :
        _ptrEvent(::std::move(ptrEvent)) {}

        /// @brief Dispatch the event to their relevant event handlers.
        /// @tparam TEvent The type of event to be targeted.
        /// @param handler The handler to the event.
        /// @param strategy The event handling strategy. Default to blocking.
        template <typename TEvent>
        void dispatch(
            const EventHandler& handler,
            EventHandlingStrategy strategy = CELERIQUE_EVENT_HANDLING_STRATEGY_BLOCKING
        ) {
            static_assert(
                ::std::is_base_of<::celerique::Event, TEvent>::value,
                "TEvent must be derived from `::celerique::Event`"
            );

            // Do nothing if event pointer is null.
            if (_ptrEvent == nullptr) return;

            // Ensuring that the has the correct type ID if the dispatching
            // to a specific event implementation.
            if (::std::type_index(typeid(TEvent)) != _ptrEvent->typeID() &&
            !::std::is_same<TEvent, Event>::value) return;

            switch(strategy) {
            case CELERIQUE_EVENT_HANDLING_STRATEGY_BLOCKING:
                if (_ptrEvent->shouldPropagate()) {
                    // Blocking execution.
                    handler(_ptrEvent);
                }
                break;
            case CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC:
                ::std::thread handlingThread(::std::bind(
                    handler, _ptrEvent
                ));
                // Detaching from this thread so the event handling
                // can still go on even if `handlingThread` goes out of scope.
                handlingThread.detach();
                break;
            }
        }

    private:
        /// @brief The shared pointer to the event to be dispatched.
        ::std::shared_ptr<Event> _ptrEvent;
    };

    /// @brief An interface to an event listener.
    class IEventListener {
    public:
        /// @brief The event handler method.
        /// @param ptrEvent The shared pointer to the event being dispatched.
        virtual void onEvent(::std::shared_ptr<Event> ptrEvent) {}

    public:
        /// @brief Pure virtual destructor.
        virtual ~IEventListener() = 0;
    };

    /// @brief A base type for anything that dispatches events to its listeners.
    class EventBroadcaster {
    public:
        /// @brief Add an event listener callback to `_vecListeners`.
        /// @param listener The function pointer to the event listener.
        void addEventListener(EventHandler&& listener);
        /// @brief Add the `onEvent` method of an `IEventListener` instance.
        /// @param ptrListener The pointer to the `IEventListener` instance.
        void addEventListener(IEventListener* ptrListener);

    protected:
        /// @brief Dispatch event to the listeners based on the event object passed.
        /// @param ptrEvent The pointer to the event to be dispatched.
        /// @param strategy The dispatch strategy (blocking by default.)
        void broadcast(
            const ::std::shared_ptr<Event>& ptrEvent,
            EventHandlingStrategy strategy = CELERIQUE_EVENT_HANDLING_STRATEGY_BLOCKING
        );

    protected:
        /// @brief The list of event handlers that will receive event dispatch calls.
        ::std::list<EventHandler> _listListeners;

    public:
        /// @brief Pure virtual destructor.
        virtual ~EventBroadcaster() = 0;
    };
}
#endif
// End C++ Only Region.

// CELERIQUE_IMPL_EVENT macro definition.
#if !defined(CELERIQUE_IMPL_EVENT)
#if defined(__cplusplus)
/// @brief A macro that shortens implementation of virtual
/// methods when inheriting from `::celerique::Event`.
/// @param className The name of the class to be implemented.
/// @param eventCategory The category of which this event belongs to.
#define CELERIQUE_IMPL_EVENT(className, eventCategory) \
inline EventCategory category() const override { \
return eventCategory; \
} \
inline ::std::type_index typeID() const override { \
return ::std::type_index(typeid(className)); }
#else
/// @brief A macro that shortens implementation of virtual
/// methods when inheriting from `::celerique::Event`. (Shortens to nothing).
/// @param className The name of the class to be implemented.
/// @param eventCategory The category of which this event belongs to.
#define CELERIQUE_IMPL_EVENT(className, eventCategory)
#endif
#endif
// CELERIQUE_IMPL_EVENT macro definition END.

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.