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
        void useGraphicsApi(::std::shared_ptr<IGraphicsAPI> ptrGraphicsApi);

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
        /// @param ptrGraphicsPipelineConfig The unique pointer to the graphics pipeline configuration.
        /// @return The unique identifier to the graphics pipeline configuration that was just added.
        virtual PipelineConfigID addGraphicsPipelineConfig(
            ::std::unique_ptr<PipelineConfig>&& ptrGraphicsPipelineConfig
        );
        /// @brief Set the graphics pipeline configuration to be used for graphics rendering.
        /// @param graphicsPipelineConfigId The specified identifier of the graphics pipeline config.
        virtual void setGraphicsPipelineConfig(PipelineConfigID graphicsPipelineConfigId);
        /// @brief Reset the graphics pipeline configs vector.
        virtual void resetGraphicsPipelineConfigs();

        /// @brief Add the window handle to the graphics API.
        /// @param uiProtocol The UI protocol used to create UI elements.
        /// @param windowHandle The handle to the window according to UI protocol.
        virtual void addWindow(UiProtocol uiProtocol, Pointer windowHandle) = 0;
        /// @brief Remove the window handle from the graphics API registry.
        /// @param windowHandle The handle to the window according to UI protocol.
        virtual void removeWindow(Pointer windowHandle) = 0;

    // Protected member variables.
    protected:
        /// @brief The vector of unique pointers to graphics pipeline configurations.
        ::std::vector<::std::unique_ptr<PipelineConfig>> _vecPtrGraphicsPipelineConfig;
        /// @brief The identifier to the current graphics pipeline being used for rendering graphics.
        PipelineConfigID _currentGraphicsPipelineConfigUsed = 0;
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