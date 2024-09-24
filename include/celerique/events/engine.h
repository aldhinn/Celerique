/*

File: ./include/celerique/events/engine.h
Author: Aldhinn Espinas
Description: This header file contains engine event types.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_EVENTS_ENGINE_HEADER_FILE)
#define CELERIQUE_EVENTS_ENGINE_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/events.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
namespace celerique { namespace event {
    /// @brief A class of events regarding engine events.
    class Engine : public virtual EventBase {
    public:
        /// @brief Pure virtual destructor.
        virtual ~Engine() = 0;
    };

    /// @brief An event type regarding the engine shutting down.
    class CELERIQUE_SHARED_SYMBOL EngineShutdown final : public virtual event::Engine,
    public virtual EventBase {
    public:
        CELERIQUE_IMPL_EVENT(EngineShutdown, CELERIQUE_EVENT_CATEGORY_ENGINE);
    };
}}
#endif
// End C++ Only Region.
#endif
// End of file.
// DO NOT WRITE BEYOND HERE.