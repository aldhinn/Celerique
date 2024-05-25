/*

File: ./vulkan/src/graphics.cpp
Author: Aldhinn Espinas
Description: This source file contains internal implementations of vulkan graphics interfaces.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/vulkan/graphics.h>
#include <celerique/vulkan/internal/graphics.h>

#include <celerique/logging.h>

#include <stdexcept>
#include <utility>

/// @brief Gets the interface to the vulkan graphics API.
/// @return The shared pointer to the vulkan graphics API interface.
::std::shared_ptr<::celerique::IGraphicsAPI> celerique::vulkan::getGraphicsApiInterface() {
    return ::celerique::vulkan::internal::GraphicsAPI::get();
}

/// @brief Gets the singleton instance.
/// @return The singleton instance shared pointer.
::std::shared_ptr<::celerique::vulkan::internal::GraphicsAPI> celerique::vulkan::internal::GraphicsAPI::get() {
    if (_ptrInst == nullptr) {
        _ptrInst = ::std::make_shared<internal::GraphicsAPI>();
    }
    return _ptrInst;
}

/// @brief Add a graphics pipeline configuration.
/// @param ptrGraphicsPipelineConfig The unique pointer to the graphics pipeline configuration.
/// @return The unique identifier to the graphics pipeline configuration that was just added.
::celerique::PipelineConfigID celerique::vulkan::internal::GraphicsAPI::addGraphicsPipelineConfig(
    ::std::unique_ptr<PipelineConfig>&& ptrGraphicsPipelineConfig
) {
    refManager.addGraphicsPipeline(ptrGraphicsPipelineConfig.get());
    return IGraphicsAPI::addGraphicsPipelineConfig(::std::move(ptrGraphicsPipelineConfig));
}

/// @brief Add the window handle to the graphics API.
/// @param uiProtocol The UI protocol used to create UI elements.
/// @param windowHandle The handle to the window according to UI protocol.
void ::celerique::vulkan::internal::GraphicsAPI::addWindow(UiProtocol uiProtocol, Pointer windowHandle)  {
    refManager.addWindow(uiProtocol, windowHandle);
}

/// @brief Remove the window handle from the graphics API registry.
/// @param windowHandle The handle to the window according to UI protocol.
void ::celerique::vulkan::internal::GraphicsAPI::removeWindow(Pointer windowHandle) {
    refManager.removeWindow(windowHandle);
}

/// @brief The shared pointer to the singleton instance.
::std::shared_ptr<::celerique::vulkan::internal::GraphicsAPI> celerique::vulkan::internal::GraphicsAPI::_ptrInst = nullptr;

/// @brief Default constructor.
::celerique::vulkan::internal::GraphicsAPI::GraphicsAPI() : refManager(Manager::getRef()) {
    if (_ptrInst != nullptr) {
        const char* errorMessage = "There was an unauthorized graphics API interface instance.";
        celeriqueLogFatal(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    celeriqueLogTrace("Initialized interface to the vulkan graphics API.");
}