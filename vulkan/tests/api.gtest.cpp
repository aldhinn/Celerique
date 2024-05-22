/*

File: ./vulkan/tests/api.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the vulkan graphics api implementations.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/vulkan/graphics.h>
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
        MOCK_METHOD2(addWindow, void(UiProtocol, Pointer));
        MOCK_METHOD1(removeWindow, void(Pointer));

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
/*
IN DEBUG MODE, these log messages are expected to appear:

[DEBUG] Created an x11 window. x11/src/window.cpp:92
[TRACE] Registered an x11 vulkan surface. vulkan/src/manager.cpp:437
[TRACE] Selected the best physical device for graphics. vulkan/src/manager.cpp:522
[TRACE] An existing graphics logical device of a window with the same UI protocol is to be used. vulkan/src/manager.cpp:89
[TRACE] Present mailbox mode. vulkan/src/manager.cpp:878
[TRACE] Created swapchain. vulkan/src/manager.cpp:705
[TRACE] Created swapchain image views. vulkan/src/manager.cpp:775
[TRACE] Using existing render pass for a previously registered window of the same UI protocol. vulkan/src/manager.cpp:782
[DEBUG] Registered window. vulkan/src/manager.cpp:96
[TRACE] Destroyed window image views. vulkan/src/manager.cpp:129
[TRACE] Erased window swapchain extent. vulkan/src/manager.cpp:133
[TRACE] Destroyed window swapchain. vulkan/src/manager.cpp:140
[TRACE] Destroyed window surface. vulkan/src/manager.cpp:146
[DEBUG] Removed window from registry. vulkan/src/manager.cpp:150
[TRACE] Registered an x11 vulkan surface. vulkan/src/manager.cpp:437
[TRACE] Selected the best physical device for graphics. vulkan/src/manager.cpp:522
[TRACE] An existing graphics logical device of a window with the same UI protocol is to be used. vulkan/src/manager.cpp:89
[TRACE] Present mailbox mode. vulkan/src/manager.cpp:878
[TRACE] Created swapchain. vulkan/src/manager.cpp:705
[TRACE] Created swapchain image views. vulkan/src/manager.cpp:775
[TRACE] Using existing render pass for a previously registered window of the same UI protocol. vulkan/src/manager.cpp:782
[DEBUG] Registered window. vulkan/src/manager.cpp:96
[DEBUG] X11 window destroyed. x11/src/window.cpp:529
[TRACE] Destroyed window image views. vulkan/src/manager.cpp:129
[TRACE] Erased window swapchain extent. vulkan/src/manager.cpp:133
[TRACE] Destroyed window swapchain. vulkan/src/manager.cpp:140
[TRACE] Destroyed window surface. vulkan/src/manager.cpp:146
[DEBUG] Removed window from registry. vulkan/src/manager.cpp:150

*/
        ::std::unique_ptr<IWindow> ptrWindow = createWindow(
            700, 300, ""
        );
        ::std::shared_ptr<IGraphicsAPI> vulkanGraphicsApi = getGraphicsApiInterface();
        ptrWindow->useGraphicsApi(vulkanGraphicsApi);
        ptrWindow->useGraphicsApi(vulkanGraphicsApi);
    }
}}