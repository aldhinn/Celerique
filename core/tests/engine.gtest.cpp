/*

File: ./core/tests/engine.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the the engine functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <utility>
#include <atomic>

namespace celerique {
    /// @brief The GTest unit test suite for testing engine functionalities.
    class EngineUnitTestCpp : public ::testing::Test {};

    /// @brief An interface to a mock graphics user interface window.
    class MockEngineWindow : public virtual WindowBase {
    public:
        MOCK_METHOD1(onUpdate, void(::std::shared_ptr<IUpdateData>));

        MockEngineWindow() {
            EXPECT_CALL(*this, onUpdate).WillRepeatedly([&](::std::shared_ptr<IUpdateData> ptrUpdateData) {
                _atomicDidUpdate.store(true);
            });
        }

        /// @brief The state indicating whether onUpdate was called.
        bool didUpdate() const { return _atomicDidUpdate.load(); }

    // Private member variables.
    private:
        /// @brief Atomic state variable indicating whether onUpdate was called.
        ::std::atomic<bool> _atomicDidUpdate = false;
    };
    /// @brief A mock application layer.
    class MockApplicationLayer : public virtual ApplicationLayerBase {
    public:
        MOCK_METHOD1(onUpdate, void(::std::shared_ptr<IUpdateData>));

        MockApplicationLayer() {
            EXPECT_CALL(*this, onUpdate).WillRepeatedly([&](::std::shared_ptr<IUpdateData> ptrUpdateData) {
                // Shutdown engine after 10 many update calls.
                if (_atomicOnUpdateCount.load() >= 10) {
                    broadcast(
                        ::std::make_shared<::celerique::event::EngineShutdown>(),
                        CELERIQUE_EVENT_HANDLING_STRATEGY_BLOCKING
                    );
                }
                // Extracting elapsed time and logging.
                EngineUpdateData* ptrEngineUpdateData = dynamic_cast<EngineUpdateData*>(ptrUpdateData.get());
                if (ptrEngineUpdateData != nullptr) {
                    celeriqueLogInfo(
                        "Time step: " + ::std::to_string(ptrEngineUpdateData->elapsedNanoSecs()) +
                        " nanoseconds. " + ::std::to_string(ptrEngineUpdateData->elapsedMicroSecs()) +
                        " microseconds. " + ::std::to_string(ptrEngineUpdateData->elapsedMilliSecs()) +
                        " milliseconds."
                    );
                }
                // Update count.
                _atomicOnUpdateCount.store(_atomicOnUpdateCount.load() + 1);
            });
        }

    /// @brief The state indicating whether onUpdate was called.
    bool didUpdate() const { return _atomicOnUpdateCount.load() > 0; }

    // Private member variables.
    private:
        /// @brief The amount of times onUpdate was called.
        ::std::atomic<uint32_t> _atomicOnUpdateCount = 0;
    };

    TEST_F(EngineUnitTestCpp, windowGetsUpdateWhenEngineUpdates) {
        /// @brief Mock window interface pointer.
        ::std::unique_ptr<WindowBase> ptrWindow = ::std::make_unique<MockEngineWindow>();
        // Try to extract mock window pointer to be used later.
        MockEngineWindow* ptrMockWindow = dynamic_cast<MockEngineWindow*>(ptrWindow.get());

        // Add the window we created to the engine.
        addWindow(::std::move(ptrWindow));
        // Call update with no update data.
        onUpdate();

        if (ptrMockWindow == nullptr) {
            GTEST_ASSERT_FALSE(true);
        }
        GTEST_ASSERT_TRUE(ptrMockWindow->didUpdate());
    }

    TEST_F(EngineUnitTestCpp, engineAppLoopCycle) {
        /// @brief Mock application layer.
        ::std::unique_ptr<ApplicationLayerBase> ptrAppLayer = ::std::make_unique<MockApplicationLayer>();
        // Try to extract mock application layer pointer to be used later.
        MockApplicationLayer* ptrMockAppLayer = dynamic_cast<MockApplicationLayer*>(ptrAppLayer.get());
        addAppLayer(::std::move(ptrAppLayer));
        run();
        // Run has stopped running once execution goes beyond here.

        if (ptrMockAppLayer == nullptr) {
            GTEST_ASSERT_FALSE(true);
        }
        GTEST_ASSERT_TRUE(ptrMockAppLayer->didUpdate());
    }
}