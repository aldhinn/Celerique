/*

File: ./core/tests/graphics.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the the graphics functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/graphics.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <unordered_set>

namespace celerique {
    /// @brief An implementation of an interface to a mock graphics API.
    class MockGraphicsApi : public IGraphicsAPI {
    public:
        MOCK_METHOD1(addGraphicsPipelineConfig, PipelineConfigID(PipelineConfig&&));
        MOCK_METHOD1(removeGraphicsPipelineConfig, void(PipelineConfigID));
        MOCK_METHOD4(updateUniform, void(PipelineConfigID, size_t, void*, size_t));
        MOCK_METHOD6(draw, void(PipelineConfigID, size_t, size_t, size_t, void*, uint32_t*));
        MOCK_METHOD2(addWindow, void(UiProtocol, Pointer));
        MOCK_METHOD1(removeWindow, void(Pointer));
        MOCK_METHOD1(reCreateSwapChain, void(Pointer));
        MOCK_METHOD2(createBuffer, GpuBufferID(size_t, GpuBufferUsage));
        MOCK_METHOD3(copyToBuffer, void(GpuBufferID, void*, size_t));
        MOCK_METHOD1(freeBuffer, void(GpuBufferID));
    };
    /// @brief A mock implementation of an interface to a graphical user interface window.
    class MockWindow : public IWindow {
    public:
        MOCK_METHOD1(onUpdate, void(::std::shared_ptr<IUpdateData>));;
    };

    /// @brief The GTest unit test suite for the generic graphics API tests.
    class GraphicsUnitTestCpp : public ::testing::Test {
        protected:
        /// @brief The unique pointer to the window interface instance.
        ::std::shared_ptr<IWindow> _ptrWindow;
        /// @brief The shared pointer to the mock graphics API interface.
        ::std::shared_ptr<IGraphicsAPI> _ptrGraphicsApi;

    public:
        /// @brief Function to be run before the start of every test.
        inline void SetUp() override {
            // Create a new window instance.
            _ptrWindow = ::std::make_shared<MockWindow>();
            // Create a new graphics API interface instance.
            _ptrGraphicsApi = ::std::make_shared<MockGraphicsApi>();
        }
    };

    TEST_F(GraphicsUnitTestCpp, graphicsApiUsageOfWindow) {
        // Test will fail if the graphics API interface's addWindow method isn't called once.
        EXPECT_CALL(*(dynamic_cast<MockGraphicsApi*>(_ptrGraphicsApi.get())), addWindow).WillOnce(::testing::Return());

        _ptrWindow->useGraphicsApi(_ptrGraphicsApi);
    }

    TEST_F(GraphicsUnitTestCpp, windowRegistersToOneGraphicsApi) {
        // This will be called multiple times.
        EXPECT_CALL(*(dynamic_cast<MockGraphicsApi*>(_ptrGraphicsApi.get())), addWindow).WillRepeatedly(::testing::Return());
        // Test will fail if the graphics API interface's removeWindow method isn't called once.
        EXPECT_CALL(*(dynamic_cast<MockGraphicsApi*>(_ptrGraphicsApi.get())), removeWindow).WillOnce(::testing::Return());

        _ptrWindow->useGraphicsApi(_ptrGraphicsApi);
        // Register again.
        _ptrWindow->useGraphicsApi(_ptrGraphicsApi);
    }

    TEST_F(GraphicsUnitTestCpp, windowRemovesItselfFromGraphicsApiRegistryUponDestruction) {
        // This will be called multiple times.
        EXPECT_CALL(*(dynamic_cast<MockGraphicsApi*>(_ptrGraphicsApi.get())), addWindow).WillRepeatedly(::testing::Return());
        // Test will fail if the graphics API interface's removeWindow method isn't called once.
        EXPECT_CALL(*(dynamic_cast<MockGraphicsApi*>(_ptrGraphicsApi.get())), removeWindow).WillOnce(::testing::Return());

        _ptrWindow->useGraphicsApi(_ptrGraphicsApi);
        _ptrWindow.reset();
    }
}