/*

File: ./x11/src/window.cpp
Author: Aldhinn Espinas
Description: This source file contains internal implementation details wrapping around an x11 window.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/x11/window.h>
#include <celerique/x11/internal/window.h>

#include <celerique/logging.h>
#include <celerique/events/keyboard.h>
#include <celerique/events/mouse.h>
#include <celerique/events/window.h>

#include <utility>
#include <stdexcept>

#include <X11/Xatom.h>

::std::unique_ptr<::celerique::WindowBase> celerique::x11::createWindow(
    ::celerique::x11::PixelUnits defaultWidth,
    ::celerique::x11::PixelUnits defaultHeight,
    ::std::string&& title
) {
    using ::celerique::x11::internal::Window;
    return ::std::make_unique<Window>(defaultWidth, defaultHeight, ::std::move(title));
}

/// @brief Member init constructor.
/// @param defaultWidth The default horizontal dimension of the window.
/// @param defaultHeight The default vertical dimension of the window.
/// @param title The title on the window's title bar.
::celerique::x11::internal::Window::Window(
    PixelUnits defaultWidth, PixelUnits defaultHeight, ::std::string&& title
) : _ptrDisplay(XOpenDisplay(NULL)) {
    // Check if x11 display is open.
    if (_ptrDisplay == nullptr) {
        const char* errorMessage = "Unable to open X11 display.";
        celeriqueLogFatal(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief The screen number.
    int screenNum = DefaultScreen(_ptrDisplay);
    // Create the window.
    XID xWindowId = XCreateSimpleWindow(
        _ptrDisplay, RootWindow(_ptrDisplay, screenNum), 0, 0, defaultWidth,
        defaultHeight, 1, BlackPixel(_ptrDisplay, screenNum), BlackPixel(_ptrDisplay, screenNum)
    );
    if (xWindowId == 0) {
        XCloseDisplay(_ptrDisplay);
        _ptrDisplay = nullptr;

        const char* errorMessage = "Failed to create x11 window.";
        celeriqueLogFatal(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    _windowHandle = reinterpret_cast<Pointer>(xWindowId);
    _uiProtocol = CELERIQUE_UI_PROTOCOL_X11;

    // Track window size.
    _atomicRecentWindowWidth.store(defaultWidth, ::std::memory_order_release);
    _atomicRecentWindowHeight.store(defaultHeight, ::std::memory_order_release);

    // Set window title.
    XStoreName(_ptrDisplay, xWindowId, title.c_str());
    // Set x11 window to receive certain events.
    XSelectInput(
        _ptrDisplay, xWindowId,
        KeyPressMask | KeyReleaseMask | FocusChangeMask | PropertyChangeMask | PointerMotionMask |
        EnterWindowMask | LeaveWindowMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask
    );

    // Setup to handle window request close event.
    _atomWmDeleteMessage = XInternAtom(_ptrDisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(_ptrDisplay, xWindowId, &_atomWmDeleteMessage, 1);

    // Get atom value for `_NET_WM_STATE`.
    _atomNetWmState = XInternAtom(_ptrDisplay, "_NET_WM_STATE", False);
    // Get atom value for `_NET_WM_STATE_HIDDEN`.
    _atomNetWmStateHidden = XInternAtom(_ptrDisplay, "_NET_WM_STATE_HIDDEN", False);

    // Show window.
    XMapWindow(_ptrDisplay, xWindowId);
    // Ensure all commands are sent to the X server.
    XFlush(_ptrDisplay);

    celeriqueLogDebug("Created an x11 window.");
}

/// @brief Updates the state.
/// @param ptrArg The shared pointer to the update data container.
void ::celerique::x11::internal::Window::onUpdate(::std::shared_ptr<IUpdateData> ptrUpdateData) {
    /// @brief Container for the x11 event.
    XEvent x11Event = {};
    // Collect next event.
    XNextEvent(_ptrDisplay, &x11Event);

    switch(x11Event.type) {
    case ClientMessage: {
        // Checks if the client sent a delete window message request.
        if (static_cast<Atom>(x11Event.xclient.data.l[0]) == _atomWmDeleteMessage) {
            broadcast(
                ::std::make_shared<::celerique::event::WindowRequestClose>(),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        }
    } return;

    case ConfigureNotify: {
        /// @brief The configured horizontal position of the window.
        PixelUnits xPos = static_cast<PixelUnits>(x11Event.xconfigure.x);
        /// @brief The configured vertical position of the window.
        PixelUnits yPos = static_cast<PixelUnits>(x11Event.xconfigure.y);
        /// @brief The configured width of the window.
        PixelUnits width = static_cast<PixelUnits>(x11Event.xconfigure.width);
        /// @brief The configured height of the window.
        PixelUnits height = static_cast<PixelUnits>(x11Event.xconfigure.height);

        // Checking if the window moved.
        if (xPos != _atomicRecentWindowXPos.load() || yPos != _atomicRecentWindowYPos.load()) {
            broadcast(
                ::std::make_shared<::celerique::event::WindowMove>(xPos, yPos),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
            // Update window position.
            _atomicRecentWindowXPos.store(xPos, ::std::memory_order_release);
            _atomicRecentWindowYPos.store(yPos, ::std::memory_order_release);
        }
        // Checking if the window resized.
        if (width != _atomicRecentWindowWidth.load() || height != _atomicRecentWindowHeight.load()) {
            broadcast(
                ::std::make_shared<::celerique::event::WindowResize>(width, height),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
            // Update window sizes.
            _atomicRecentWindowWidth.store(width, ::std::memory_order_release);
            _atomicRecentWindowHeight.store(height, ::std::memory_order_release);

            ::std::thread reCreateSwapChainThread([&]() {
                /// @brief The retrieved shared pointer of this window's graphics API interface.
                ::std::shared_ptr<IGraphicsAPI> ptrGraphicsApi = _weakPtrGraphicsApi.lock();
                // Check if not null before re-creating swapchain.
                if (ptrGraphicsApi != nullptr) {
                    ptrGraphicsApi->reCreateSwapChain(_windowHandle);
                }
            });
            reCreateSwapChainThread.detach();
        }
    } return;

    case FocusIn: {
        broadcast(
            ::std::make_shared<::celerique::event::WindowFocused>(),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
        _atomicIsActive.store(true, ::std::memory_order_release);
    } return;

    case PropertyNotify: {
        if (x11Event.xproperty.atom == _atomNetWmState && _atomicIsActive.load()) {
            /// @brief Stores the actual type of the retrieved property.
            Atom actualType;
            /// @brief Stores the actual format of the retrieved property (8, 16, or 32 bits).
            int actualFormat;
            /// @brief Stores the number of items retrieved.
            size_t numItems;
            /// @brief Stores the number of bytes remaining after the retrieved data.
            size_t bytesAfter;
            /// @brief The pointer to an unsigned char that will point to the retrieved data.
            unsigned char* ptrProp = nullptr;

            if (XGetWindowProperty(
                _ptrDisplay, reinterpret_cast<XID>(_windowHandle), _atomNetWmState, 0, (~0L),
                False, XA_ATOM, &actualType, &actualFormat, &numItems, &bytesAfter, &ptrProp
            ) == Success) {
                for (size_t i = 0; ptrProp != nullptr && i < numItems; i++) {
                    if (reinterpret_cast<Atom*>(ptrProp)[i] == _atomNetWmStateHidden) {
                        broadcast(
                            ::std::make_shared<::celerique::event::WindowMinimized>(),
                            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
                        );
                        _atomicIsActive.store(false, ::std::memory_order_release);
                    }
                }
                XFree(ptrProp);
            }
        }
    } return;

    case MotionNotify: {
        // The amount of offset in the horizontal dimension.
        const PixelUnits deltaX = static_cast<PixelUnits>(x11Event.xmotion.x) - _atomicRecentMouseXPos.load();
        // The amount of offset in the vertical dimension.
        const PixelUnits deltaY = static_cast<PixelUnits>(x11Event.xmotion.x) - _atomicRecentMouseYPos.load();
         // Halt from here on as the mouse pointer didn't move.
        if (deltaX == 0 || deltaY == 0) return;

        // If the mouse hasn't been getting tracked.
        if (!_atomicMousePointerTracking.load()) {
            // Record mouse positions.
            _atomicRecentMouseXPos.store(static_cast<PixelUnits>(x11Event.xmotion.x), ::std::memory_order_release);
            _atomicRecentMouseYPos.store(static_cast<PixelUnits>(x11Event.xmotion.x), ::std::memory_order_release);
            // Start tracking mouse pointer.
            _atomicMousePointerTracking.store(true, ::std::memory_order_release);
            // Halt from here on.
            return;
        }

        broadcast(
            ::std::make_shared<::celerique::event::MouseMoved>(deltaX, deltaY),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
        // Update position.
        _atomicRecentMouseXPos.store(static_cast<PixelUnits>(x11Event.xmotion.x), ::std::memory_order_release);
        _atomicRecentMouseYPos.store(static_cast<PixelUnits>(x11Event.xmotion.x), ::std::memory_order_release);
    } return;

    case EnterNotify: {
        // Record mouse positions.
        _atomicRecentMouseXPos.store(static_cast<PixelUnits>(x11Event.xmotion.x), ::std::memory_order_release);
        _atomicRecentMouseYPos.store(static_cast<PixelUnits>(x11Event.xmotion.x), ::std::memory_order_release);
        // Start tracking mouse pointer.
        _atomicMousePointerTracking.store(true, ::std::memory_order_release);
    } return;

    case LeaveNotify: {
        _atomicMousePointerTracking.store(false, ::std::memory_order_release);
    } return;

    case ButtonPress: {
        switch(x11Event.xbutton.button) {
        case Button1: {
            broadcast(
                ::std::make_shared<::celerique::event::MouseClicked>(
                    CELERIQUE_MOUSE_BUTTON_LEFT, x11Event.xbutton.x, x11Event.xbutton.y
                ),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        } return;
        case Button2: {
            broadcast(
                ::std::make_shared<::celerique::event::MouseClicked>(
                    CELERIQUE_MOUSE_BUTTON_SCROLL, x11Event.xbutton.x, x11Event.xbutton.y
                ),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        } return;
        case Button3: {
            broadcast(
                ::std::make_shared<::celerique::event::MouseClicked>(
                    CELERIQUE_MOUSE_BUTTON_RIGHT, x11Event.xbutton.x, x11Event.xbutton.y
                ),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        } return;
        case 4: {
            broadcast(
                ::std::make_shared<::celerique::event::MouseScrolled>(0.0f, -0.5f),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        } return;
        case 5: {
            broadcast(
                ::std::make_shared<::celerique::event::MouseScrolled>(0.0f, 0.5f),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        } return;
        case 6: {
            broadcast(
                ::std::make_shared<::celerique::event::MouseScrolled>(-0.5f, 0.0f),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        } return;
        case 7: {
            broadcast(
                ::std::make_shared<::celerique::event::MouseScrolled>(0.5f, 0.0f),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        } return;
        case 8: { /* Do nothing. */ } return;
        case 9: { /* Do nothing. */ } return;

        default:
            celeriqueLogDebug("Unsupported mouse button code: " + ::std::to_string(x11Event.xbutton.button));
            return;
        }
    }

    case ButtonRelease: {
        switch(x11Event.xbutton.button) {
        case Button1: {
            broadcast(
                ::std::make_shared<::celerique::event::MouseClicked>(
                    CELERIQUE_MOUSE_BUTTON_LEFT, x11Event.xbutton.x, x11Event.xbutton.y
                ),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        } return;
        case Button2: {
            broadcast(
                ::std::make_shared<::celerique::event::MouseClicked>(
                    CELERIQUE_MOUSE_BUTTON_SCROLL, x11Event.xbutton.x, x11Event.xbutton.y
                ),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        } return;
        case Button3: {
            broadcast(
                ::std::make_shared<::celerique::event::MouseClicked>(
                    CELERIQUE_MOUSE_BUTTON_RIGHT, x11Event.xbutton.x, x11Event.xbutton.y
                ),
                CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
            );
        } return;
        case 4: { /* Do nothing. */ } return;
        case 5: { /* Do nothing. */ } return;
        case 6: { /* Do nothing. */ } return;
        case 7: { /* Do nothing. */ } return;
        case 8: { /* Do nothing. */ } return;
        case 9: { /* Do nothing. */ } return;

        default:
            celeriqueLogDebug("Unsupported mouse button code: " + ::std::to_string(x11Event.xbutton.button));
            return;
        }
    }

    case KeyPress: {
        CeleriqueKeyCode keyCode = x11KeyCodeToCeleriqueKeyCode(XLookupKeysym(&x11Event.xkey, 0));
        if (keyCode == CELERIQUE_KEYBOARD_KEY_NULL) return;

        // TODO: Calculate if repeating.
        broadcast(
            ::std::make_shared<::celerique::event::KeyboardKeyPressed>(keyCode),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return;

    case KeyRelease: {
        CeleriqueKeyCode keyCode = x11KeyCodeToCeleriqueKeyCode(XLookupKeysym(&x11Event.xkey, 0));
        if (keyCode == CELERIQUE_KEYBOARD_KEY_NULL) return;
        broadcast(
            ::std::make_shared<::celerique::event::KeyboardKeyReleased>(keyCode),
            CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC
        );
    } return;

    default:
        return;
    }
}

/// @brief Convert the x11 key code to the Celerique key codes.
/// @param x11KeySym The x11 key sym value.
/// @return The Celerique key code value.
CeleriqueKeyCode celerique::x11::internal::Window::x11KeyCodeToCeleriqueKeyCode(KeySym x11KeySym) {
    switch(x11KeySym) {
    case XK_0:
        return CELERIQUE_KEYBOARD_KEY_0;
    case XK_1:
        return CELERIQUE_KEYBOARD_KEY_1;
    case XK_2:
        return CELERIQUE_KEYBOARD_KEY_2;
    case XK_3:
        return CELERIQUE_KEYBOARD_KEY_3;
    case XK_4:
        return CELERIQUE_KEYBOARD_KEY_4;
    case XK_5:
        return CELERIQUE_KEYBOARD_KEY_5;
    case XK_6:
        return CELERIQUE_KEYBOARD_KEY_6;
    case XK_7:
        return CELERIQUE_KEYBOARD_KEY_7;
    case XK_8:
        return CELERIQUE_KEYBOARD_KEY_8;
    case XK_9:
        return CELERIQUE_KEYBOARD_KEY_9;

    case XK_KP_0:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_0;
    case XK_KP_1:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_1;
    case XK_KP_2:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_2;
    case XK_KP_3:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_3;
    case XK_KP_4:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_4;
    case XK_KP_5:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_5;
    case XK_KP_6:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_6;
    case XK_KP_7:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_7;
    case XK_KP_8:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_8;
    case XK_KP_9:
        return CELERIQUE_KEYBOARD_KEY_NUMPAD_9;

    case XK_a:
        return CELERIQUE_KEYBOARD_KEY_A;
    case XK_b:
        return CELERIQUE_KEYBOARD_KEY_B;
    case XK_c:
        return CELERIQUE_KEYBOARD_KEY_C;
    case XK_d:
        return CELERIQUE_KEYBOARD_KEY_D;
    case XK_e:
        return CELERIQUE_KEYBOARD_KEY_E;
    case XK_f:
        return CELERIQUE_KEYBOARD_KEY_F;
    case XK_g:
        return CELERIQUE_KEYBOARD_KEY_G;
    case XK_h:
        return CELERIQUE_KEYBOARD_KEY_H;
    case XK_i:
        return CELERIQUE_KEYBOARD_KEY_I;
    case XK_j:
        return CELERIQUE_KEYBOARD_KEY_J;
    case XK_k:
        return CELERIQUE_KEYBOARD_KEY_K;
    case XK_l:
        return CELERIQUE_KEYBOARD_KEY_L;
    case XK_m:
        return CELERIQUE_KEYBOARD_KEY_M;
    case XK_n:
        return CELERIQUE_KEYBOARD_KEY_N;
    case XK_o:
        return CELERIQUE_KEYBOARD_KEY_O;
    case XK_p:
        return CELERIQUE_KEYBOARD_KEY_P;
    case XK_q:
        return CELERIQUE_KEYBOARD_KEY_Q;
    case XK_r:
        return CELERIQUE_KEYBOARD_KEY_R;
    case XK_s:
        return CELERIQUE_KEYBOARD_KEY_S;
    case XK_t:
        return CELERIQUE_KEYBOARD_KEY_T;
    case XK_u:
        return CELERIQUE_KEYBOARD_KEY_U;
    case XK_v:
        return CELERIQUE_KEYBOARD_KEY_V;
    case XK_w:
        return CELERIQUE_KEYBOARD_KEY_W;
    case XK_x:
        return CELERIQUE_KEYBOARD_KEY_X;
    case XK_y:
        return CELERIQUE_KEYBOARD_KEY_Y;
    case XK_z:
        return CELERIQUE_KEYBOARD_KEY_Z;

    case XK_Escape:
        return CELERIQUE_KEYBOARD_KEY_ESC;
    case XK_Tab:
        return CELERIQUE_KEYBOARD_KEY_TAB;
    case XK_Caps_Lock:
        return CELERIQUE_KEYBOARD_KEY_CAPS_LOCK;
    case XK_Shift_L:
        return CELERIQUE_KEYBOARD_KEY_LEFT_SHIFT;
    case XK_Control_L:
        return CELERIQUE_KEYBOARD_KEY_LEFT_CONTROL;
    case XK_Alt_L:
        return CELERIQUE_KEYBOARD_KEY_LEFT_ALT;
    case XK_space:
        return CELERIQUE_KEYBOARD_KEY_SPACE_BAR;
    case XK_Alt_R:
        return CELERIQUE_KEYBOARD_KEY_RIGHT_ALT;
    case XK_Control_R:
        return CELERIQUE_KEYBOARD_KEY_RIGHT_CONTROL;
    case XK_Shift_R:
        return CELERIQUE_KEYBOARD_KEY_RIGHT_SHIFT;
    case XK_Return:
        return CELERIQUE_KEYBOARD_KEY_ENTER;
    case XK_BackSpace:
        return CELERIQUE_KEYBOARD_KEY_BACKSPACE;
    case XK_Delete:
        return CELERIQUE_KEYBOARD_KEY_DELETE;

    case XK_Up:
        return CELERIQUE_KEYBOARD_KEY_UP;
    case XK_Down:
        return CELERIQUE_KEYBOARD_KEY_DOWN;
    case XK_Left:
        return CELERIQUE_KEYBOARD_KEY_LEFT;
    case XK_Right:
        return CELERIQUE_KEYBOARD_KEY_RIGHT;

    case XK_F1:
        return CELERIQUE_KEYBOARD_KEY_F1;
    case XK_F2:
        return CELERIQUE_KEYBOARD_KEY_F2;
    case XK_F3:
        return CELERIQUE_KEYBOARD_KEY_F3;
    case XK_F4:
        return CELERIQUE_KEYBOARD_KEY_F4;
    case XK_F5:
        return CELERIQUE_KEYBOARD_KEY_F5;
    case XK_F6:
        return CELERIQUE_KEYBOARD_KEY_F6;
    case XK_F7:
        return CELERIQUE_KEYBOARD_KEY_F7;
    case XK_F8:
        return CELERIQUE_KEYBOARD_KEY_F8;
    case XK_F9:
        return CELERIQUE_KEYBOARD_KEY_F9;
    case XK_F10:
        return CELERIQUE_KEYBOARD_KEY_F10;
    case XK_F11:
        return CELERIQUE_KEYBOARD_KEY_F11;
    case XK_F12:
        return CELERIQUE_KEYBOARD_KEY_F12;

    default:
        celeriqueLogTrace("Unsupported x11 key sym code: " + ::std::to_string(x11KeySym));
        return CELERIQUE_KEYBOARD_KEY_NULL;
    }
}

/// @brief Destructor.
::celerique::x11::internal::Window::~Window() {
    if (_windowHandle != 0 && _ptrDisplay != nullptr) {
        XDestroyWindow(_ptrDisplay, reinterpret_cast<XID>(_windowHandle));
        broadcast(
            ::std::make_shared<::celerique::event::WindowClose>(),
            CELERIQUE_EVENT_HANDLING_STRATEGY_BLOCKING
        );
    }
    if (_ptrDisplay != nullptr) {
        XCloseDisplay(_ptrDisplay);
        _ptrDisplay = nullptr;
    }

    celeriqueLogDebug("X11 window destroyed.");
}