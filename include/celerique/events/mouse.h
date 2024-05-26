/*

File: ./include/celerique/events/mouse.h
Author: Aldhinn Espinas
Description: This header file contains event type definitions for events
    relating to mouse inputs.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_EVENTS_MOUSE_HEADER_FILE)
#define CELERIQUE_EVENTS_MOUSE_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/events.h>
#include <celerique/events/cursor.h>

/// @brief The type of mouse button involved in a mouse event.
typedef uint8_t CeleriqueMouseButton;

/// @brief Mouse button code for null.
#define CELERIQUE_MOUSE_BUTTON_NULL                                 0x00
/// @brief The left button of the mouse.
#define CELERIQUE_MOUSE_BUTTON_LEFT                                 0x01
/// @brief The right button of the mouse.
#define CELERIQUE_MOUSE_BUTTON_RIGHT                                0x02
/// @brief The clickable scroll button.
#define CELERIQUE_MOUSE_BUTTON_SCROLL                               0x03

// Begin C++ Only Region.
#if defined(__cplusplus)
namespace celerique { namespace event {
    /// @brief The type of mouse button involved in a mouse event.
    typedef CeleriqueMouseButton MouseButton;

    /// @brief A class of events regarding mouse events.
    class Mouse : public virtual Cursor, public virtual Event {
    public:
        /// @brief Pure virtual destructor.
        virtual ~Mouse() = 0;
    };

    /// @brief An event type regarding the mouse cursor moving.
    class CELERIQUE_SHARED_SYMBOL MouseMoved final :
    public virtual CursorMoved, public virtual Event, public virtual Mouse {
    public:
        /// @brief Init constructor.
        /// @param deltaX The horizontal component of the offset.
        /// @param deltaY The vertical component of the offset.
        inline MouseMoved(PixelUnits deltaX, PixelUnits deltaY) :
        CursorMoved(deltaX, deltaY) {}

        CELERIQUE_IMPL_EVENT(MouseMoved, CELERIQUE_EVENT_CATEGORY_MOUSE);
    };

    /// @brief An event type regarding a mouse button being clicked.
    class CELERIQUE_SHARED_SYMBOL MouseClicked final : public virtual Mouse,
    public virtual CursorPointed, public virtual Event {
    public:
        /// @brief Init constructor.
        /// @param button The mouse button being clicked.
        /// @param xPos The horizontal position of the cursor in the screen.
        /// @param yPos The vertical position of the cursor in the screen.
        inline MouseClicked(MouseButton button, PixelUnits xPos, PixelUnits yPos) :
        _button(button), CursorPointed(xPos, yPos) {}

        /// @brief The mouse button being clicked.
        /// @return `_button` value.
        inline MouseButton button() const { return _button; }

    private:
        /// @brief The mouse button being clicked.
        MouseButton _button;

    public:
        CELERIQUE_IMPL_EVENT(MouseClicked, CELERIQUE_EVENT_CATEGORY_MOUSE);
    };

    /// @brief An event type regarding a mouse button being released.
    class CELERIQUE_SHARED_SYMBOL MouseReleased final : public virtual Mouse,
    public virtual CursorPointed, public virtual Event {
    public:
        /// @brief Init constructor.
        /// @param button The mouse button being released.
        /// @param xPos The horizontal position of the cursor in the screen.
        /// @param yPos The vertical position of the cursor in the screen.
        inline MouseReleased(MouseButton button, PixelUnits xPos, PixelUnits yPos) :
        _button(button), CursorPointed(xPos, yPos) {}

        /// @brief The mouse button being released.
        /// @return `_button` value.
        inline MouseButton button() const { return _button; }

    private:
        /// @brief The mouse button being released.
        MouseButton _button;

    public:
        CELERIQUE_IMPL_EVENT(MouseReleased, CELERIQUE_EVENT_CATEGORY_MOUSE);
    };

    /// @brief An event type regarding a mouse scroll.
    class CELERIQUE_SHARED_SYMBOL MouseScrolled final : public virtual Mouse,
    public virtual Event {
    public:
        /// @brief Init constructor.
        /// @param deltaX The horizontal component of the offset.
        /// @param deltaY The vertical component of the offset.
        inline MouseScrolled(float deltaX, float deltaY) :
        _deltaX(deltaX), _deltaY(deltaY) {}

        /// @brief The horizontal component of the offset.
        /// @return `_deltaX` value.
        inline float deltaX() const { return _deltaX; }
        /// @brief The vertical component of the offset.
        /// @return `_deltaY` value.
        inline float deltaY() const { return _deltaY; }

        CELERIQUE_IMPL_EVENT(MouseScrolled, CELERIQUE_EVENT_CATEGORY_MOUSE);

    private:
        /// @brief The horizontal component of the offset.
        float _deltaX;
        /// @brief The vertical component of the offset.
        float _deltaY;
    };
}}
#endif
// End C++ Only Region.
#endif
// End of file.
// DO NOT WRITE BEYOND HERE.