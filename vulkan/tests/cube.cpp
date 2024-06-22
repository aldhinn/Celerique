/*

File: ./vulkan/tests/cube.cpp
Author: Aldhinn Espinas
Description: This is a test application of drawing a cube in the window.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique.h>
#include <celerique/vulkan/graphics.h>

#include <utility>
#include <shared_mutex>
#include <mutex>

namespace celerique {
    /// @brief The container for a cube's vertex input variables.
    class CubeVertex {
    public:
        /// @brief Member init constructor.
        /// @param position The position of a point of a triangle in the world.
        /// @param colour The colour of a point of a triangle.
        CubeVertex(const Vec4& position, const Vec4& colour) :
        _position(position), _colour(colour) {}

        /// @return The pipeline input layout for a `CubeVertex`.
        static ::std::vector<InputLayout> vecInputLayouts() {
            /// @brief The collection of layouts of vertex inputs.
            ::std::vector<InputLayout> vecVertexInputLayouts(2);

            // Layout for position.
            vecVertexInputLayouts[0].name = "inPosition";
            vecVertexInputLayouts[0].location = 0;
            vecVertexInputLayouts[0].inputType = CELERIQUE_PIPELINE_INPUT_TYPE_FLOAT;
            vecVertexInputLayouts[0].numElements = 4;
            vecVertexInputLayouts[0].offset = offsetof(CubeVertex, _position);
            celeriqueLogDebug("offsetof(CubeVertex, _position) = " + ::std::to_string(offsetof(CubeVertex, _position)));

            // Layout for colour.
            vecVertexInputLayouts[1].name = "inColour";
            vecVertexInputLayouts[1].location = 1;
            vecVertexInputLayouts[1].inputType = CELERIQUE_PIPELINE_INPUT_TYPE_FLOAT;
            vecVertexInputLayouts[1].numElements = 4;
            vecVertexInputLayouts[1].offset = offsetof(CubeVertex, _colour);
            celeriqueLogDebug("offsetof(CubeVertex, _colour) = " + ::std::to_string(offsetof(CubeVertex, _colour)));

            return vecVertexInputLayouts;
        }

    // Getters.
    public:
        /// @brief The position of a point of a triangle in the world.
        /// @return The const reference to the `_position`.
        const Vec4& position() const { return _position; }
        /// @brief The position of a point of a triangle in the world.
        /// @return The reference to the `_position`.
        Vec4& position() { return _position; }

        /// @brief The colour of a point of a triangle.
        /// @return The const reference to the `_colour`.
        const Vec4& colour() const { return _colour; }
        /// @brief The colour of a point of a triangle.
        /// @return The reference to the `_colour`.
        Vec4& colour() { return _colour; }

    // Vertex attributes.
    private:
        /// @brief The position of a point of a triangle in the world.
        Vec4 _position;
        /// @brief The colour of a point of a triangle.
        Vec4 _colour;
    };

    /// @brief The application layer that facilitates drawing a cube.
    class CubeApp : public virtual IApplicationLayer {
    public:
        /// @brief Updates the state.
        /// @param ptrArg The shared pointer to the update data container.
        void onUpdate(::std::shared_ptr<IUpdateData> ptrUpdateData) override {
            ::std::shared_lock<::std::shared_mutex> readLock(_sharedMutex);
            _ptrVulkanApi->draw(
                _cubeGraphicsPipelineId, _vecIndices.size(), sizeof(CubeVertex), _vecVertices.size(),
                reinterpret_cast<void*>(_vecVertices.data()), _vecIndices.data()
            );
        }

        /// @brief The event handler method.
        /// @param ptrEvent The shared pointer to the event being dispatched.
        void onEvent(::std::shared_ptr<Event> ptrEvent) override {
            EventDispatcher dispatcher(::std::move(ptrEvent));
            dispatcher.dispatch<event::WindowRequestClose>([&](::std::shared_ptr<Event>) {
                broadcast(::std::make_shared<event::EngineShutdown>());
            });
        }

        /// @brief Default constructor.
        CubeApp() : _ptrVulkanApi(vulkan::getGraphicsApiInterface()) {
            ::std::unique_lock<::std::shared_mutex> writeLock(_sharedMutex);

            configureGraphicsPipeline();
            loadMesh();
        }

    // Initializations.
    private:
        /// @brief Configure the graphics pipeline.
        void configureGraphicsPipeline() {
            /// @brief Map of shader stages to their shader programs.
            ::std::unordered_map<ShaderStage, ShaderProgram> mapShaderStageToShaderProgram;
            mapShaderStageToShaderProgram[CELERIQUE_SHADER_STAGE_VERTEX] = loadShaderProgram(
                CELERIQUE_REPO_ROOT_DIR "/vulkan/tests/cube.vert.spv"
            );
            mapShaderStageToShaderProgram[CELERIQUE_SHADER_STAGE_FRAGMENT] = loadShaderProgram(
                CELERIQUE_REPO_ROOT_DIR "/vulkan/tests/cube.frag.spv"
            );

            /// @brief The collection of layouts of vertex inputs.
            ::std::vector<InputLayout> vecVertexInputLayouts = CubeVertex::vecInputLayouts();

            _cubeGraphicsPipelineId = _ptrVulkanApi->addGraphicsPipelineConfig(
                PipelineConfig(::std::move(mapShaderStageToShaderProgram), ::std::move(vecVertexInputLayouts))
            );
        }
        /// @brief Hard code the vertices of the mesh.
        void loadMesh() {
            // Load vertices.
            _vecVertices.reserve(7);
            _vecVertices.insert(_vecVertices.end(), {
                CubeVertex({0.0f, -0.4f, 0.0f, 1.0f}, {0.0f, 0.3f, 0.6f, 1.0f}),
                CubeVertex({0.0f, 0.1f, 0.0f, 1.0f}, {0.1f, 0.6f, 0.9f, 1.0f}),
                CubeVertex({-0.4f, -0.2f, 0.0f, 1.0f}, {0.0f, 0.4f, 0.7f, 1.0f}),
                CubeVertex({0.4f, -0.2f, 0.0f, 1.0f}, {0.0f, 0.4f, 0.7f, 1.0f}),
                CubeVertex({0.4f, 0.3f, 0.0f, 1.0f}, {0.0f, 0.25f, 0.55f, 1.0f}),
                CubeVertex({0.0f, 0.6f, 0.0f, 1.0f}, {0.0f, 0.3f, 0.6f, 1.0f}),
                CubeVertex({-0.4f, 0.3f, 0.0f, 1.0f}, {0.0f, 0.25f, 0.55f, 1.0f})
            });

            // Load indices.
            _vecIndices.reserve(18);
            _vecIndices.insert(_vecIndices.end(), {
                0, 3, 2,
                2, 3, 1,
                1, 3, 5,
                3, 4, 5,
                1, 5, 2,
                2, 5, 6
            });
        }

    // Private member variables.
    private:
        /// @brief The shared pointer to the interface to the vulkan graphics API.
        ::std::shared_ptr<IGraphicsAPI> _ptrVulkanApi;
        /// @brief The identifier of the graphics pipeline for drawing a cube.
        PipelineConfigID _cubeGraphicsPipelineId;
        /// @brief The collection of vertices to be fed to the draw call.
        ::std::vector<CubeVertex> _vecVertices;
        /// @brief The collection of indices of each vertices to be drawn.
        ::std::vector<uint32_t> _vecIndices;
        /// @brief The shared mutex for the entire application layer.
        ::std::shared_mutex _sharedMutex;
    };
}

int main(int argc, char** argv) {
#if defined(CELERIQUE_FOR_LINUX_SYSTEMS) || defined(CELERIQUE_FOR_BSD_SYSTEMS)
    using ::celerique::x11::createWindow;
#elif defined(CELERIQUE_FOR_WINDOWS)
    using ::celerique::win32::createWindow;
#endif

    ::std::unique_ptr<::celerique::IWindow> ptrWindow = createWindow(700, 500, "Cube Application");
    ptrWindow->useGraphicsApi(::celerique::vulkan::getGraphicsApiInterface());
    ::celerique::addWindow(::std::move(ptrWindow));

    ::celerique::addAppLayer(::std::make_unique<::celerique::CubeApp>());
    ::celerique::run();

    return EXIT_SUCCESS;
}