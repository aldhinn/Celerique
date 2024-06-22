/*

File: ./vulkan/include/celerique/vulkan/internal/graphics.h
Author: Aldhinn Espinas
Description: This header file contains internal interfaces to vulkan graphics related objects.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_VULKAN_INTERNAL_GRAPHICS_HEADER_FILE)
#define CELERIQUE_VULKAN_INTERNAL_GRAPHICS_HEADER_FILE

#include <celerique/graphics.h>
#include <celerique/vulkan/internal/manager.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <memory>

namespace celerique { namespace vulkan { namespace internal {
    /// @brief The interface to the vulkan graphics API.
    class GraphicsAPI final : public IGraphicsAPI {
    public:
        /// @brief Gets the singleton instance.
        /// @return The singleton instance shared pointer.
        static ::std::shared_ptr<GraphicsAPI> get();

        /// @brief Add a graphics pipeline configuration.
        /// @param graphicsPipelineConfig The graphics pipeline configuration.
        /// @return The unique identifier to the graphics pipeline configuration that was just added.
        PipelineConfigID addGraphicsPipelineConfig(PipelineConfig&& graphicsPipelineConfig) override;
        /// @brief Remove the graphics pipeline configuration specified.
        /// @param graphicsPipelineConfigId The identifier of the graphics pipeline configuration to be removed.
        void removeGraphicsPipelineConfig(PipelineConfigID graphicsPipelineConfigId) override;
        /// @brief Clear the collection of graphics pipeline configurations.
        void clearGraphicsPipelineConfigs() override;

        /// @brief Graphics draw call.
        /// @param graphicsPipelineConfigId The identifier for the graphics pipeline configuration to be used for drawing.
        /// @param numVerticesToDraw The number of vertices to be drawn.
        /// @param vertexStride The size of the individual vertex input.
        /// @param numVertexElements The number of individual vertices to draw.
        /// @param ptrVertexBuffer The pointer to the vertex buffer.
        /// @param ptrIndexBuffer The pointer to the index buffer.
        void draw(
            PipelineConfigID graphicsPipelineConfigId, size_t numVerticesToDraw, size_t vertexStride = 0,
            size_t numVertexElements = 0, void* ptrVertexBuffer = nullptr, uint32_t* ptrIndexBuffer = nullptr
        ) override;

        /// @brief Add the window handle to the graphics API.
        /// @param uiProtocol The UI protocol used to create UI elements.
        /// @param windowHandle The handle to the window according to UI protocol.
        void addWindow(UiProtocol uiProtocol, Pointer windowHandle) override;
        /// @brief Remove the window handle from the graphics API registry.
        /// @param windowHandle The handle to the window according to UI protocol.
        void removeWindow(Pointer windowHandle) override;

    private:
        /// @brief The shared pointer to the singleton instance.
        static ::std::shared_ptr<internal::GraphicsAPI> _ptrInst;
        /// @brief The reference to the vulkan resource manager.
        Manager& refManager;

    // Constructors.
    public:
        /// @brief Default constructor.
        GraphicsAPI();

        /// @brief Prevent copying.
        GraphicsAPI(const GraphicsAPI&) = delete;
        /// @brief Prevent moving.
        GraphicsAPI(GraphicsAPI&&) = delete;
        /// @brief Prevent copy re-assignment.
        GraphicsAPI& operator=(const GraphicsAPI&) = delete;
        /// @brief Prevent move re-assignment.
        GraphicsAPI& operator=(GraphicsAPI&&) = delete;
    };
}}}
#endif
// End C++ Only Region.
#endif
// End of file.
// DO NOT WRITE BEYOND HERE.