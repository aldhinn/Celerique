/*

File: ./vulkan/src/graphics.cpp
Author: Aldhinn Espinas
Description: This source file contains internal implementations of vulkan graphics interfaces.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/vulkan/api.h>
#include <celerique/vulkan/internal/graphics.h>

#include <celerique/logging.h>

#include <stdexcept>
#include <utility>

/// @brief Gets the interface to the vulkan graphics API.
/// @return The shared pointer to the vulkan graphics API interface.
::std::shared_ptr<::celerique::IGraphicsAPI> celerique::vulkan::getGraphicsApiInterface() {
    return ::celerique::vulkan::internal::GraphicsAPI::get();
}

/// @brief Gets the interface to the vulkan resources functionalities.
/// @return The shared pointer to the vulkan resources interface.
::std::shared_ptr<::celerique::IGpuResources> celerique::vulkan::getGpuResourcesInterface() {
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
/// @param graphicsPipelineConfig The graphics pipeline configuration.
/// @return The unique identifier to the graphics pipeline configuration that was just added.
::celerique::PipelineConfigID celerique::vulkan::internal::GraphicsAPI::addGraphicsPipelineConfig(
    const PipelineConfig& graphicsPipelineConfig
) {
    PipelineConfigID currentId = genPipelineConfigID();
    refManager.addGraphicsPipeline(graphicsPipelineConfig, currentId);
    return currentId;
}

/// @brief Remove the graphics pipeline configuration specified.
/// @param graphicsPipelineConfigId The identifier of the graphics pipeline configuration to be removed.
void ::celerique::vulkan::internal::GraphicsAPI::removeGraphicsPipelineConfig(PipelineConfigID graphicsPipelineConfigId) {
    refManager.removeGraphicsPipeline(graphicsPipelineConfigId);
}

/// @brief Clear the collection of graphics pipeline configurations.
void ::celerique::vulkan::internal::GraphicsAPI::clearGraphicsPipelineConfigs() {
    refManager.clearGraphicsPipelines();
}

/// @brief Update the values of the uniform of a graphics pipeline.
/// @param graphicsPipelineConfigId The unique identifier to the graphics pipeline configuration.
/// @param bindingPoint The binding point of the uniform.
/// @param ptrData The pointer to the buffer containing the new data.
/// @param sizeData The size of the new data to be passed.
void celerique::vulkan::internal::GraphicsAPI::updateUniform(
    PipelineConfigID graphicsPipelineConfigId, size_t bindingPoint,
    void* ptrData, size_t sizeData
) {
    // TODO: Implement.
}

/// @brief Graphics draw call.
/// @param graphicsPipelineConfigId The identifier for the graphics pipeline configuration to be used for drawing.
/// @param numVerticesToDraw The number of vertices to be drawn.
/// @param vertexStride The size of the individual vertex input.
/// @param numVertexElements The number of individual vertices to draw.
/// @param ptrVertexBuffer The pointer to the vertex buffer.
/// @param ptrIndexBuffer The pointer to the index buffer.
void ::celerique::vulkan::internal::GraphicsAPI::draw(
    PipelineConfigID graphicsPipelineConfigId, size_t numVerticesToDraw, size_t vertexStride,
    size_t numVertexElements, void* ptrVertexBuffer, uint32_t* ptrIndexBuffer
) {
    refManager.draw(graphicsPipelineConfigId, numVerticesToDraw, vertexStride, numVertexElements, ptrVertexBuffer, ptrIndexBuffer);
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

/// @brief Re-create the swapchain of the specified window.
/// @param windowHandle The handle to the window of which swapchain to re-create.
void ::celerique::vulkan::internal::GraphicsAPI::reCreateSwapChain(Pointer windowHandle) {
    refManager.reCreateSwapChain(windowHandle);
}

/// @brief The shared pointer to the singleton instance.
::std::shared_ptr<::celerique::vulkan::internal::GraphicsAPI> celerique::vulkan::internal::GraphicsAPI::_ptrInst = nullptr;

/// @brief Default constructor.
::celerique::vulkan::internal::GraphicsAPI::GraphicsAPI() {
    if (_ptrInst != nullptr) {
        const char* errorMessage = "There was an unauthorized graphics API interface instance.";
        celeriqueLogFatal(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    celeriqueLogTrace("Initialized interface to the vulkan graphics API.");
}