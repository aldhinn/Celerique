/*

File: ./core/src/graphics.cpp
Author: Aldhinn Espinas
Description: This source file contains implementations of the graphics functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/graphics.h>

void ::celerique::WindowBase::useGraphicsApi(::std::shared_ptr<IGraphicsAPI> ptrGraphicsApi) {
    ::std::shared_ptr<IGraphicsAPI> ptrPrevGraphicsApi = _weakPtrGraphicsApi.lock();
    if (ptrPrevGraphicsApi != nullptr) {
        // Remove this window from the graphics API that this was registered to.
        ptrPrevGraphicsApi->removeWindow(_windowHandle);
    }

    ptrGraphicsApi->addWindow(_uiProtocol, _windowHandle);

    // Keep reference to the current graphics API this window is using for rendering.
    _weakPtrGraphicsApi = ptrGraphicsApi;
}

/// @brief Virtual destructor.
::celerique::WindowBase::~WindowBase() {
    ::std::shared_ptr<IGraphicsAPI> ptrPrevGraphicsApi = _weakPtrGraphicsApi.lock();
    if (ptrPrevGraphicsApi != nullptr) {
        // Remove this window from a graphics API that this was registered to.
        ptrPrevGraphicsApi->removeWindow(_windowHandle);
    }
}

/// @brief Pure virtual destructor.
::celerique::IGraphicsAPI::~IGraphicsAPI() {}