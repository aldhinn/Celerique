/*

File: ./core/src/engine.cpp
Author: Aldhinn Espinas
Description: This source file contains implementations of the engine.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/internal/engine.h>

#include <utility>
#include <mutex>

/// @brief Updates the state.
/// @param ptrArg The shared pointer to the update data container.
void ::celerique::internal::Engine::onUpdate(::std::shared_ptr<IUpdateData> ptrUpdateData) {
    {
        ::std::shared_lock<::std::shared_mutex> readLock(_layerMutex);
        // Update layers.
        for (::std::unique_ptr<ApplicationLayerBase>& ptrAppLayer :_listPtrAppLayers) {
            ptrAppLayer->onUpdate(ptrUpdateData);
        }
    }
    {
        ::std::shared_lock<::std::shared_mutex> readLock(_windowsMutex);
        // Update graphical user interface windows.
        for (::std::unique_ptr<WindowBase>& ptrWindow : _listPtrWindows) {
            ptrWindow->onUpdate();
        }
    }
}

/// @brief The event handler method.
/// @param ptrEvent The shared pointer to the event being dispatched.
void ::celerique::internal::Engine::onEvent(::std::shared_ptr<EventBase> ptrEvent) {
    EventDispatcher dispatcher(ptrEvent);
    dispatcher.dispatch<::celerique::event::EngineShutdown>(
        ::std::bind(&Engine::onEngineShutdown, this, ptrEvent), CELERIQUE_EVENT_HANDLING_STRATEGY_BLOCKING
    );
    // Halt here if event should not propagate any longer.
    if (!ptrEvent->shouldPropagate()) return;
    // Propagate window and input events to layers.
    if ((ptrEvent->category() & (CELERIQUE_EVENT_CATEGORY_WINDOW | CELERIQUE_EVENT_CATEGORY_INPUT)) == 0) return;

    {
        ::std::shared_lock<::std::shared_mutex> readLock(_layerMutex);

        // Dispatch input and window events to layers (from last to first).
        for (auto layerRIterator = _listPtrAppLayers.rbegin(); layerRIterator != _listPtrAppLayers.rend(); layerRIterator++) {
            dispatcher.dispatch<::celerique::EventBase>(
                ::std::bind(&ApplicationLayerBase::onEvent, (*layerRIterator).get(), ptrEvent), CELERIQUE_EVENT_HANDLING_STRATEGY_BLOCKING
            );
        }
    }
}

/// @brief Add an application layer to be managed by the engine.
/// @param ptrAppLayer The unique pointer to the application layer instance.
void ::celerique::internal::Engine::addAppLayer(::std::unique_ptr<ApplicationLayerBase>&& ptrAppLayer) {
    ::std::unique_lock<::std::shared_mutex> writeLock(_layerMutex);

    ptrAppLayer->addEventListener(this);
    _listPtrAppLayers.emplace_back(::std::move(ptrAppLayer));
    celeriqueLogTrace("Added a layer.");
}

/// @brief Add a graphical user interface window to be managed by the engine.
/// @param ptrWindow The unique pointer to the window instance.
void ::celerique::internal::Engine::addWindow(::std::unique_ptr<WindowBase>&& ptrWindow) {
    ::std::unique_lock<::std::shared_mutex> writeLock(_windowsMutex);

    ptrWindow->addEventListener(this);
    _listPtrWindows.emplace_back(::std::move(ptrWindow));
    celeriqueLogTrace("Added a graphical user interface window.");
}

/// @brief Creates and run the application run loop.
void ::celerique::internal::Engine::run() {
    using clock = ::std::chrono::high_resolution_clock;

    /// @brief The container for the previous time point.
    ::std::chrono::time_point prevTime = clock::now();
    /// @brief The container for the current time point.
    ::std::chrono::time_point currentTime = prevTime;

    celeriqueLogTrace("Starting application loop.");
    while(_atomicShouldAppLoopRunning.load()) {
        // Record current time.
        currentTime = clock::now();
        // Update engine state.
        onUpdate(::std::make_shared<EngineUpdateData>(
            ::std::chrono::duration_cast<::std::chrono::nanoseconds>(currentTime - prevTime)
        ));
        // Update previous time data.
        prevTime = currentTime;
    }
    celeriqueLogTrace("Ended application loop.");
}

/// @brief Gets the reference to the engine object.
::celerique::internal::Engine& celerique::internal::Engine::getRef() {
    /// @brief The singleton instance of the engine.
    static Engine singletonInst;
    return singletonInst;
}

/// @brief The event handler for engine shutdown event.
/// @param ptrEvent The shared pointer to the event being dispatched.
void ::celerique::internal::Engine::onEngineShutdown(::std::shared_ptr<EventBase> ptrEvent) {
    _atomicShouldAppLoopRunning.store(false, ::std::memory_order_release);
    ptrEvent->completePropagation();
    celeriqueLogTrace("Engine shutdown event was dispatched.");
}

/// @brief Private default constructor to prevent external instantiation.
::celerique::internal::Engine::Engine() {
    celeriqueLogDebug("Initialized engine.");
}

/// @brief Private destructor to prevent external deletion.
::celerique::internal::Engine::~Engine() {
    celeriqueLogDebug("Destroyed engine.");
}

/// @brief Updates the engine.
/// @param ptrArg The shared pointer to the update data container.
void ::celerique::onUpdate(::std::shared_ptr<EngineUpdateData> ptrUpdateData) {
    internal::Engine::getRef().onUpdate(::std::move(ptrUpdateData));
}

/// @brief Add an application layer to be managed by the engine.
/// @param ptrAppLayer The unique pointer to the application layer instance.
void ::celerique::addAppLayer(::std::unique_ptr<ApplicationLayerBase>&& ptrAppLayer) {
    internal::Engine::getRef().addAppLayer(::std::move(ptrAppLayer));
}

/// @brief Add a graphical user interface window to be managed by the engine.
/// @param ptrWindow The unique pointer to the window instance.
void ::celerique::addWindow(::std::unique_ptr<WindowBase>&& ptrWindow) {
    internal::Engine::getRef().addWindow(::std::move(ptrWindow));
}

/// @brief Creates and run the application run loop.
void ::celerique::run() {
    internal::Engine::getRef().run();
}

/// @brief Initializer constructor.
/// @param elapsedNanoSecs The amount of time in nano seconds that passed since the last update cycle.
::celerique::EngineUpdateData::EngineUpdateData(::std::chrono::nanoseconds&& elapsedNanoSecs) :
_elapsedNanoSecs(::std::move(elapsedNanoSecs)) {}

/// @return The amount of time in nanoseconds that passed since the last update cycle.
int64_t celerique::EngineUpdateData::elapsedNanoSecs() const {
    return _elapsedNanoSecs.count();
}

/// @return The amount of time in microseconds that passed since the last update cycle.
int64_t celerique::EngineUpdateData::elapsedMicroSecs() const {
    return ::std::chrono::duration_cast<::std::chrono::microseconds>(_elapsedNanoSecs).count();
}

/// @return The amount of time in milliseconds that passed since the last update cycle.
int64_t celerique::EngineUpdateData::elapsedMilliSecs() const {
    return ::std::chrono::duration_cast<::std::chrono::milliseconds>(_elapsedNanoSecs).count();
}

/// @brief Pure virtual destructor.
::celerique::ApplicationLayerBase::~ApplicationLayerBase() {}