/*

File: ./include/celerique/x11/window.h
Author: Aldhinn Espinas
Description: This header file contains declarations wrapping around an x11 window.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_X11_WINDOW_HEADER_FILE)
#define CELERIQUE_X11_WINDOW_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/types.h>
#include <celerique/graphics.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <memory>

namespace celerique { namespace x11 {
    /// @brief The type for the number of pixel units in the screen.
    typedef CeleriquePixelUnits PixelUnits;

    /// @brief Create an x11 window.
    /// @param defaultWidth The default horizontal dimension of the window.
    /// @param defaultHeight The default vertical dimension of the window.
    /// @param title The title on the window's title bar.
    CELERIQUE_SHARED_SYMBOL ::std::unique_ptr<IWindow> createWindow(
        PixelUnits defaultWidth, PixelUnits defaultHeight, ::std::string&& title
    );
}}
#endif
// End C++ Only Region.

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.