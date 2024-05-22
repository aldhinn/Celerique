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

/// @brief The type of programming language the shader was written on.
typedef uint8_t CeleriqueShaderSrcLang;

/// @brief Null value for `CeleriqueShaderSrcLang` type.
#define CELERIQUE_SHADER_SRC_LANG_NULL                                                      0x00

/// @brief Using GLSL to write the shader program.
#define CELERIQUE_SHADER_SRC_LANG_GLSL                                                      0x01
/// @brief Using HLSL to write the shader program.
#define CELERIQUE_SHADER_SRC_LANG_HLSL                                                      0x02

/// @brief The type of a pipeline shader stage.
typedef uint8_t CeleriqueShaderStage;

/// @brief Null value for `CeleriqueShaderStage` type.
#define CELERIQUE_SHADER_STAGE_NULL                                                         0x00

/// @brief In reference to the vertex shader stage.
#define CELERIQUE_SHADER_STAGE_VERTEX                                                       0x01
/// @brief In reference to the tessellation control shader stage.
#define CELERIQUE_SHADER_STAGE_TESSELLATION_CONTROL                                         0x02
/// @brief In reference to the hull shader stage.
#define CELERIQUE_SHADER_STAGE_HULL                                                         0x02
/// @brief In reference to the tessellation evaluation shader stage.
#define CELERIQUE_SHADER_STAGE_TESSELLATION_EVALUATION                                      0x03
/// @brief In reference to the domain shader stage.
#define CELERIQUE_SHADER_STAGE_DOMAIN                                                       0x03
/// @brief In reference to the geometry shader stage.
#define CELERIQUE_SHADER_STAGE_GEOMETRY                                                     0x04
/// @brief In reference to the fragment shader stage.
#define CELERIQUE_SHADER_STAGE_FRAGMENT                                                     0x05
/// @brief In reference to the pixel shader stage.
#define CELERIQUE_SHADER_STAGE_PIXEL                                                        0x05
/// @brief In reference to the compute shader stage.
#define CELERIQUE_SHADER_STAGE_COMPUTE                                                      0x06
/// @brief In reference to an unspecified shader stage.
#define CELERIQUE_SHADER_STAGE_UNSPECIFIED                                                  0xFF

/// @brief The type of the pipeline configuration unique identifier.
typedef uintptr_t CeleriquePipelineConfigID;

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <vector>
#include <memory>

namespace celerique {
    /// @brief The interface to the specific graphics API.
    class IGraphicsAPI;
    /// @brief The container to a loaded shader program.
    class ShaderProgram;
    /// @brief The interface to a pipeline configuration.
    class IPipelineConfig;

    /// @brief The type of UI protocol used to create UI elements.
    typedef CeleriqueUiProtocol UiProtocol;
    /// @brief The type for a pointer container.
    typedef CeleriquePointer Pointer;
    /// @brief The type of a pipeline shader stage.
    typedef CeleriqueShaderStage ShaderStage;
    /// @brief Type for a byte character.
    typedef CeleriqueByte Byte;
    /// @brief The type of the pipeline configuration unique identifier.
    typedef CeleriquePipelineConfigID PipelineConfigID;

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
            ::std::unique_ptr<IPipelineConfig>&& ptrGraphicsPipelineConfig
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
        ::std::vector<::std::unique_ptr<IPipelineConfig>> _vecPtrGraphicsPipelineConfig;
        /// @brief The identifier to the current graphics pipeline being used for rendering graphics.
        PipelineConfigID _currentGraphicsPipelineConfigUsed = 0;
        /// @brief The value of the next graphics pipeline config identifier value.
        PipelineConfigID _nextGraphicsPipelineConfigId = 0;

    public:
        /// @brief Pure virtual destructor.
        virtual ~IGraphicsAPI() = 0;
    };

    /// @brief Load a shader program from the file path of the binary specified.
    /// @param binaryPath The file path of the binary where the shader is to be loaded from.
    /// @return The loaded shader container instance.
    CELERIQUE_SHARED_SYMBOL ShaderProgram loadShaderProgram(const ::std::string& binaryPath);

    /// @brief The container to a loaded shader program.
    class CELERIQUE_SHARED_SYMBOL ShaderProgram final {
    public:
        /// @brief Init member constructor.
        /// @param size The size of the buffer containing the shader program.
        /// @param ptrBuffer The pointer to the heap allocated buffer containing the shader program.
        ShaderProgram(size_t size = 0, Byte* ptrBuffer = nullptr);

        /// @brief The size of the buffer containing the shader program.
        /// @return `_size` value.
        inline size_t size() const { return _size; }
        /// @brief The pointer to the heap allocated buffer containing the shader program.
        /// @return `_ptrBuffer` value.
        inline Byte* ptrBuffer() const { return _ptrBuffer; }
        /// @brief Determines the state of this shader program container.
        /// @return `true` if this instance is containing no shader, otherwise `false`.
        inline bool isEmpty() const { return _size == 0 || _ptrBuffer == nullptr; }

    // Private member variables.
    private:
        /// @brief The size of the buffer containing the shader program.
        size_t _size;
        /// @brief The pointer to the heap allocated buffer containing the shader program.
        Byte* _ptrBuffer;

    // Copying and moving.
    public:
        /// @brief Copying prevented.
        ShaderProgram(const ShaderProgram&) = delete;
        /// @brief Move constructor.
        /// @param other The r-value reference to the other shader program
        /// container instance where the data is moving from.
        ShaderProgram(ShaderProgram&& other);
        /// @brief Copy re-assignment prevented.
        ShaderProgram& operator=(const ShaderProgram&) = delete;
        /// @brief Move re-assignment operator.
        /// @param other The r-value reference to the other shader program
        /// container instance where the data is moving from.
        /// @return The reference to this instance.
        ShaderProgram& operator=(ShaderProgram&& other);

    public:
        /// @brief Destructor.
        ~ShaderProgram();
    };

    /// @brief The interface to a pipeline configuration.
    class IPipelineConfig {
    public:
        /// @brief Member init constructor.
        /// @param mapStageTypeToShaderProgram The map of shader stages to their corresponding shader programs.
        IPipelineConfig(::std::unordered_map<ShaderStage, ShaderProgram>&& mapShaderStageToShaderProgram = {});

        /// @brief Access the shader program of a particular shader stage.
        /// @param stage The shader stage specified.
        /// @return The reference to the shader program container.
        ShaderProgram& shaderProgram(ShaderStage stage);

    protected:
        /// @brief The map of shader stages to their corresponding shader programs.
        ::std::unordered_map<ShaderStage, ShaderProgram> _mapShaderStageToShaderProgram;

    public:
        /// @brief Pure virtual destructor.
        virtual ~IPipelineConfig() = 0;
    };
}
#endif
// End C++ Only Region

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.