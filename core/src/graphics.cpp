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

/// @brief Add a graphics pipeline configuration.
/// @param ptrGraphicsPipelineConfig The unique pointer to the graphics pipeline configuration.
/// @return The unique identifier to the graphics pipeline configuration that was just added.
::celerique::PipelineConfigID celerique::IGraphicsAPI::addGraphicsPipelineConfig(
    ::std::unique_ptr<PipelineConfig>&& ptrGraphicsPipelineConfig
) {
    /// @brief The current id of the pipeline config ID to be mapped.
    PipelineConfigID currentId = _nextGraphicsPipelineConfigId;
    // Add the graphics pipeline.
    _mapIdToPtrGraphicsPipelineConfig[currentId] = ::std::move(ptrGraphicsPipelineConfig);
    // Update next ID value.
    _nextGraphicsPipelineConfigId++;
    return currentId;
}

/// @brief Remove the graphics pipeline configuration specified.
/// @param graphicsPipelineConfigId The identifier of the graphics pipeline configuration to be removed.
void ::celerique::IGraphicsAPI::removeGraphicsPipelineConfig(PipelineConfigID graphicsPipelineConfigId) {
    /// @brief The iterator to the specified graphics pipeline configuration.
    auto iteratorGraphicsPipelineConfig = _mapIdToPtrGraphicsPipelineConfig.find(graphicsPipelineConfigId);
    // Remove if found.
    if (iteratorGraphicsPipelineConfig != _mapIdToPtrGraphicsPipelineConfig.end())
        _mapIdToPtrGraphicsPipelineConfig.erase(iteratorGraphicsPipelineConfig);
}

/// @brief Clear the collection of graphics pipeline configurations.
void ::celerique::IGraphicsAPI::clearGraphicsPipelineConfigs() {
    _mapIdToPtrGraphicsPipelineConfig.clear();
    _nextGraphicsPipelineConfigId = 0;
}

/// @brief Pure virtual destructor.
::celerique::IGraphicsAPI::~IGraphicsAPI() {}