/*

File: ./core/src/graphics.cpp
Author: Aldhinn Espinas
Description: This source file contains implementations of the graphics functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/graphics.h>

/// @brief Virtual destructor.
::celerique::IWindow::~IWindow() {
    ::std::shared_ptr<IGraphicsAPI> ptrPrevGraphicsApi = _weakPtrGraphicsApi.lock();
    if (ptrPrevGraphicsApi != nullptr) {
        // Remove this window from a graphics API that this was registered to.
        ptrPrevGraphicsApi->removeWindow(_windowHandle);
    }
}

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

/// @brief Add a graphics pipeline configuration.
/// @param ptrGraphicsPipelineConfig The unique pointer to the graphics pipeline configuration.
/// @return The unique identifier to the graphics pipeline configuration that was just added.
::celerique::PipelineConfigID celerique::IGraphicsAPI::addGraphicsPipelineConfig(
    ::std::unique_ptr<PipelineConfig>&& ptrGraphicsPipelineConfig
) {
    // Add the graphics pipeline.
    _vecPtrGraphicsPipelineConfig.emplace_back(::std::move(ptrGraphicsPipelineConfig));
    return _nextGraphicsPipelineConfigId++;
}

/// @brief Set the graphics pipeline configuration to be used for graphics rendering.
/// @param graphicsPipelineConfigId The specified identifier of the graphics pipeline config.
void celerique::IGraphicsAPI::setGraphicsPipelineConfig(PipelineConfigID graphicsPipelineConfigId) {
    _currentGraphicsPipelineConfigUsed = graphicsPipelineConfigId;
}

/// @brief Reset the graphics pipeline configs vector.
void celerique::IGraphicsAPI::resetGraphicsPipelineConfigs() {
    _vecPtrGraphicsPipelineConfig.clear();
    _nextGraphicsPipelineConfigId = 0;
}

/// @brief Pure virtual destructor.
::celerique::IGraphicsAPI::~IGraphicsAPI() {}