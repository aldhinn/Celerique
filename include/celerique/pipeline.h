/*

File: ./include/celerique/pipeline.h
Author: Aldhinn Espinas
Description: This header file contains declarations for GPU pipeline wrappers.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_PIPELINE_HEADER_FILE)
#define CELERIQUE_PIPELINE_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/types.h>

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
/// @brief The type of a GPU buffer unique identifier.
typedef uintptr_t CeleriqueGpuBufferID;

/// @brief The type of a particular pipeline input variable.
typedef uint8_t CeleriquePipelineInputType;
/// @brief Null value for `CeleriquePipelineInputType` type.
#define CELERIQUE_PIPELINE_INPUT_TYPE_NULL                                                  0x00

/// @brief Float pipeline input type.
#define CELERIQUE_PIPELINE_INPUT_TYPE_FLOAT                                                 0x01
/// @brief Integer pipeline input type.
#define CELERIQUE_PIPELINE_INPUT_TYPE_INT                                                   0x02
/// @brief Double pipeline input type.
#define CELERIQUE_PIPELINE_INPUT_TYPE_DOUBLE                                                0x03
/// @brief Boolean pipeline input type.
#define CELERIQUE_PIPELINE_INPUT_TYPE_BOOLEAN                                               0x04

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <unordered_map>
#include <string>
#include <list>

namespace celerique {
    /// @brief The type of the pipeline configuration unique identifier.
    typedef CeleriquePipelineConfigID PipelineConfigID;
    /// @brief The type of a GPU buffer unique identifier.
    typedef CeleriqueGpuBufferID GpuBufferID;
    /// @brief The type of a pipeline shader stage.
    typedef CeleriqueShaderStage ShaderStage;
    /// @brief The type of programming language the shader was written on.
    typedef CeleriqueShaderSrcLang ShaderSrcLang;
    /// @brief Type for a byte character.
    typedef CeleriqueByte Byte;
    /// @brief The type of a particular pipeline input variable.
    typedef CeleriquePipelineInputType PipelineInputType;

    /// @brief The container to a loaded shader program.
    class ShaderProgram;
    /// @brief A layout of a particular shader input variable.
    struct InputLayout;

    /// @brief Load a shader program from the file path of the binary specified.
    /// @param binaryPath The file path of the binary where the shader is to be loaded from.
    /// @return The loaded shader container instance.
    CELERIQUE_SHARED_SYMBOL ShaderProgram loadShaderProgram(const ::std::string& binaryPath);
    /// @brief Parse the shader source language from the file extension.
    /// @param filePath The file path string value.
    /// @return The shader source language type.
    CELERIQUE_SHARED_SYMBOL ShaderSrcLang fileExtToShaderSrcLang(const ::std::string& filePath);

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

    /// @brief Describes a pipeline configuration.
    class CELERIQUE_SHARED_SYMBOL PipelineConfig final {
    public:
        /// @brief Member init constructor.
        /// @param mapStageTypeToShaderProgram The map of shader stages to their corresponding shader programs.
        /// @param listVertexInputLayouts The collection of layouts of vertex inputs.
        PipelineConfig(
            ::std::unordered_map<ShaderStage, ShaderProgram>&& mapShaderStageToShaderProgram = {},
            ::std::list<InputLayout>&& listVertexInputLayouts = {}
        );

        /// @brief Access the shader program of a particular shader stage.
        /// @param stage The shader stage specified.
        /// @return The const reference to the shader program container.
        const ShaderProgram& shaderProgram(ShaderStage stage) const;
        /// @brief Access the shader program of a particular shader stage.
        /// @param stage The shader stage specified.
        /// @return The reference to the shader program container.
        ShaderProgram& shaderProgram(ShaderStage stage);
        /// @return The shader stages defined in this pipeline configuration.
        ::std::list<ShaderStage> listStages() const;

        /// @brief The collection of layouts of vertex inputs.
        /// @return The const reference to `_listVertexInputLayouts`.
        const ::std::list<InputLayout>& listVertexInputLayouts() const;
        /// @brief The collection of layouts of vertex inputs.
        /// @return The reference to `_listVertexInputLayouts`.
        ::std::list<InputLayout>& listVertexInputLayouts();

        /// @brief Calculate and return the stride.
        /// @return The stride value.
        size_t stride() const;

    protected:
        /// @brief The map of shader stages to their corresponding shader programs.
        ::std::unordered_map<ShaderStage, ShaderProgram> _mapShaderStageToShaderProgram;
        /// @brief The collection of layouts of vertex inputs.
        ::std::list<InputLayout> _listVertexInputLayouts;
    };

    /// @brief A layout of a particular shader input variable.
    struct InputLayout {
        /// @brief An index used to identify a specific input in the shader.
        size_t location;
        /// @brief The byte offset of the input variable within a particular batch of input variables.
        size_t offset;
        /// @brief The number of elements this variable contains. (Default 1).
        size_t numElements = 1;
        /// @brief The type of the input variable.
        PipelineInputType inputType;
        /// @brief The name of the variable.
        const char* name = "";
    };
}
#endif
// End C++ Only Region

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.