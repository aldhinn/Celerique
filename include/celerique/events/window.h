/*

File: ./include/celerique/events/window.h
Author: Aldhinn Espinas
Description: This header file contains event type definitions for events
    in a desktop window (not referring to just Microsoft's product).

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_EVENTS_WINDOW_HEADER_FILE)
#define CELERIQUE_EVENTS_WINDOW_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/events.h>
#include <celerique/types.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
namespace celerique { namespace event {
    /// @brief The number of pixel units in the screen.
    typedef CeleriquePixelUnits PixelUnits;

    /// @brief A class of events regarding a desktop window.
    class Window : public virtual EventBase {
    public:
        /// @brief Pure virtual destructor.
        virtual ~Window() = 0;
    };

    /// @brief An event type that contains information about resizing of the window.
    class CELERIQUE_SHARED_SYMBOL WindowResize final :
    public virtual Window, public virtual EventBase {
    public:
        /// @brief Init constructor.
        /// @param width Describes the new width of the window being resized.
        /// @param height Describes the new height of the window being resized.
        inline WindowResize(PixelUnits width, PixelUnits height) :
        _width(width), _height(height) {}

        /// @brief Describes the new width of the window being resized.
        /// @return `_width` value.
        inline PixelUnits width() const { return _width; }
        /// @brief Describes the new height of the window being resized.
        /// @return `_height` value.
        inline PixelUnits height() const { return _height; }

        CELERIQUE_IMPL_EVENT(WindowResize, CELERIQUE_EVENT_CATEGORY_WINDOW);

    private:
        /// @brief Describes the new width of the window being resized.
        PixelUnits _width;
        /// @brief Describes the new height of the window being resized.
        PixelUnits _height;
    };

    /// @brief An event type that contains information about the
    /// movement of the window relative to the screen.
    class CELERIQUE_SHARED_SYMBOL WindowMove final :
    public virtual Window, public virtual EventBase {
    public:
        /// @brief Init constructor.
        /// @param xPos The new horizontal coordinate position relative
        /// to the screen of the window to be moved.
        /// @param yPos The new vertical coordinate position relative
        /// to the screen of the window to be moved.
        inline WindowMove(PixelUnits xPos, PixelUnits yPos) :
        _xPos(xPos), _yPos(yPos) {}

        /// @brief The new horizontal coordinate position relative
        /// to the screen of the window to be moved.
        /// @return `_xPos` value.
        inline PixelUnits xPos() const { return _xPos; }
        /// @brief The new vertical coordinate position relative
        /// to the screen of the window to be moved.
        /// @return `_yPos` value.
        inline PixelUnits yPos() const { return _yPos; }

        CELERIQUE_IMPL_EVENT(WindowMove, CELERIQUE_EVENT_CATEGORY_WINDOW);

    private:
        /// @brief The new horizontal coordinate position relative
        /// to the screen of the window to be moved.
        PixelUnits _xPos;
        /// @brief The new vertical coordinate position relative
        /// to the screen of the window to be moved.
        PixelUnits _yPos;
    };

    /// @brief An event type to be dispatched when the window is closing.
    class CELERIQUE_SHARED_SYMBOL WindowClose final :
    public virtual Window, public virtual EventBase {
    public:
        CELERIQUE_IMPL_EVENT(WindowClose, CELERIQUE_EVENT_CATEGORY_WINDOW);
    };

    /// @brief An event type to be dispatched when there is a
    /// request to close the window.
    class CELERIQUE_SHARED_SYMBOL WindowRequestClose final :
    public virtual Window, public virtual EventBase {
    public:
        CELERIQUE_IMPL_EVENT(WindowRequestClose, CELERIQUE_EVENT_CATEGORY_WINDOW);
    };

    /// @brief An event type to be dispatched when the window is minimized.
    class CELERIQUE_SHARED_SYMBOL WindowMinimized final :
    public virtual Window, public virtual EventBase {
    public:
        CELERIQUE_IMPL_EVENT(WindowMinimized, CELERIQUE_EVENT_CATEGORY_WINDOW);
    };

    /// @brief An event type to be dispatched when the window is being focused.
    class CELERIQUE_SHARED_SYMBOL WindowFocused final :
    public virtual Window, public virtual EventBase {
    public:
        CELERIQUE_IMPL_EVENT(WindowFocused, CELERIQUE_EVENT_CATEGORY_WINDOW);
    };
}}
#endif
// End C++ Only Region.
#endif
// End of file.
// DO NOT WRITE BEYOND HERE.