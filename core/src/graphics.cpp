/*

File: ./core/src/graphics.cpp
Author: Aldhinn Espinas
Description: This source file contains implementations of the graphics functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/graphics.h>

void ::celerique::IWindow::useGraphicsApi(::std::shared_ptr<IGraphicsAPI> ptrGraphicsApi) {
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
::celerique::IWindow::~IWindow() {
    ::std::shared_ptr<IGraphicsAPI> ptrPrevGraphicsApi = _weakPtrGraphicsApi.lock();
    if (ptrPrevGraphicsApi != nullptr) {
        // Remove this window from a graphics API that this was registered to.
        ptrPrevGraphicsApi->removeWindow(_windowHandle);
    }
}

/// @brief Clear the collection of graphics pipeline configurations.
void ::celerique::IGraphicsAPI::clearGraphicsPipelineConfigs() {
    _nextGraphicsPipelineConfigId = 0;
}

/// @brief Pure virtual destructor.
::celerique::IGraphicsAPI::~IGraphicsAPI() {}