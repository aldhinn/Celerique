/*

File: ./core/src/graphics.cpp
Author: Aldhinn Espinas
Description: This source file contains implementations of the graphics functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/graphics.h>
#include <celerique/logging.h>

#include <fstream>

/// @brief Virtual destructor.
::celerique::IWindow::~IWindow() {
    ::std::shared_ptr<IGraphicsAPI> ptrPrevGraphicsApi = _weakPtrGraphicsApi.lock();
    if (ptrPrevGraphicsApi != nullptr) {
        // Remove this window from a graphics API that this was registered to.
        ptrPrevGraphicsApi->removeWindow(_windowHandle);
    }
}

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

/// @brief Add a graphics pipeline configuration.
/// @param ptrGraphicsPipelineConfig The unique pointer to the graphics pipeline configuration.
/// @return The unique identifier to the graphics pipeline configuration that was just added.
::celerique::PipelineConfigID celerique::IGraphicsAPI::addGraphicsPipelineConfig(
    ::std::unique_ptr<IPipelineConfig>&& ptrGraphicsPipelineConfig
) {
    // Add the graphics pipeline.
    _vecPtrGraphicsPipelineConfig.emplace_back(::std::move(ptrGraphicsPipelineConfig));
    return _nextGraphicsPipelineConfigId++;
}

/// @brief Set the graphics pipeline configuration to be used for graphics rendering.
/// @param graphicsPipelineConfigId The specified identifier of the graphics pipeline config.
void celerique::IGraphicsAPI::setGraphicsPipelineConfig(PipelineConfigID graphicsPipelineConfigId) {
    _currentGraphicsPipelineConfigUsed = graphicsPipelineConfigId;
}

/// @brief Reset the graphics pipeline configs vector.
void celerique::IGraphicsAPI::resetGraphicsPipelineConfigs() {
    _vecPtrGraphicsPipelineConfig.clear();
    _nextGraphicsPipelineConfigId = 0;
}

/// @brief Pure virtual destructor.
::celerique::IGraphicsAPI::~IGraphicsAPI() {}

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
::celerique::IPipelineConfig::IPipelineConfig(::std::unordered_map<ShaderStage, ShaderProgram>&& mapShaderStageToShaderProgram) :
_mapShaderStageToShaderProgram(::std::move(mapShaderStageToShaderProgram)) {}

/// @brief Access the shader program of a particular shader stage.
/// @param stage The shader stage specified.
/// @return The reference to the shader program container.
::celerique::ShaderProgram& celerique::IPipelineConfig::shaderProgram(ShaderStage stage) {
    return _mapShaderStageToShaderProgram[stage];
}

/// @brief Pure virtual destructor.
::celerique::IPipelineConfig::~IPipelineConfig() {}