/*

File: ./vulkan/tests/api.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the vulkan api implementations.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/vulkan/api.h>
#include <celerique/vulkan/internal/graphics.h>
#include <celerique/logging.h>

#if defined(CELERIQUE_FOR_WINDOWS)
#include <celerique/win32/window.h>
#else
#include <celerique/x11/window.h>
#endif

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace celerique { namespace vulkan {
#if defined(CELERIQUE_FOR_WINDOWS)
    using namespace ::celerique::win32;
#else
    using namespace ::celerique::x11;
#endif

    /// @brief The GTest unit test suite for the vulkan resource management system.
    class GraphicsApiUnitTestCpp : public ::testing::Test {};

    TEST_F(GraphicsApiUnitTestCpp, moreThanOneInstanceOfGraphicsApiIsNotAllowed) {
        // The legal way of getting the vulkan graphics API interface.
        ::std::shared_ptr<IGraphicsAPI> vulkanGraphicsApi = getGraphicsApiInterface();

        GTEST_TEST_THROW_(
            internal::GraphicsAPI(),
            ::std::runtime_error,
            GTEST_FATAL_FAILURE_
        );
    }

    /// @brief An implementation to the interface to a mock graphics API.
    class MockGraphicsAPI : public IGraphicsAPI {
    public:
        MOCK_METHOD1(addGraphicsPipelineConfig, PipelineConfigID(const PipelineConfig&));
        MOCK_METHOD1(removeGraphicsPipelineConfig, void(PipelineConfigID));
        MOCK_METHOD0(clearGraphicsPipelineConfigs, void());
        MOCK_METHOD4(updateUniform, void(PipelineConfigID, size_t, void*, size_t));
        MOCK_METHOD6(draw, void(PipelineConfigID, size_t, size_t, size_t, void*, uint32_t*));
        MOCK_METHOD2(addWindow, void(UiProtocol, Pointer));
        MOCK_METHOD1(removeWindow, void(Pointer));
        MOCK_METHOD1(reCreateSwapChain, void(Pointer));
        MOCK_METHOD2(createBuffer, GpuBufferID(size_t, GpuBufferUsage));
        MOCK_METHOD3(copyToBuffer, void(GpuBufferID, void*, size_t));
        MOCK_METHOD1(freeBuffer, void(GpuBufferID));
        MOCK_METHOD0(clearBuffers, void());

        MockGraphicsAPI() {
            EXPECT_CALL(*this, addWindow).WillRepeatedly(::testing::Return());
        }
    };

    TEST_F(GraphicsApiUnitTestCpp, windowRegistersToAnotherGraphicsApi) {
        ::std::unique_ptr<IWindow> ptrWindow = createWindow(
            700, 300, ""
        );
        ::std::shared_ptr<IGraphicsAPI> vulkanGraphicsApi = getGraphicsApiInterface();
        ptrWindow->useGraphicsApi(vulkanGraphicsApi);

        ::std::shared_ptr<IGraphicsAPI> mockGraphicsApi = ::std::make_shared<MockGraphicsAPI>();

        celeriqueLogInfo("Window about to register to a mock graphics API.");
        // Window will remove itself from vulkan graphics API, then will register to another graphics API.
        ptrWindow->useGraphicsApi(mockGraphicsApi);
    }

    TEST_F(GraphicsApiUnitTestCpp, expectedBehaviourWhenCallingTwice) {
        ::std::unique_ptr<IWindow> ptrWindow = createWindow(
            700, 300, ""
        );
        ::std::shared_ptr<IGraphicsAPI> vulkanGraphicsApi = getGraphicsApiInterface();
        ptrWindow->useGraphicsApi(vulkanGraphicsApi);
        ptrWindow->useGraphicsApi(vulkanGraphicsApi);
    }
}}