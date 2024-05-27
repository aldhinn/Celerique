/*

File: ./core/src/pipeline.cpp
Author: Aldhinn Espinas
Description: This source file contains implementations of GPU pipeline wrappers.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/pipeline.h>
#include <celerique/logging.h>

#include <fstream>

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

/// @brief Move constructor.
/// @param other The r-value reference to the other shader program
/// container instance where the data is moving from.
::celerique::ShaderProgram::ShaderProgram(ShaderProgram&& other) : _size(other._size),
_ptrBuffer(other._ptrBuffer) {
    other._size = 0;
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
    other._size = 0;
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
::celerique::PipelineConfig::PipelineConfig(::std::unordered_map<ShaderStage, ShaderProgram>&& mapShaderStageToShaderProgram) :
_mapShaderStageToShaderProgram(::std::move(mapShaderStageToShaderProgram)) {}

/// @brief Access the shader program of a particular shader stage.
/// @param stage The shader stage specified.
/// @return The reference to the shader program container.
::celerique::ShaderProgram& celerique::PipelineConfig::shaderProgram(ShaderStage stage) {
    return _mapShaderStageToShaderProgram[stage];
}

/// @return The shader stages defined in this pipeline configuration.
::std::vector<::celerique::ShaderStage> celerique::PipelineConfig::vecStages() const {
    /// @brief The container for the collection of shader stages.
    ::std::vector<ShaderStage> vecStages;
    for (const auto& pairShaderStageToShaderProgram : _mapShaderStageToShaderProgram) {
        /// @brief The shader stage value.
        ShaderStage shaderStage = pairShaderStageToShaderProgram.first;
        // Collect the shader stage.
        vecStages.push_back(shaderStage);
    }
    return vecStages;
}