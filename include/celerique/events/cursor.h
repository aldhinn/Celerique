/*

File: ./include/celerique/events/cursor.h
Author: Aldhinn Espinas
Description: This header file contains event type definitions for events
    relating to cursor inputs.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_EVENTS_CURSOR_HEADER_FILE)
#define CELERIQUE_EVENTS_CURSOR_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/events.h>
#include <celerique/types.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
namespace celerique { namespace event {
    /// @brief The number of pixel units in the screen.
    typedef CeleriquePixelUnits PixelUnits;

    /// @brief A class of events regarding cursors events like a mouse or a touch event.
    class Cursor : public virtual EventBase {
    public:
        /// @brief Pure virtual destructor.
        virtual ~Cursor() = 0;
    };

    /// @brief A cursor event pointed at a specific coordinate in the screen.
    class CursorPointed : public virtual Cursor, public virtual EventBase {
    public:
        /// @brief Init constructor.
        /// @param xPos The horizontal position of the cursor in the screen.
        /// @param yPos The vertical position of the cursor in the screen.
        inline CursorPointed(PixelUnits xPos, PixelUnits yPos) :
        _xPos(xPos), _yPos(yPos) {}

        /// @brief The horizontal position of the cursor in the screen.
        /// @return `_xPos` value.
        inline PixelUnits xPos() const { return _xPos; }
        /// @brief The vertical position of the cursor in the screen.
        /// @return `_yPos` value.
        inline PixelUnits yPos() const { return _yPos; }

    private:
        /// @brief The horizontal position of the cursor in the screen.
        PixelUnits _xPos;
        /// @brief The vertical position of the cursor in the screen.
        PixelUnits _yPos;

    public:
        /// @brief Pure virtual destructor.
        virtual ~CursorPointed() = 0;
    };

    /// @brief A cursor event regarding cursor movements in the screen.
    class CursorMoved : public virtual Cursor, public virtual EventBase {
    public:
        /// @brief Init constructor.
        /// @param deltaX The horizontal component of the offset.
        /// @param deltaY The vertical component of the offset
        inline CursorMoved(PixelUnits deltaX, PixelUnits deltaY) :
        _deltaX(deltaX), _deltaY(deltaY) {}

        /// @brief The horizontal component of the offset.
        /// @return `_deltaX` value.
        inline PixelUnits deltaX() const { return _deltaX; }
        /// @brief The vertical component of the offset.
        /// @return `_deltaY` value.
        inline PixelUnits deltaY() const { return _deltaY; }

    private:
        /// @brief The horizontal component of the offset.
        PixelUnits _deltaX;
        /// @brief The vertical component of the offset.
        PixelUnits _deltaY;

    public:
        /// @brief Pure virtual destructor.
        virtual ~CursorMoved() = 0;
    };
}}
#endif
// End C++ Only Region.
#endif
// End of file.
// DO NOT WRITE BEYOND HERE.