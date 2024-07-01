/*

File: ./core/include/celerique/internal/engine.h
Author: Aldhinn Espinas
Description: This header file contains internal interfaces to the engine.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_INTERNAL_ENGINE_HEADER_FILE)
#define CELERIQUE_INTERNAL_ENGINE_HEADER_FILE

#include <celerique.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <list>
#include <memory>
#include <atomic>
#include <shared_mutex>

namespace celerique { namespace internal {
    /// @brief The class description of the engine's internal implementations.
    class Engine final : public virtual IStateful, public virtual IEventListener {
    public:
        /// @brief Updates the state of the engine.
        /// @param ptrArg The shared pointer to the update data container.
        void onUpdate(::std::shared_ptr<IUpdateData> ptrUpdateData = nullptr) override;
        /// @brief The event handler method.
        /// @param ptrEvent The shared pointer to the event being dispatched.
        void onEvent(::std::shared_ptr<Event> ptrEvent) override;
        /// @brief Add an application layer to be managed by the engine.
        /// @param ptrAppLayer The unique pointer to the application layer instance.
        void addAppLayer(::std::unique_ptr<IApplicationLayer>&& ptrAppLayer);
        /// @brief Add a graphical user interface window to be managed by the engine.
        /// @param ptrWindow The unique pointer to the window instance.
        void addWindow(::std::unique_ptr<IWindow>&& ptrWindow);
        /// @brief Creates and run the application run loop.
        void run();

        /// @brief Gets the reference to the engine object.
        static Engine& getRef();

    // Private helper functions.
    private:
        /// @brief The event handler for engine shutdown event.
        /// @param ptrEvent The shared pointer to the event being dispatched.
        void onEngineShutdown(::std::shared_ptr<Event> ptrEvent);

    // Private member variables.
    private:
        /// @brief The collection of application layer instances.
        ::std::list<::std::unique_ptr<IApplicationLayer>> _listPtrAppLayers;
        /// @brief The mutex for `_listPtrAppLayers`.
        ::std::shared_mutex _layerMutex;
        /// @brief The graphical user interface windows managed by the engine.
        ::std::list<::std::unique_ptr<IWindow>> _listPtrWindows;
        /// @brief The mutex for `_listPtrWindows`.
        ::std::shared_mutex _windowsMutex;
        /// @brief The state that indicate if the application loop should keep running.
        ::std::atomic<bool> _atomicShouldAppLoopRunning = true;

    private:
        /// @brief Private default constructor to prevent external instantiation.
        Engine();
        /// @brief Private destructor to prevent external deletion.
        ~Engine();

    public:
        /// @brief Prevent copying.
        Engine(const Engine&) = delete;
        /// @brief Prevent moving.
        Engine(Engine&&) = delete;
        /// @brief Prevent copy re-assignment.
        Engine& operator=(const Engine&) = delete;
        /// @brief Prevent move re-assignment.
        Engine& operator=(Engine&&) = delete;
    };
}}
#endif
// End C++ Only Region.

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.