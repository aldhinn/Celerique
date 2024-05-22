/*

File: ./x11/include/celerique/x11/internal/window.h
Author: Aldhinn Espinas
Description: This header file contains internal declarations wrapping around an x11 window.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_X11_INTERNAL_WINDOW_HEADER_FILE)
#define CELERIQUE_X11_INTERNAL_WINDOW_HEADER_FILE

#include <celerique/graphics.h>
#include <celerique/encoding/keyboard.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <atomic>

namespace celerique { namespace x11 { namespace internal {
    /// @brief Wrapper for an x11 window.
    class Window final : public virtual IWindow {
    public:
        /// @brief Member init constructor.
        /// @param defaultWidth The default horizontal dimension of the window.
        /// @param defaultHeight The default vertical dimension of the window.
        /// @param title The title on the window's title bar.
        Window(
            PixelUnits defaultWidth, PixelUnits defaultHeight, ::std::string&& title
        );

        /// @brief Updates the state.
        /// @param ptrArg The unique pointer to the update data container.
        void onUpdate(::std::unique_ptr<IUpdateData>&& ptrUpdateData = nullptr) override;

    // Helper functions.
    private:
        /// @brief Convert the x11 key code to the Celerique key codes.
        /// @param x11KeySym The x11 key sym value.
        /// @return The Celerique key code value.
        static CeleriqueKeyCode x11KeyCodeToCeleriqueKeyCode(KeySym x11KeySym);

    // Private member variables.
    private:
        /// @brief The pointer to the X11 display.
        _XDisplay* _ptrDisplay;
        /// @brief The coded message for window closing request.
        Atom _atomWmDeleteMessage;
        /// @brief The atom value for `_NET_WM_STATE`.
        Atom _atomNetWmState;
        /// @brief The atom value for `_NET_WM_STATE_HIDDEN`.
        Atom _atomNetWmStateHidden;
        /// @brief The state variable indicating whether this window is active or not.
        ::std::atomic<bool> _atomicIsActive = true;
        /// @brief The atomic container for the most recent recorded x-coordinate of the mouse.
        ::std::atomic<PixelUnits> _atomicRecentMouseXPos = 0;
        /// @brief The atomic container for the most recent recorded y-coordinate of the mouse.
        ::std::atomic<PixelUnits> _atomicRecentMouseYPos = 0;
        /// @brief The state variable whether the mouse pointer is being tracked.
        ::std::atomic<bool> _atomicMousePointerTracking = false;
        /// @brief The atomic container for the most recent recorded horizontal coordinate position of the window in the screen.
        ::std::atomic<PixelUnits> _atomicRecentWindowXPos;
        /// @brief The atomic container for the most recent recorded verticals coordinate position of the window in the screen.
        ::std::atomic<PixelUnits> _atomicRecentWindowYPos;
        /// @brief The atomic container for the most recent recorded width of the window.
        ::std::atomic<PixelUnits> _atomicRecentWindowWidth;
        /// @brief The atomic container for the most recent recorded height of the window.
        ::std::atomic<PixelUnits> _atomicRecentWindowHeight;

    public:
        /// @brief Destructor.
        ~Window();
    };
}}}
#endif
// End C++ Only Region.

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.