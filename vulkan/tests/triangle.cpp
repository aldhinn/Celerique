/*

File: ./vulkan/tests/triangle.cpp
Author: Aldhinn Espinas
Description: This is a test application of drawing a triangle in the window.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique.h>
#include <celerique/vulkan/graphics.h>

#include <utility>

namespace celerique {
    class TriangleApp : public virtual IApplicationLayer {
    public:
        /// @brief Updates the state.
        /// @param ptrArg The shared pointer to the update data container.
        void onUpdate(::std::shared_ptr<IUpdateData> ptrUpdateData) override {
            _ptrVulkanApi->draw(_triangleGraphicsPipelineId);
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
        TriangleApp() : _ptrVulkanApi(vulkan::getGraphicsApiInterface()) {
            /// @brief Map of shader stages to their shader programs.
            ::std::unordered_map<ShaderStage, ShaderProgram> mapShaderStageToShaderProgram;
            mapShaderStageToShaderProgram[CELERIQUE_SHADER_STAGE_VERTEX] = loadShaderProgram(
                CELERIQUE_REPO_ROOT_DIR "/vulkan/tests/triangle.vert.spv"
            );
            mapShaderStageToShaderProgram[CELERIQUE_SHADER_STAGE_FRAGMENT] = loadShaderProgram(
                CELERIQUE_REPO_ROOT_DIR "/vulkan/tests/triangle.frag.spv"
            );

            _triangleGraphicsPipelineId = _ptrVulkanApi->addGraphicsPipelineConfig(
                PipelineConfig(::std::move(mapShaderStageToShaderProgram))
            );
        }

    // Private member variables.
    private:
        /// @brief The shared pointer to the interface to the vulkan graphics API.
        ::std::shared_ptr<IGraphicsAPI> _ptrVulkanApi;
        /// @brief The identifier of the graphics pipeline for drawing a triangle.
        PipelineConfigID _triangleGraphicsPipelineId;
    };
}

int main(int argc, char** argv) {
#if defined(CELERIQUE_FOR_LINUX_SYSTEMS) || defined(CELERIQUE_FOR_BSD_SYSTEMS)
    using ::celerique::x11::createWindow;
#elif defined(CELERIQUE_FOR_WINDOWS)
    using ::celerique::win32::createWindow;
#endif

    ::std::unique_ptr<::celerique::IWindow> ptrWindow = createWindow(700, 500, "Triangle Application");
    ptrWindow->useGraphicsApi(::celerique::vulkan::getGraphicsApiInterface());
    ::celerique::addWindow(::std::move(ptrWindow));

    ::celerique::addAppLayer(::std::make_unique<::celerique::TriangleApp>());
    ::celerique::run();

    return EXIT_SUCCESS;
}