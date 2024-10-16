/*

File: ./core/src/pipeline.cpp
Author: Aldhinn Espinas
Description: This source file contains implementations of GPU pipeline wrappers.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/pipeline.h>
#include <celerique/logging.h>

#include <fstream>
#include <mutex>

/// @brief Load a shader program from the file path of the binary specified.
/// @param binaryPath The file path of the binary where the shader is to be loaded from.
/// @return The loaded shader container instance.
::celerique::ShaderProgram celerique::loadShaderProgram(const ::std::string& binaryPath) {
    /// @brief The input stream of the binary file.
    ::std::basic_ifstream<::celerique::Byte> streamBinaryFile(binaryPath, ::std::ios::binary);
    if (!streamBinaryFile.is_open()) {
        celeriqueLogWarning("Failed to open shader file.");
        return ::celerique::ShaderProgram();
    }

    // Determine the file size
    streamBinaryFile.seekg(0, ::std::ios::end);
    ::std::streampos endOfFilePos = streamBinaryFile.tellg();
    if (endOfFilePos == ::std::basic_ifstream<::celerique::Byte>::pos_type(-1)) {
        celeriqueLogWarning("Failed to determine shader file size.");
        return ::celerique::ShaderProgram();
    }
    streamBinaryFile.seekg(0, ::std::ios::beg);

    /// @brief The size of the buffer containing the shader program.
    size_t size = static_cast<size_t>(endOfFilePos);
    /// @brief The pointer to the heap allocated buffer containing the shader program.
    ::celerique::Byte* ptrBuffer = new ::celerique::Byte[size];

    // Read file contents.
    if (!streamBinaryFile.read(ptrBuffer, static_cast<::std::streamsize>(endOfFilePos))) {
        celeriqueLogWarning("Failed to read from shader file.");
        delete[] ptrBuffer;
        return ::celerique::ShaderProgram();
    }

    return ::celerique::ShaderProgram(size, ptrBuffer);
}

/// @brief Parse the shader source language from the file extension.
/// @param filePath The file path string value.
/// @return The shader source language type.
::celerique::ShaderSrcLang celerique::fileExtToShaderSrcLang(const ::std::string& filePath) {
    // Find the last occurrence of the dot character
    size_t dotPosition = filePath.find_last_of('.');

    // If no dot is found, or the dot is the first character, there is no extension.
    if (dotPosition == std::string::npos || dotPosition == 0) {
        return CELERIQUE_SHADER_SRC_LANG_NULL;
    }
    /// @brief The extension string value.
    ::std::string extension = filePath.substr(dotPosition + 1);

    if (extension == "glsl") return CELERIQUE_SHADER_SRC_LANG_GLSL;
    if (extension == "hlsl") return CELERIQUE_SHADER_SRC_LANG_HLSL;

    return CELERIQUE_SHADER_SRC_LANG_NULL;
}

/// @brief Init member constructor.
/// @param size The size of the buffer containing the shader program.
/// @param ptrBuffer The pointer to the heap allocated buffer containing the shader program.
::celerique::ShaderProgram::ShaderProgram(size_t size, ::celerique::Byte* ptrBuffer) : _size(size), _ptrBuffer(ptrBuffer) {}

#if defined(_MSC_VER) && defined(CELERIQUE_ENGINE_LINKED_SHARED)
/// @brief Copy constructor.
/// @param other The other instance to be copied.
::celerique::ShaderProgram::ShaderProgram(const ShaderProgram& other) : _size(other._size) {
    _ptrBuffer = new Byte[_size];
    ::std::copy(other._ptrBuffer, other._ptrBuffer + _size, _ptrBuffer);
}
#endif

/// @brief Move constructor.
/// @param other The r-value reference to the other shader program
/// container instance where the data is moving from.
::celerique::ShaderProgram::ShaderProgram(ShaderProgram&& other) : _size(other._size),
_ptrBuffer(other._ptrBuffer) {
    other._ptrBuffer = nullptr;
}

/// @brief Move re-assignment operator.
/// @param other The r-value reference to the other shader program
/// container instance where the data is moving from.
/// @return The reference to this instance.
::celerique::ShaderProgram& celerique::ShaderProgram::operator=(ShaderProgram&& other) {
    // If there currently is a shader program being loaded.
    if (_ptrBuffer != nullptr) delete[] _ptrBuffer;

    _size = other._size;
    _ptrBuffer = other._ptrBuffer;
    other._ptrBuffer = nullptr;

    return *this;
}

/// @brief Destructor.
::celerique::ShaderProgram::~ShaderProgram() {
    if (_ptrBuffer != nullptr) {
        delete[] _ptrBuffer;
        _ptrBuffer = nullptr;
    }
}

/// @brief Member init constructor.
/// @param mapStageTypeToShaderProgram The map of shader stages to their corresponding shader programs.
/// @param listVertexInputLayouts The collection of layouts of vertex inputs.
/// @param listUnformInputLayouts The collection of layouts of uniform inputs.
::celerique::PipelineConfig::PipelineConfig(
    ::std::unordered_map<ShaderStage, ShaderProgram>&& mapShaderStageToShaderProgram,
    ::std::list<InputLayout>&& listVertexInputLayouts,
    ::std::list<InputLayout>&& listUnformInputLayouts
) : _mapShaderStageToShaderProgram(::std::move(mapShaderStageToShaderProgram)),
_listVertexInputLayouts(::std::move(listVertexInputLayouts)),
_listUnformInputLayouts(::std::move(listUnformInputLayouts)) {}

/// @brief A shader program container that contains no shader.
static ::celerique::ShaderProgram emptyShaderProgram(0, nullptr);

/// @brief Access the shader program of a particular shader stage.
/// @param stage The shader stage specified.
/// @return The const reference to the shader program container.
const ::celerique::ShaderProgram& celerique::PipelineConfig::shaderProgram(ShaderStage stage) const {
    /// @brief The iterator for the particular shader stage to shader program pair.
    auto iteratorShaderProgram = _mapShaderStageToShaderProgram.find(stage);
    // Return the empty shader program reference if no shader program is paired with a particular shader stage.
    if (iteratorShaderProgram == _mapShaderStageToShaderProgram.end())
        return emptyShaderProgram;
    return (*iteratorShaderProgram).second;
}

/// @brief Access the shader program of a particular shader stage.
/// @param stage The shader stage specified.
/// @return The reference to the shader program container.
::celerique::ShaderProgram& celerique::PipelineConfig::shaderProgram(ShaderStage stage) {
    return _mapShaderStageToShaderProgram[stage];
}

/// @return The shader stages defined in this pipeline configuration.
::std::list<::celerique::ShaderStage> celerique::PipelineConfig::listStages() const {
    /// @brief The container for the collection of shader stages.
    ::std::list<ShaderStage> listStages;
    for (const auto& pairShaderStageToShaderProgram : _mapShaderStageToShaderProgram) {
        /// @brief The shader stage value.
        ShaderStage shaderStage = pairShaderStageToShaderProgram.first;
        // Collect the shader stage.
        listStages.push_back(shaderStage);
    }
    return listStages;
}

/// @brief The collection of layouts of vertex inputs.
/// @return The const reference to `_listVertexInputLayouts`.
const ::std::list<::celerique::InputLayout>& celerique::PipelineConfig::listVertexInputLayouts() const {
    return _listVertexInputLayouts;
}

/// @brief The collection of layouts of vertex inputs.
/// @return The reference to `_listVertexInputLayouts`.
::std::list<::celerique::InputLayout>& celerique::PipelineConfig::listVertexInputLayouts() {
    return _listVertexInputLayouts;
}

/// @brief The collection of layouts of uniform inputs.
/// @return The const reference to `_listUnformInputLayouts`.
const ::std::list<::celerique::InputLayout>& celerique::PipelineConfig::listUnformInputLayouts() const {
    return _listUnformInputLayouts;
}

/// @brief The collection of layouts of uniform inputs.
/// @return The reference to `_listUnformInputLayouts`.
::std::list<::celerique::InputLayout>& celerique::PipelineConfig::listUnformInputLayouts() {
    return _listUnformInputLayouts;
}

/// @brief Calculate and return the stride value.
/// @return The stride value.
size_t celerique::PipelineConfig::stride() const {
    /// @brief The variable that collects how much stride there should be.
    size_t stride = 0;

    // Iterate over input layouts.
    for (const InputLayout& inputLayout : _listVertexInputLayouts) {
        /// @brief The size of each data type.
        size_t dataTypeSize = 0;
        // Determine the size of the type.
        switch(inputLayout.inputType) {
        case CELERIQUE_PIPELINE_INPUT_TYPE_FLOAT:
            dataTypeSize = sizeof(float);
            break;
        case CELERIQUE_PIPELINE_INPUT_TYPE_INT:
            dataTypeSize = sizeof(int);
            break;
        case CELERIQUE_PIPELINE_INPUT_TYPE_DOUBLE:
            dataTypeSize = sizeof(double);
            break;
        case CELERIQUE_PIPELINE_INPUT_TYPE_BOOLEAN:
            dataTypeSize = sizeof(bool);
            break;
        }

        stride += dataTypeSize * inputLayout.numElements;
    }

    return stride;
}

/// @brief Pure virtual destructor.
::celerique::IGpuResources::~IGpuResources() {}

/// @brief The next value of `PipelineConfigID` to be generated.
static ::celerique::PipelineConfigID nextPipelineConfigId = 0;
/// @brief The mutex object restricting access to `nextPipelineConfigId`.
static ::std::mutex nextPipelineConfigIdMutex;
/// @brief Generate an engine-wide unique pipeline configuration identifier.
/// @return The generated `PipelineConfigID`.
::celerique::PipelineConfigID celerique::genPipelineConfigID() {
    ::std::lock_guard<::std::mutex> writeLock(nextPipelineConfigIdMutex);
    return ++nextPipelineConfigId;
}

/// @brief The next value of `GpuBufferID` to be generated.
static ::celerique::GpuBufferID nextGpuBufferId = 0;
/// @brief The mutex object restricting access to `nextGpuBufferId`.
static ::std::mutex nextGpuBufferIdMutex;
/// @brief Generate an engine-wide unique GPU buffer identifier.
/// @return The generated `GpuBufferID`.
::celerique::GpuBufferID celerique::genGpuBufferId() {
    ::std::lock_guard<::std::mutex> writeLock(nextGpuBufferIdMutex);
    return ++nextGpuBufferId;
}