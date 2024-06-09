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
        MOCK_METHOD2(addWindow, void(UiProtocol, Pointer));
        MOCK_METHOD1(removeWindow, void(Pointer));
        MOCK_METHOD1(draw, void(PipelineConfigID));
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

    TEST_F(GraphicsUnitTestCpp, graphicsPipelineConfigIdStartsAt0) {
        /// @brief Mock graphics API instance.
        MockGraphicsApi mockGraphicsApi;

        GTEST_ASSERT_EQ(mockGraphicsApi.addGraphicsPipelineConfig(PipelineConfig()), 0);
        // Reset.
        mockGraphicsApi.clearGraphicsPipelineConfigs();
        // It should be back to 0.
        GTEST_ASSERT_EQ(mockGraphicsApi.addGraphicsPipelineConfig(PipelineConfig()), 0);
    }

    TEST_F(GraphicsUnitTestCpp, verifyUniqueIdGeneration) {
        /// @brief The number of iterations of ID generation calls.
        const uint32_t iterations = 1000;
        /// @brief Mock graphics API instance.
        MockGraphicsApi mockGraphicsApi;
        /// @brief The container of IDs generated.
        ::std::vector<PipelineConfigID> vecGeneratedIds;
        vecGeneratedIds.reserve(iterations);

        // Generate IDs
        for (uint32_t i = 0; i < iterations; i++) {
            vecGeneratedIds.push_back(mockGraphicsApi.addGraphicsPipelineConfig(PipelineConfig()));
        }

        /// @brief The container for the IDs that were iterated over.
        ::std::unordered_set<PipelineConfigID> setIteratedOverIds;
        // Iterate over the generated IDs and examine them one by one for duplicates.
        for (PipelineConfigID configId : vecGeneratedIds) {
            // If the current config id is in `setIteratedOverIds`, then a duplicate exists.
            if (setIteratedOverIds.find(configId) != setIteratedOverIds.end()) {
                GTEST_ASSERT_FALSE(true);
            }
            setIteratedOverIds.insert(configId);
        }
    }

    TEST_F(GraphicsUnitTestCpp, verifyShaderSrcExtensionParsing) {
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("some/file"), CELERIQUE_SHADER_SRC_LANG_NULL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("some/file.cpp.glsl"), CELERIQUE_SHADER_SRC_LANG_GLSL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("some/file.glsl."), CELERIQUE_SHADER_SRC_LANG_NULL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("glsl."), CELERIQUE_SHADER_SRC_LANG_NULL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("hlsl."), CELERIQUE_SHADER_SRC_LANG_NULL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("glsl.hlsl"), CELERIQUE_SHADER_SRC_LANG_HLSL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("hlsl.glsl"), CELERIQUE_SHADER_SRC_LANG_GLSL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("hlsl.hlsl.glsl"), CELERIQUE_SHADER_SRC_LANG_GLSL);
    }
}