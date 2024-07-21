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
        /// @param normal The normal vector of the surface.
        CubeVertex(const Vec3& position, const Vec3& normal) :
        _position(position), _normal(normal) {}

        /// @return The pipeline input layout for a `CubeVertex`.
        static ::std::list<InputLayout> listInputLayouts() {
            /// @brief The collection of layouts of vertex inputs.
            ::std::list<InputLayout> listVertexInputLayouts;

            /// @brief Layout for position.
            InputLayout positionLayout = {};
            positionLayout.name = "inPosition";
            positionLayout.location = 0;
            positionLayout.inputType = CELERIQUE_PIPELINE_INPUT_TYPE_FLOAT;
            positionLayout.numElements = 3;
            positionLayout.offset = offsetof(CubeVertex, _position);

            listVertexInputLayouts.emplace_back(positionLayout);
            celeriqueLogDebug("offsetof(CubeVertex, _position) = " + ::std::to_string(offsetof(CubeVertex, _position)));

            /// @brief Layout for normal.
            InputLayout normalLayout = {};
            normalLayout.name = "inNormal";
            normalLayout.location = 1;
            normalLayout.inputType = CELERIQUE_PIPELINE_INPUT_TYPE_FLOAT;
            normalLayout.numElements = 3;
            normalLayout.offset = offsetof(CubeVertex, _normal);

            listVertexInputLayouts.emplace_back(normalLayout);
            celeriqueLogDebug("offsetof(CubeVertex, _position) = " + ::std::to_string(offsetof(CubeVertex, _normal)));

            return listVertexInputLayouts;
        }

    // Getters.
    public:
        /// @brief The position of a point of a triangle in the world.
        /// @return The const reference to the `_position`.
        const Vec3& position() const { return _position; }
        /// @brief The position of a point of a triangle in the world.
        /// @return The reference to the `_position`.
        Vec3& position() { return _position; }

        /// @brief The normal vector of the surface.
        /// @return The const reference to the `_normal`.
        const Vec3& normal() const { return _normal; }
        /// @brief The normal vector of the surface.
        /// @return The reference to the `_normal`.
        Vec3& normal() { return _normal; }

    // Vertex attributes.
    private:
        /// @brief The position of a point of a triangle in the world.
        Vec3 _position;
        /// @brief The normal vector of the surface.
        Vec3 _normal;
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
            ::std::list<InputLayout> listVertexInputLayouts = CubeVertex::listInputLayouts();

            _cubeGraphicsPipelineId = _ptrVulkanApi->addGraphicsPipelineConfig(
                PipelineConfig(::std::move(mapShaderStageToShaderProgram), ::std::move(listVertexInputLayouts))
            );
        }
        /// @brief Hard code the vertices of the mesh.
        void loadMesh() {
            // Load vertices.
            _vecVertices.reserve(36);
            _vecVertices.insert(_vecVertices.end(), {
                // Top face
                CubeVertex({-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
                CubeVertex({0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
                CubeVertex({0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
                CubeVertex({0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
                CubeVertex({-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
                CubeVertex({-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
                // Front face
                CubeVertex({-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}),
                CubeVertex({0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}),
                CubeVertex({0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}),
                CubeVertex({0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}),
                CubeVertex({-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}),
                CubeVertex({-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}),
                // Right face
                CubeVertex({0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}),
                CubeVertex({0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}),
                CubeVertex({0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}),
                CubeVertex({0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}),
                CubeVertex({0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}),
                CubeVertex({0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}),
            });

            // Load indices.
            _vecIndices.reserve(36);
            _vecIndices.insert(_vecIndices.end(), {
                // Top face
                0, 1, 2, 3, 4, 5,
                // Front face
                6, 7, 8, 9, 10, 11,
                // Right face
                12, 13, 14, 15, 16, 17
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