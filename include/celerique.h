/*

File: ./include/celerique.h
Author: Aldhinn Espinas
Description: This header file contains all the necessary data structures and function calls to
    interface with the Celerique Engine in C, C++ or any similarly compatible programming languages.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_ENGINE_HEADER_FILE)
#define CELERIQUE_ENGINE_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/types.h>
#include <celerique/logging.h>
#include <celerique/events.h>
#include <celerique/math.h>
#include <celerique/graphics.h>

#include <celerique/events/cursor.h>
#include <celerique/events/keyboard.h>
#include <celerique/events/mouse.h>
#include <celerique/events/window.h>
#include <celerique/events/engine.h>

#if defined(CELERIQUE_FOR_LINUX_SYSTEMS) || defined(CELERIQUE_FOR_BSD_SYSTEMS)
#include <celerique/x11/window.h>
#elif defined(CELERIQUE_FOR_WINDOWS)
#include <celerique/win32/window.h>
#else
#error No graphical user interface window wrapper for target system yet.
#endif

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <chrono>

namespace celerique {
    /// @brief The container for the engine's update argument data.
    class EngineUpdateData;
    /// @brief The interface for an application layer.
    class IApplicationLayer;

    /// @brief Updates the state of the engine.
    /// @param ptrArg The shared pointer to the update data container.
    CELERIQUE_SHARED_SYMBOL void onUpdate(::std::shared_ptr<EngineUpdateData> ptrUpdateData = nullptr);
    /// @brief Add an application layer to be managed by the engine.
    /// @param ptrAppLayer The unique pointer to the application layer instance.
    CELERIQUE_SHARED_SYMBOL void addAppLayer(::std::unique_ptr<IApplicationLayer>&& ptrAppLayer);
    /// @brief Add a graphical user interface window to be managed by the engine.
    /// @param ptrWindow The unique pointer to the window instance.
    CELERIQUE_SHARED_SYMBOL void addWindow(::std::unique_ptr<IWindow>&& ptrWindow);
    /// @brief Creates and run the application run loop.
    CELERIQUE_SHARED_SYMBOL void run();

    /// @brief The container for the engine's update argument data.
    class CELERIQUE_SHARED_SYMBOL EngineUpdateData : public virtual IUpdateData {
    public:
        EngineUpdateData(::std::chrono::nanoseconds&& elapsedNanoSecs);

        /// @return The amount of time in nanoseconds that passed since the last update cycle.
        int64_t elapsedNanoSecs() const;
        /// @return The amount of time in microseconds that passed since the last update cycle.
        int64_t elapsedMicroSecs() const;
        /// @return The amount of time in milliseconds that passed since the last update cycle.
        int64_t elapsedMilliSecs() const;

    // Private member variables.
    private:
        /// @brief The amount of time in nano seconds that passed since the last update cycle.
        ::std::chrono::nanoseconds _elapsedNanoSecs;
    };

    /// @brief The interface for an application layer.
    class IApplicationLayer : public virtual IStateful, public virtual IEventListener,
    public virtual EventBroadcaster {
    public:
        /// @brief Virtual destructor.
        virtual ~IApplicationLayer() {}
    };
}
#endif
// End C++ Only Region.

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.