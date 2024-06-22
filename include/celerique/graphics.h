/*

File: ./include/celerique/graphics.h
Author: Aldhinn Espinas
Description: This header file contains the graphics functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_GRAPHICS_HEADER_FILE)
#define CELERIQUE_GRAPHICS_HEADER_FILE

#include <celerique/abstracts.h>
#include <celerique/events.h>
#include <celerique/types.h>
#include <celerique/pipeline.h>

/// @brief The type of UI protocol used to create UI elements.
typedef uint8_t CeleriqueUiProtocol;

/// @brief Null value for `CeleriqueUiProtocol` type.
#define CELERIQUE_UI_PROTOCOL_NULL                                                          0x00

/// @brief Using x11 to build UI elements.
#define CELERIQUE_UI_PROTOCOL_X11                                                           0x01
/// @brief Using the wayland protocol to build UI elements.
#define CELERIQUE_UI_PROTOCOL_WAYLAND                                                       0x02
/// @brief Using win32 api to build UI elements.
#define CELERIQUE_UI_PROTOCOL_WIN32                                                         0x03

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

namespace celerique {
    /// @brief The interface to the specific graphics API.
    class IGraphicsAPI;

    /// @brief The type of UI protocol used to create UI elements.
    typedef CeleriqueUiProtocol UiProtocol;
    /// @brief The type for a pointer container.
    typedef CeleriquePointer Pointer;

    /// @brief An interface to a graphical user interface window.
    class IWindow : public virtual IStateful, public virtual IEventListener,
    public virtual EventBroadcaster {
    public:
        /// @brief Use a particular graphics API for rendering.
        /// @param ptrGraphicsApi 
        virtual void useGraphicsApi(::std::shared_ptr<IGraphicsAPI> ptrGraphicsApi);

    // Protected member variables.
    protected:
        /// @brief The UI protocol used to create UI elements.
        UiProtocol _uiProtocol = CELERIQUE_UI_PROTOCOL_NULL;
        /// @brief The handle to the window according to UI protocol.
        Pointer _windowHandle = 0;
        /// @brief The weak pointer to the graphics API interface.
        ::std::weak_ptr<IGraphicsAPI> _weakPtrGraphicsApi;

    public:
        /// @brief Virtual destructor.
        virtual ~IWindow();
    };

    /// @brief The interface to the specific graphics API.
    class IGraphicsAPI {
    public:
        /// @brief Add a graphics pipeline configuration.
        /// @param graphicsPipelineConfig The graphics pipeline configuration.
        /// @return The unique identifier to the graphics pipeline configuration that was just added.
        virtual PipelineConfigID addGraphicsPipelineConfig(
            PipelineConfig&& graphicsPipelineConfig
        );
        /// @brief Remove the graphics pipeline configuration specified.
        /// @param graphicsPipelineConfigId The identifier of the graphics pipeline configuration to be removed.
        virtual void removeGraphicsPipelineConfig(PipelineConfigID graphicsPipelineConfigId);
        /// @brief Clear the collection of graphics pipeline configurations.
        virtual void clearGraphicsPipelineConfigs();

        /// @brief Graphics draw call.
        /// @param graphicsPipelineConfigId The identifier for the graphics pipeline configuration to be used for drawing.
        /// @param numVerticesToDraw The number of vertices to be drawn.
        /// @param vertexStride The size of the individual vertex input.
        /// @param numVertexElements The number of individual vertices to draw.
        /// @param ptrVertexBuffer The pointer to the vertex buffer.
        /// @param ptrIndexBuffer The pointer to the index buffer.
        virtual void draw(
            PipelineConfigID graphicsPipelineConfigId, size_t numVerticesToDraw, size_t vertexStride = 0,
            size_t numVertexElements = 0, void* ptrVertexBuffer = nullptr, uint32_t* ptrIndexBuffer = nullptr
        ) = 0;

        /// @brief Add the window handle to the graphics API.
        /// @param uiProtocol The UI protocol used to create UI elements.
        /// @param windowHandle The handle to the window according to UI protocol.
        virtual void addWindow(UiProtocol uiProtocol, Pointer windowHandle) = 0;
        /// @brief Remove the window handle from the graphics API registry.
        /// @param windowHandle The handle to the window according to UI protocol.
        virtual void removeWindow(Pointer windowHandle) = 0;
        /// @brief Re-create the swapchain of the specified window.
        /// @param windowHandle The handle to the window of which swapchain to re-create.
        virtual void recreateSwapChain(Pointer windowHandle) = 0;

    // Protected member variables.
    protected:
        /// @brief The map of config identifiers to graphics pipeline configurations.
        ::std::unordered_map<PipelineConfigID, PipelineConfig> _mapIdToGraphicsPipelineConfig;
        /// @brief The value of the next graphics pipeline config identifier value.
        PipelineConfigID _nextGraphicsPipelineConfigId = 0;

    public:
        /// @brief Pure virtual destructor.
        virtual ~IGraphicsAPI() = 0;
    };
}
#endif
// End C++ Only Region

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.