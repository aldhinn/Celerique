/*

File: ./core/tests/events.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the Celerique Engine event system.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <atomic>

#include <celerique/events.h>
#include <celerique/logging.h>

namespace celerique {
    /// @brief A mock implementation of an event.
    class MockEvent1 : public Event {
    public:
        MOCK_CONST_METHOD0(category, EventCategory());
        MOCK_CONST_METHOD0(typeID, ::std::type_index());

        inline MockEvent1() {
            EXPECT_CALL(*this, category).WillRepeatedly(::testing::Return(CELERIQUE_EVENT_CATEGORY_NONE));
            EXPECT_CALL(*this, typeID()).WillRepeatedly(::testing::Return(::std::type_index(typeid(MockEvent1))));
        }
    };
    /// @brief Another mock implementation of an event.
    class MockEvent2 : public Event {
    public:
        MOCK_CONST_METHOD0(category, EventCategory());
        MOCK_CONST_METHOD0(typeID, ::std::type_index());

        inline MockEvent2() {
            EXPECT_CALL(*this, category).WillRepeatedly(::testing::Return(CELERIQUE_EVENT_CATEGORY_NONE));
            EXPECT_CALL(*this, typeID()).WillRepeatedly(::testing::Return(::std::type_index(typeid(MockEvent2))));
        }
    };

    /// @brief The GTest unit test suite for the event system.
    class EventUnitTestCpp : public ::testing::Test, public virtual IEventListener,
    public virtual EventBroadcaster {
    protected:
        /// @brief Reset the state variables.
        inline void resetStates() {
            _didDispatchMockEvent1 = false;
            _didDispatchMockEvent2 = false;
            _didDispatchGenericEvent = false;
            _listListeners.clear();
        }

        /// @brief State variable indicating if an event handler for
        /// `MockEvent1` type was dispatched.
        /// @return `_didDispatchMockEvent1` value.
        inline bool didDispatchMockEvent1() { return _didDispatchMockEvent1; }
        /// @brief State variable indicating if an event for
        /// `MockEvent2` type was dispatched.
        /// @return `_didDispatchMockEvent2` value.
        inline bool didDispatchMockEvent2() { return _didDispatchMockEvent2; }
        /// @brief State variable indicating if an event for
        /// a generic `Event` type was dispatched.
        /// @return `_didDispatchGenericEvent` value.
        inline bool didDispatchGenericEvent() { return _didDispatchGenericEvent; }

    public:
        /// @brief Event handler for events of type `MockEvent1`
        /// @param ptrEvent The shared pointer to the event being handled.
        inline void onMockEvent1(::std::shared_ptr<Event> ptrEvent) {
            _didDispatchMockEvent1 = true;
        }
        /// @brief Event handler for events of type `MockEvent2`
        /// @param ptrEvent The shared pointer to the event being handled.
        inline void onMockEvent2(::std::shared_ptr<Event> ptrEvent) {
            _didDispatchMockEvent2 = true;
        }
        /// @brief A generic event handler.
        /// @param ptrEvent The shared pointer to the event being handled.
        inline void onEvent(::std::shared_ptr<Event> ptrEvent) override {
            _didDispatchGenericEvent = true;
        }

    protected:
        /// @brief Pointer to a `MockEvent1` instance.
        ::std::shared_ptr<Event> ptrMockEvent1 = ::std::make_shared<MockEvent1>();
        /// @brief Pointer to a `MockEvent2` instance.
        ::std::shared_ptr<Event> ptrMockEvent2 = ::std::make_shared<MockEvent2>();

    private:
        /// @brief State variable indicating if an event handler for
        /// `MockEvent1` type was dispatched.
        bool _didDispatchMockEvent1 = false;
        /// @brief State variable indicating if an event for
        /// `MockEvent2` type was dispatched.
        bool _didDispatchMockEvent2 = false;
        /// @brief State variable indicating if an event for
        /// a generic `Event` type was dispatched.
        bool _didDispatchGenericEvent = false;

    public:
        /// @brief Function to be run before the start of every test.
        inline void SetUp() override {
            resetStates();

            // Add listeners.
            addEventListener(this);
            addEventListener(::std::bind(
                &EventUnitTestCpp::onMockEvent1, this, ::std::placeholders::_1
            ));
            addEventListener(::std::bind(
                &EventUnitTestCpp::onMockEvent2, this, ::std::placeholders::_1
            ));
        }
    };

    TEST_F(EventUnitTestCpp, ensureCorrectEventDispatched) {
        // Dispatcher.
        EventDispatcher dispatcher(ptrMockEvent1);

        // Dispatch to a `MockEvent1` event handler.
        dispatcher.dispatch<MockEvent1>(::std::bind(
            &EventUnitTestCpp::onMockEvent1, this, ::std::placeholders::_1
        ));

        GTEST_ASSERT_TRUE(didDispatchMockEvent1());
        GTEST_ASSERT_FALSE(didDispatchMockEvent2()); // Only `MockEvent1` was dispatched.
    }

    TEST_F(EventUnitTestCpp, dispatchACompletedEvent) {
        // Complete it's propagation. This handlers should not run anymore.
        ptrMockEvent1->completePropagation();
        // Dispatcher.
        EventDispatcher dispatcher(ptrMockEvent1);

        // Dispatch to a `MockEvent1` event handler.
        dispatcher.dispatch<MockEvent1>(::std::bind(
            &EventUnitTestCpp::onMockEvent1, this, ::std::placeholders::_1
        ));

        // The dispatcher should not have been called because it was a completed
        GTEST_ASSERT_FALSE(didDispatchMockEvent1());
    }

    TEST_F(EventUnitTestCpp, dispatchWrongEvent) {
        // Dispatcher.
        EventDispatcher dispatcher(ptrMockEvent2);

        // Dispatch to a `MockEvent1` event handler.
        dispatcher.dispatch<MockEvent1>(::std::bind(
            &EventUnitTestCpp::onMockEvent1, this, ::std::placeholders::_1
        ));

        GTEST_ASSERT_FALSE(didDispatchMockEvent1());
        GTEST_ASSERT_FALSE(didDispatchMockEvent2());
    }

    TEST_F(EventUnitTestCpp, asyncDispatch) {
        // Dispatcher.
        EventDispatcher dispatcher(ptrMockEvent1);
        // Dispatch to a `MockEvent1` event handler asynchronously.
        dispatcher.dispatch<MockEvent1>(::std::bind(
            &EventUnitTestCpp::onMockEvent1, this, ::std::placeholders::_1
        ), CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC);

        // Wait for a second to ensure the event was handled in the background.
        ::std::this_thread::sleep_for(::std::chrono::seconds(1));

        GTEST_ASSERT_TRUE(didDispatchMockEvent1());
    }

    TEST_F(EventUnitTestCpp, dispatchGenericEvent) {
        // Dispatcher.
        EventDispatcher dispatcher(ptrMockEvent1);
        // Dispatch to a generic event handler.
        dispatcher.dispatch<Event>(::std::bind(
            &EventUnitTestCpp::onEvent, this, ::std::placeholders::_1
        ));

        GTEST_ASSERT_TRUE(didDispatchGenericEvent());
    }

    /// @brief An event type that carries some data.
    class DataCarryingEvent : public virtual Event {
    public:
        /// @brief Init constructor.
        /// @param data The data to be passed.
        inline DataCarryingEvent(size_t data) : _data(data) {}

        /// @brief The data to be passed.
        /// @return `_data` value.
        inline size_t data() const { return _data; }

        CELERIQUE_IMPL_EVENT(DataCarryingEvent, CELERIQUE_EVENT_CATEGORY_NONE);

    private:
        /// @brief The data to be passed.
        size_t _data;
    };

    TEST_F(EventUnitTestCpp, dataTransfer) {
        // The data to be sent over the event system.
        size_t dataSent = 69;
        // The container variable for the data received from the event system.
        ::std::atomic<size_t> dataReceived = 0;

        // Construct a shared pointer event to be dispatched.
        ::std::shared_ptr<Event> ptrEvent = ::std::make_shared<DataCarryingEvent>(dataSent);
        // Attempt to dispatch.
        EventDispatcher dispatcher(ptrEvent);
        dispatcher.dispatch<DataCarryingEvent>([&](::std::shared_ptr<Event> e) {
            // Extract mock event pointer. No need to de-allocate.
            DataCarryingEvent* ptrMockEvent = dynamic_cast<DataCarryingEvent*>(e.get());
            // If failed, halt execution.
            if (ptrMockEvent == nullptr) return;
            // Pass data.
            dataReceived.store(ptrMockEvent->data());
        });
        // Check data received.
        GTEST_ASSERT_EQ(dataSent, dataReceived.load());
    }

    TEST_F(EventUnitTestCpp, deferredDataTransfer) {
        // The data to be sent over the event system.
        size_t dataSent = 69;
        // The container variable for the data received from the event system.
        ::std::atomic<size_t> dataReceived = 0;
        {
            // Construct a shared pointer event to be dispatched.
            ::std::shared_ptr<DataCarryingEvent> ptrEvent = ::std::make_shared<DataCarryingEvent>(dataSent);
            EventDispatcher dispatcher(ptrEvent);
            // Dispatch asynchronously to quickly get dispatcher object out of scope.
            dispatcher.dispatch<DataCarryingEvent>([&](::std::shared_ptr<Event> e) {
                // Extract mock event pointer. No need to de-allocate.
                DataCarryingEvent* ptrMockEvent = dynamic_cast<DataCarryingEvent*>(e.get());
                // If failed, halt execution.
                if (ptrMockEvent == nullptr) return;
                // Wait for three seconds before executing.
                ::std::this_thread::sleep_for(::std::chrono::seconds(3));
                // Store the received data.
                dataReceived.store(ptrMockEvent->data());
            }, CELERIQUE_EVENT_HANDLING_STRATEGY_ASYNC);
        }
        celeriqueLogInfo(
            "dataReceived = " + ::std::to_string(dataReceived.load()) +
            " at the time ptrEvent went out of scope."
        );

        // Wait for five seconds to ensure the event was handled in the background.
        ::std::this_thread::sleep_for(::std::chrono::seconds(5));
        // Verify it was received.
        GTEST_ASSERT_EQ(dataSent, dataReceived.load());

        celeriqueLogInfo(
            "dataReceived = " + ::std::to_string(dataReceived.load())
        );
    }

    TEST_F(EventUnitTestCpp, verifyBroadcast) {
        // Broadcast a mock event.
        broadcast(ptrMockEvent1);

        // Verify all handlers were dispatched.
        GTEST_ASSERT_TRUE(didDispatchGenericEvent());
        GTEST_ASSERT_TRUE(didDispatchMockEvent1());
        GTEST_ASSERT_TRUE(didDispatchMockEvent2());
    }
}