/*

File: ./win32/src/window.cpp
Author: Aldhinn Espinas
Description: This source file contains internal implementation details wrapping around a win32 API window.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/win32/window.h>
#include <celerique/win32/internal/window.h>

#include <celerique/logging.h>
#include <celerique/events/window.h>
#include <celerique/events/keyboard.h>
#include <celerique/events/mouse.h>

#include <utility>
#include <stdexcept>

/// @brief The state variable that indicates if the window class has already been registered.
bool ::celerique::win32::internal::Window::_hasWindowClassRegistered = false;

::std::unique_ptr<::celerique::IWindow> celerique::win32::createWindow(
    ::celerique::win32::PixelUnits defaultWidth,
    ::celerique::win32::PixelUnits defaultHeight,
    ::std::string&& title
) {
    using ::celerique::win32::internal::Window;
    return ::std::make_unique<Window>(defaultWidth, defaultHeight, ::std::move(title));
}

/// @brief Member init constructor.
/// @param defaultWidth The default horizontal dimension of the window.
/// @param defaultHeight The default vertical dimension of the window.
/// @param title The title on the window's title bar.
::celerique::win32::internal::Window::Window(
    PixelUnits defaultWidth, PixelUnits defaultHeight, ::std::string&& title
) {
    /// @brief The name of the window class for Celerique Applications.
    const char* celeriqueWindowClassName = "CeleriqueWin32WindowClass";
    /// @brief The application instance handle.
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    // Register window class if it hasn't been registered yet.
    if (!_hasWindowClassRegistered) {
        // Register window class.
        WNDCLASS wc = { 0 };
        wc.lpfnWndProc = &::celerique::win32::internal::Window::WindowProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = celeriqueWindowClassName;
        if (RegisterClass(&wc) == 0) {
            const char* errorMessage = "Failed to register window class.";
            celeriqueLogFatal(errorMessage);
            // Halt execution and throw an exception.
            throw ::std::runtime_error(errorMessage);
        }
        _hasWindowClassRegistered = true;
    }

    // Create the window.
    _windowHandle = reinterpret_cast<::celerique::Pointer>(CreateWindowEx(
        0, celeriqueWindowClassName, title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, nullptr
    ));
    if (_windowHandle == 0) {
        const char* errorMessage = "Failed to create Win32 window.";
        celeriqueLogFatal(errorMessage);
        // Halt execution and throw an exception.
        throw ::std::runtime_error(errorMessage);
    }
    _uiProtocol = CELERIQUE_UI_PROTOCOL_WIN32;

    // Set the pointer to this implementation instance as the `_windowHandle`'s
    // user data long pointer to enable window procedure
    // functions to retrieve this instance.
    SetWindowLongPtr(reinterpret_cast<HWND>(_windowHandle), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Adjust the window size to include the client area.
    RECT clientRect = {
        0, 0, static_cast<LONG>(defaultWidth), static_cast<LONG>(defaultHeight)
    };
    AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
    // Resize the window.
    SetWindowPos(
        reinterpret_cast<HWND>(_windowHandle), NULL, 0, 0, clientRect.right - clientRect.left,
        clientRect.bottom - clientRect.top, SWP_NOMOVE | SWP_NOZORDER
    );

    ShowWindow(reinterpret_cast<HWND>(_windowHandle), SW_SHOW);
    UpdateWindow(reinterpret_cast<HWND>(_windowHandle));

    celeriqueLogDebug("Created a win32 window.");
}

/// @brief Updates the state.
/// @param ptrArg The shared pointer to the update data container.
void ::celerique::win32::internal::Window::onUpdate(::std::shared_ptr<IUpdateData> ptrUpdateData) {
    // The container for the message.
    MSG message = {0};
    // If there is a message, translate and dispatch.
    if (GetMessage(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

/// @brief Converts the key codes (in `WPARAM`) from the
/// win32 api to this framework's key code.
/// @param wParam The keyboard key code value in `WPARAM`.
/// @return The corresponding `CeleriqueKeyCode` value.
CeleriqueKeyCode celerique::win32::internal::Window::win32WParamToCeleriqueKeyCode(WPARAM wParam) {
    switch (wParam) {
    case '0':
        return CELERIQUE_KEYBOARD_KEY_0;
    case '1':
        return CELERIQUE_KEYBOARD_KEY_1;
    case '2':
        return CELERIQUE_KEYBOARD_KEY_2;
    case '3':
        return CELERIQUE_KEYBOARD_KEY_3;
    case '4':
        return CELERIQUE_KEYBOARD_KEY_4;
    case '5':
        return CELERIQUE_KEYBOARD_KEY_5;
    case '6':
        return CELERIQUE_KEYBOARD_KEY_6;
    case '7':
        return CELERIQUE_KEYBOARD_KEY_7;
    case '8':
        return CELERIQUE_KEYBOARD_KEY_8;
    case '9':
        return CELERIQUE_KEYBOARD_KEY_9;

    case VK_NUMPAD0:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_0;
    case VK_NUMPAD1:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_1;
    case VK_NUMPAD2:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_2;
    case VK_NUMPAD3:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_3;
    case VK_NUMPAD4:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_4;
    case VK_NUMPAD5:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_5;
    case VK_NUMPAD6:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_6;
    case VK_NUMPAD7:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_7;
    case VK_NUMPAD8:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_8;
    case VK_NUMPAD9:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_9;

    case 'A':
        return CELERIQUE_KEYBOARD_KEY_A;
    case 'B':
        return CELERIQUE_KEYBOARD_KEY_B;
    case 'C':
        return CELERIQUE_KEYBOARD_KEY_C;
    case 'D':
        return CELERIQUE_KEYBOARD_KEY_D;
    case 'E':
        return CELERIQUE_KEYBOARD_KEY_E;
    case 'F':
        return CELERIQUE_KEYBOARD_KEY_F;
    case 'G':
        return CELERIQUE_KEYBOARD_KEY_G;
    case 'H':
        return CELERIQUE_KEYBOARD_KEY_H;
    case 'I':
        return CELERIQUE_KEYBOARD_KEY_I;
    case 'J':
        return CELERIQUE_KEYBOARD_KEY_J;
    case 'K':
        return CELERIQUE_KEYBOARD_KEY_K;
    case 'L':
        return CELERIQUE_KEYBOARD_KEY_L;
    case 'M':
        return CELERIQUE_KEYBOARD_KEY_M;
    case 'N':
        return CELERIQUE_KEYBOARD_KEY_N;
    case 'O':
        return CELERIQUE_KEYBOARD_KEY_O;
    case 'P':
        return CELERIQUE_KEYBOARD_KEY_P;
    case 'Q':
        return CELERIQUE_KEYBOARD_KEY_Q;
    case 'R':
        return CELERIQUE_KEYBOARD_KEY_R;
    case 'S':
        return CELERIQUE_KEYBOARD_KEY_S;
    case 'T':
        return CELERIQUE_KEYBOARD_KEY_T;
    case 'U':
        return CELERIQUE_KEYBOARD_KEY_U;
    case 'V':
        return CELERIQUE_KEYBOARD_KEY_V;
    case 'W':
        return CELERIQUE_KEYBOARD_KEY_W;
    case 'X':
        return CELERIQUE_KEYBOARD_KEY_X;
    case 'Y':
        return CELERIQUE_KEYBOARD_KEY_Y;
    case 'Z':
        return CELERIQUE_KEYBOARD_KEY_Z;

    case VK_ESCAPE:
        return CELERIQUE_KEYBOARD_KEY_ESC;
    case VK_TAB:
        return CELERIQUE_KEYBOARD_KEY_TAB;
    case VK_CAPITAL:
        return CELERIQUE_KEYBOARD_KEY_CAPS_LOCK;
    case VK_SHIFT:
        return CELERIQUE_KEYBOARD_KEY_LEFT_SHIFT;
    case VK_CONTROL:
        return CELERIQUE_KEYBOARD_KEY_LEFT_CONTROL;
    case VK_SPACE:
        return CELERIQUE_KEYBOARD_KEY_SPACE_BAR;
    case VK_RETURN:
        return CELERIQUE_KEYBOARD_KEY_ENTER;
    case VK_BACK:
        return CELERIQUE_KEYBOARD_KEY_BACKSPACE;
    case VK_DELETE:
        return CELERIQUE_KEYBOARD_KEY_DELETE;

    case VK_UP:
        return CELERIQUE_KEYBOARD_KEY_UP;
    case VK_DOWN:
        return CELERIQUE_KEYBOARD_KEY_DOWN;
    case VK_LEFT:
        return CELERIQUE_KEYBOARD_KEY_LEFT;
    case VK_RIGHT:
        return CELERIQUE_KEYBOARD_KEY_RIGHT;

    case VK_F1:
        return CELERIQUE_KEYBOARD_KEY_F1;
    case VK_F2:
        return CELERIQUE_KEYBOARD_KEY_F2;
    case VK_F3:
        return CELERIQUE_KEYBOARD_KEY_F3;
    case VK_F4:
        return CELERIQUE_KEYBOARD_KEY_F4;
    case VK_F5:
        return CELERIQUE_KEYBOARD_KEY_F5;
    case VK_F6:
        return CELERIQUE_KEYBOARD_KEY_F6;
    case VK_F7:
        return CELERIQUE_KEYBOARD_KEY_F7;
    case VK_F8:
        return CELERIQUE_KEYBOARD_KEY_F8;
    case VK_F9:
        return CELERIQUE_KEYBOARD_KEY_F9;
    case VK_F10:
        return CELERIQUE_KEYBOARD_KEY_F10;
    case VK_F11:
        return CELERIQUE_KEYBOARD_KEY_F11;
    case VK_F12:
        return CELERIQUE_KEYBOARD_KEY_F12;

    default:
        celeriqueLogDebug(
            "No key mapping exists for win32 key code value '" +
            ::std::to_string(static_cast<unsigned int>(wParam)) + "' to this framework."
        );
        return CELERIQUE_KEYBOARD_KEY_NULL;
    }
}

/// @brief The event handler for the win32 window.
/// @param windowHandle The handle to the window.
/// @param uMessage The message code.
/// @param wParam The word parameter.
/// @param lParam The long parameter.
/// @return Procedure result to be returned to the win32 api.
LRESULT CALLBACK ::celerique::win32::internal::Window::WindowProc(
    HWND windowHandle, UINT uMessage, WPARAM wParam, LPARAM lParam
) {
    // Extract the implementation instance pointer.
    Window* ptrWindow = reinterpret_cast<Window*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
    // If null, route to the default window procedure.
    if (ptrWindow == nullptr) {
        if (uMessage != WM_GETMINMAXINFO && uMessage != WM_NCCREATE &&
        uMessage != WM_NCCALCSIZE && uMessage != WM_CREATE) {
            celeriqueLogWarning(
                "Failed to retrieve the implementation pointer. Please make sure that "
                "`SetWindowLongPtr` has been called and the user data pointer to "
                "has been set to the implementation instance."
            );
        }
        return DefWindowProc(windowHandle, uMessage, wParam, lParam);
    }

    switch (uMessage) {
    case WM_CLOSE: {
        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::WindowRequestClose>(),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_DESTROY: {
        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::WindowClose>(),
            CELERIQUE_EVENT_HANDLING_STRATEGY_BLOCKING
        );
    } return 0;

    case WM_SETFOCUS: {
        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::WindowFocused>(),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_SYSCOMMAND: {
        if (wParam == SC_MINIMIZE) {
            ptrWindow->broadcast(
                ::std::make_shared<::celerique::event::WindowMinimized>(),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        }
    } return DefWindowProc(windowHandle, uMessage, wParam, lParam);

    case WM_MOVE: {
        if (!IsIconic(windowHandle)) { // If window is not minimized.
            // The horizontal position coordinate of the window in the screen.
            const PixelUnits xPos = LOWORD(lParam);
            // The vertical position coordinate of the window in the screen.
            const PixelUnits yPos = HIWORD(lParam);

            ptrWindow->broadcast(
                ::std::make_shared<::celerique::event::WindowMove>(xPos, yPos),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        }
    } return 0;

    case WM_SIZE: {
        if (!IsIconic(windowHandle)) { // If window is not minimized.
            const PixelUnits width = LOWORD(lParam);
            const PixelUnits height = HIWORD(lParam);

            ptrWindow->broadcast(
                ::std::make_shared<::celerique::event::WindowResize>(width, height),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        }
    } return 0;

    case WM_MOUSEMOVE: {
        // The amount of offset in the horizontal dimension.
        const PixelUnits deltaX = static_cast<PixelUnits>(LOWORD(lParam))
            - ptrWindow->_atomicRecentMouseXPos.load();
        // The amount of offset in the vertical dimension.
        const PixelUnits deltaY = static_cast<PixelUnits>(HIWORD(lParam))
            - ptrWindow->_atomicRecentMouseYPos.load();

        // Halt from here on as the mouse pointer didn't move.
        if (deltaX == 0 || deltaY == 0) return 0;

        // The mouse pointer was recently outside the viewport.
        if (!ptrWindow->_atomicMousePointerTracking.load()) {
            // Record mouse positions.
            ptrWindow->_atomicRecentMouseXPos.store(static_cast<PixelUnits>(LOWORD(lParam)));
            ptrWindow->_atomicRecentMouseYPos.store(static_cast<PixelUnits>(HIWORD(lParam)));
            // Start tracking mouse pointer.
            ptrWindow->_atomicMousePointerTracking.store(true);

            // Halt from here on.
            return 0;
        }

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::MouseMoved>(deltaX, deltaY),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );

        // Update recent data.
        ptrWindow->_atomicRecentMouseXPos.store(static_cast<PixelUnits>(LOWORD(lParam)));
        ptrWindow->_atomicRecentMouseYPos.store(static_cast<PixelUnits>(HIWORD(lParam)));
    } return 0;

    case WM_MOUSELEAVE: {
        // Stop tracking mouse pointer.
        ptrWindow->_atomicMousePointerTracking.store(false);
    } return 0;

    case WM_LBUTTONDOWN: {
        // The horizontal position coordinate of the mouse in the window.
        const PixelUnits xPos = static_cast<PixelUnits>(LOWORD(lParam));
        // The vertical position coordinate of the mouse in the window.
        const PixelUnits yPos = static_cast<PixelUnits>(HIWORD(lParam));

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::MouseClicked>(
                CELERIQUE_MOUSE_BUTTON_LEFT, xPos, yPos
            ),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_LBUTTONUP: {
        // The horizontal position coordinate of the mouse in the window.
        const PixelUnits xPos = static_cast<PixelUnits>(LOWORD(lParam));
        // The vertical position coordinate of the mouse in the window.
        const PixelUnits yPos = static_cast<PixelUnits>(HIWORD(lParam));

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::MouseReleased>(
                CELERIQUE_MOUSE_BUTTON_LEFT, xPos, yPos
            ),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_RBUTTONDOWN: {
        // The horizontal position coordinate of the mouse in the window.
        const PixelUnits xPos = static_cast<PixelUnits>(LOWORD(lParam));
        // The vertical position coordinate of the mouse in the window.
        const PixelUnits yPos = static_cast<PixelUnits>(HIWORD(lParam));

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::MouseClicked>(
                CELERIQUE_MOUSE_BUTTON_RIGHT, xPos, yPos
            ),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_RBUTTONUP: {
        // The horizontal position coordinate of the mouse in the window.
        const PixelUnits xPos = static_cast<PixelUnits>(LOWORD(lParam));
        // The vertical position coordinate of the mouse in the window.
        const PixelUnits yPos = static_cast<PixelUnits>(HIWORD(lParam));

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::MouseReleased>(
                CELERIQUE_MOUSE_BUTTON_RIGHT, xPos, yPos
            ),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_MBUTTONDOWN: {
        // The horizontal position coordinate of the mouse in the window.
        const PixelUnits xPos = static_cast<PixelUnits>(LOWORD(lParam));
        // The vertical position coordinate of the mouse in the window.
        const PixelUnits yPos = static_cast<PixelUnits>(HIWORD(lParam));

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::MouseClicked>(
                CELERIQUE_MOUSE_BUTTON_SCROLL, xPos, yPos
            ),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_MBUTTONUP: {
        // The horizontal position coordinate of the mouse in the window.
        const PixelUnits xPos = static_cast<PixelUnits>(LOWORD(lParam));
        // The vertical position coordinate of the mouse in the window.
        const PixelUnits yPos = static_cast<PixelUnits>(HIWORD(lParam));

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::MouseReleased>(
                CELERIQUE_MOUSE_BUTTON_SCROLL, xPos, yPos
            ),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_MOUSEWHEEL: {
        // The amount of offset in the vertical dimension.
        const float deltaY = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / (-120.0);

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::MouseScrolled>(0.0f, deltaY),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_MOUSEHWHEEL: {
        // The amount of offset in the horizontal dimension.
        const float deltaX = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / (120.0);

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::MouseScrolled>(deltaX, 0.0f),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_KEYDOWN: {
        // Parse the keyboard key code.
        const CeleriqueKeyCode keyCode = win32WParamToCeleriqueKeyCode(wParam);
        if (keyCode == CELERIQUE_KEYBOARD_KEY_NULL) return 0;

        // TODO: Calculate whether the key pressed is repeating.

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::KeyboardKeyPressed>(keyCode),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    case WM_KEYUP: {
        // Parse the keyboard key code.
        const CeleriqueKeyCode keyCode = win32WParamToCeleriqueKeyCode(wParam);
        if (keyCode == CELERIQUE_KEYBOARD_KEY_NULL) return 0;

        ptrWindow->broadcast(
            ::std::make_shared<::celerique::event::KeyboardKeyReleased>(keyCode),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return 0;

    default:
        // For all other messages, route back to the default window procedure.
        return DefWindowProc(windowHandle, uMessage, wParam, lParam);
    }
}

/// @brief Destructor.
::celerique::win32::internal::Window::~Window() {
    if (_windowHandle != 0) {
        DestroyWindow(reinterpret_cast<HWND>(_windowHandle));
    }

    celeriqueLogDebug("Win32 window destroyed.");
}