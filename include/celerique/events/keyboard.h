/*

File: ./include/celerique/events/keyboard.h
Author: Aldhinn Espinas
Description: This header file contains event type definitions for events
    relating to keyboard inputs.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_EVENTS_KEYBOARD_HEADER_FILE)
#define CELERIQUE_EVENTS_KEYBOARD_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/types.h>
#include <celerique/events.h>
#include <celerique/encoding/keyboard.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
namespace celerique { namespace event {
    /// @brief The encoding of a key pressed in the keyboard.
    typedef CeleriqueKeyCode KeyCode;

    /// @brief A class of event regarding keyboard events.
    class Keyboard : public virtual EventBase {
    public:
        /// @brief Init constructor.
        /// @param keyCode The encoding of the keyboard key focused by this event object.
        inline Keyboard(KeyCode keyCode) :
        _keyCode(keyCode) {}

        /// @brief The encoding of the keyboard key focused by this event object.
        /// @return `_keyCode` value.
        inline KeyCode keyCode() const { return _keyCode; }

    private:
        /// @brief The encoding of the keyboard key focused by this event object.
        KeyCode _keyCode;

    public:
        /// @brief Pure virtual destructor.
        virtual ~Keyboard() = 0;
    };

    /// @brief An event type regarding keyboard key presses.
    class CELERIQUE_SHARED_SYMBOL KeyboardKeyPressed final :
    public virtual Keyboard, public virtual EventBase {
    public:
        /// @brief Init constructor.
        /// @param keyCode The encoding of the key pressed by the user.
        /// @param repeat The state whether the key being pressed is repeating
        /// or being "long pressed"
        inline KeyboardKeyPressed(KeyCode keyCode, bool repeat = false) :
        Keyboard(keyCode), _repeating(repeat) {}

        /// @brief The state whether the key being pressed is repeating
        /// or being "long pressed"
        /// @return `_repeating` value.
        inline bool repeating() const { return _repeating; }

        CELERIQUE_IMPL_EVENT(KeyboardKeyPressed, CELERIQUE_EVENT_CATEGORY_KEYBOARD);

    private:
        /// @brief The state whether the key being pressed is repeating
        /// or being "long pressed"
        bool _repeating;
    };

    /// @brief An event type regarding keyboard key is released.
    class CELERIQUE_SHARED_SYMBOL KeyboardKeyReleased final :
    public virtual Keyboard, public virtual EventBase {
    public:
        /// @brief Init constructor.
        /// @param keyCode The encoding of the key released by the user.
        inline KeyboardKeyReleased(KeyCode keyCode) :
        Keyboard(keyCode) {}

        CELERIQUE_IMPL_EVENT(KeyboardKeyReleased, CELERIQUE_EVENT_CATEGORY_KEYBOARD);
    };
}}
#endif
// End C++ Only Region.
#endif
// End of file.
// DO NOT WRITE BEYOND HERE.