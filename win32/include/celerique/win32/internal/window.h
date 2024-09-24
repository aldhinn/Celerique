/*

File: ./win32/include/celerique/win32/internal/window.h
Author: Aldhinn Espinas
Description: This header file contains internal declarations wrapping around a win32 API window.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_WIN32_INTERNAL_WINDOW_HEADER_FILE)
#define CELERIQUE_WIN32_INTERNAL_WINDOW_HEADER_FILE

#include <celerique/graphics.h>
#include <celerique/encoding/keyboard.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <atomic>

namespace celerique { namespace win32 { namespace internal {
    /// @brief Wrapper for a win32 window.
    class Window final : public virtual WindowBase {
    public:
        /// @brief Member init constructor.
        /// @param defaultWidth The default horizontal dimension of the window.
        /// @param defaultHeight The default vertical dimension of the window.
        /// @param title The title on the window's title bar.
        Window(
            PixelUnits defaultWidth, PixelUnits defaultHeight, ::std::string&& title
        );

        /// @brief Updates the state.
        /// @param ptrArg The shared pointer to the update data container.
        void onUpdate(::std::shared_ptr<IUpdateData> ptrUpdateData = nullptr) override;

    // Helper methods.
    private:
        /// @brief Converts the key codes (in `WPARAM`) from the
        /// win32 api to this framework's key code.
        /// @param wParam The keyboard key code value in `WPARAM`.
        /// @return The corresponding `CeleriqueKeyCode` value.
        static CeleriqueKeyCode win32WParamToCeleriqueKeyCode(WPARAM wParam);

    // Win32 event handlers.
    private:
        /// @brief The event handler for the win32 window.
        /// @param windowHandle The handle to the window.
        /// @param uMessage The message code.
        /// @param wParam The word parameter.
        /// @param lParam The long parameter.
        /// @return Procedure result to be returned to the win32 api.
        static LRESULT CALLBACK WindowProc(
            HWND windowHandle, UINT uMessage, WPARAM wParam, LPARAM lParam
        );

    // Private member variables.
    private:
        /// @brief The state variable that indicates if the window class has already been registered.
        static bool _hasWindowClassRegistered;
        /// @brief The atomic container for the most recent recorded x-coordinate of the mouse.
        ::std::atomic<PixelUnits> _atomicRecentMouseXPos = 0;
        /// @brief The atomic container for the most recent recorded y-coordinate of the mouse.
        ::std::atomic<PixelUnits> _atomicRecentMouseYPos = 0;
        /// @brief The state variable whether the mouse pointer is being tracked.
        ::std::atomic<bool> _atomicMousePointerTracking = false;

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