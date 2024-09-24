/*

File: ./win32/tests/window.cpp
Author: Aldhinn Espinas
Description: This tests the wrapper for win32 API window.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/win32/window.h>
#include <celerique/logging.h>
#include <celerique/events/window.h>
#include <celerique/events/mouse.h>
#include <celerique/events/keyboard.h>

#include <stdlib.h>
#include <atomic>

/// @brief Test entry point.
/// @param argc The number of command line arguments.
/// @param argv The array of command line arguments in C string.
/// @return Exit code back to the operating system.
int main(int argc, char** argv) {
    celeriqueLogInfo("Started CeleriqueEngineWin32PluginTesting execution.");

    /// @brief State variable that determines whether we should keep running the loop.
    ::std::atomic<bool> isRunning = true;

    /// @brief The pointer to the graphical user interface window.
    ::std::unique_ptr<::celerique::WindowBase> ptrWindow = ::celerique::win32::createWindow(
        700, 400, "CeleriqueEngineWin32PluginTesting"
    );

    // Listen to UI events.
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::WindowRequestClose))) {
            isRunning.store(false);
            celeriqueLogDebug("WindowRequestClose event was dispatched.");
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::WindowClose))) {
            isRunning.store(false);
            celeriqueLogDebug("WindowClose event was dispatched.");
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::WindowFocused))) {
            celeriqueLogDebug("WindowFocused event was dispatched.");
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::WindowMinimized))) {
            celeriqueLogDebug("WindowMinimized event was dispatched.");
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::WindowMove))) {
            ::celerique::event::WindowMove* ptrWindowMoveEvent = dynamic_cast<::celerique::event::WindowMove*>(ptrEvent.get());
            celeriqueLogTrace(
                "Window moved to (" + ::std::to_string(ptrWindowMoveEvent->xPos())
                + ", " + ::std::to_string(ptrWindowMoveEvent->yPos()) + ")"
            );
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::WindowResize))) {
            ::celerique::event::WindowResize* ptrWindowResizeEvent = dynamic_cast<::celerique::event::WindowResize*>(ptrEvent.get());
            celeriqueLogTrace(
                "Window resized into (" + ::std::to_string(ptrWindowResizeEvent->width())
                + ", " + ::std::to_string(ptrWindowResizeEvent->height()) + ")"
            );
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::MouseMoved))) {
            ::celerique::event::MouseMoved* ptrMouseMoveEvent = dynamic_cast<::celerique::event::MouseMoved*>(ptrEvent.get());
            celeriqueLogTrace(
                "Mouse moved: (" + ::std::to_string(ptrMouseMoveEvent->deltaX())
                + ", " + ::std::to_string(ptrMouseMoveEvent->deltaY()) + ")"
            );
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::MouseClicked))) {
            ::celerique::event::MouseClicked* ptrMouseClickedEvent = dynamic_cast<::celerique::event::MouseClicked*>(ptrEvent.get());
            switch (ptrMouseClickedEvent->button()) {
            case CELERIQUE_MOUSE_BUTTON_LEFT:
                celeriqueLogDebug(
                    "Left mouse clicked @ (" + ::std::to_string(ptrMouseClickedEvent->xPos())
                    + ", " + ::std::to_string(ptrMouseClickedEvent->yPos()) + ")"
                );
                break;
            case CELERIQUE_MOUSE_BUTTON_RIGHT:
                celeriqueLogDebug(
                    "Right mouse clicked @ (" + ::std::to_string(ptrMouseClickedEvent->xPos())
                    + ", " + ::std::to_string(ptrMouseClickedEvent->yPos()) + ")"
                );
                break;
            case CELERIQUE_MOUSE_BUTTON_SCROLL:
                celeriqueLogDebug(
                    "Scroll mouse clicked @ (" + ::std::to_string(ptrMouseClickedEvent->xPos())
                    + ", " + ::std::to_string(ptrMouseClickedEvent->yPos()) + ")"
                );
                break;
            default:
                celeriqueLogWarning(
                    "Mouse clicked @ (" + ::std::to_string(ptrMouseClickedEvent->xPos())
                    + ", " + ::std::to_string(ptrMouseClickedEvent->yPos()) + ") with "
                    "unsupported mouse key code: " + ::std::to_string(ptrMouseClickedEvent->button())
                );
            }
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::MouseReleased))) {
            ::celerique::event::MouseReleased* ptrMouseReleasedEvent = dynamic_cast<::celerique::event::MouseReleased*>(ptrEvent.get());
            switch (ptrMouseReleasedEvent->button()) {
            case CELERIQUE_MOUSE_BUTTON_LEFT:
                celeriqueLogDebug(
                    "Left mouse released @ (" + ::std::to_string(ptrMouseReleasedEvent->xPos())
                    + ", " + ::std::to_string(ptrMouseReleasedEvent->yPos()) + ")"
                );
                break;
            case CELERIQUE_MOUSE_BUTTON_RIGHT:
                celeriqueLogDebug(
                    "Right mouse released @ (" + ::std::to_string(ptrMouseReleasedEvent->xPos())
                    + ", " + ::std::to_string(ptrMouseReleasedEvent->yPos()) + ")"
                );
                break;
            case CELERIQUE_MOUSE_BUTTON_SCROLL:
                celeriqueLogDebug(
                    "Scroll mouse released @ (" + ::std::to_string(ptrMouseReleasedEvent->xPos())
                    + ", " + ::std::to_string(ptrMouseReleasedEvent->yPos()) + ")"
                );
                break;
            default:
                celeriqueLogWarning(
                    "Mouse released @ (" + ::std::to_string(ptrMouseReleasedEvent->xPos())
                    + ", " + ::std::to_string(ptrMouseReleasedEvent->yPos()) + ") with "
                    "unsupported mouse key code: " + ::std::to_string(ptrMouseReleasedEvent->button())
                );
            }
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::MouseScrolled))) {
            ::celerique::event::MouseScrolled* ptrMouseScrolledEvent = dynamic_cast<::celerique::event::MouseScrolled*>(ptrEvent.get());
            celeriqueLogTrace(
                "Mouse scrolled: (" + ::std::to_string(ptrMouseScrolledEvent->deltaX())
                + ", " + ::std::to_string(ptrMouseScrolledEvent->deltaY()) + ")"
            );
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::KeyboardKeyPressed))) {
            ::celerique::event::KeyboardKeyPressed* ptrKeyboardKeyPressedEvent = dynamic_cast<::celerique::event::KeyboardKeyPressed*>(ptrEvent.get());
            celeriqueLogDebug(
                "Keyboard key pressed: " + ::std::to_string(ptrKeyboardKeyPressedEvent->keyCode())
            );
        }
    });
    ptrWindow->addEventListener([&](::std::shared_ptr<::celerique::EventBase> ptrEvent) {
        if (ptrEvent->typeID() == ::std::type_index(typeid(::celerique::event::KeyboardKeyReleased))) {
            ::celerique::event::KeyboardKeyReleased* ptrKeyboardKeyReleasedEvent = dynamic_cast<::celerique::event::KeyboardKeyReleased*>(ptrEvent.get());
            celeriqueLogDebug(
                "Keyboard key pressed: " + ::std::to_string(ptrKeyboardKeyReleasedEvent->keyCode())
            );
        }
    });

    // Application loop.
    while(isRunning.load()) {
        ptrWindow->onUpdate();
    }

    celeriqueLogInfo("Ending CeleriqueEngineWin32PluginTesting execution.");

    return EXIT_SUCCESS;
}