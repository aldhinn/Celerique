/*

File: ./core/tests/abstracts.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the the common abstract type functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/abstracts.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace celerique {
    /// @brief The GTest unit test suite for testing common abstract type functionalities.
    class AbstractsUnitTestCpp : public ::testing::Test {};

    /// @brief A mock implementation of an update data container.
    class MockUpdateData : public IUpdateData {
    public:
        /// @brief Member init constructor.
        /// @param data The update information.
        MockUpdateData(int data = 0) : _data(data) {}
        /// @brief The update information.
        int data() const { return _data; }
    private:
        /// @brief The update information.
        int _data;
    };
    /// @brief A mock implementation of a stateful object.
    class MockStateful : public IStateful {
    public:
        /// @brief Default constructor.
        MockStateful() {
            EXPECT_CALL(*this, onUpdate).WillRepeatedly([&](::std::unique_ptr<IUpdateData>&& ptrUpdateData) {
                // Determine if update data object is `MockUpdateData`.
                MockUpdateData* ptrMockUpdateData = dynamic_cast<MockUpdateData*>(ptrUpdateData.get());
                if (ptrMockUpdateData == nullptr) return;
                // Update data.
                _lastDataReceived = ptrMockUpdateData->data();
            });
        }
        /// @brief The latest data received from the last this object was updated.
        int lastDataReceived() { return _lastDataReceived; }

        MOCK_METHOD1(onUpdate, void(::std::unique_ptr<IUpdateData>&&));

    private:
        /// @brief The latest data received from the last this object was updated.
        int _lastDataReceived = 0;
    };

    TEST_F(AbstractsUnitTestCpp, statefulInterfaceDefaultNullptrOnUpdate) {
        /// @brief The interface to the mock stateful object.
        ::std::unique_ptr<IStateful> ptrStateful = ::std::make_unique<MockStateful>();

        // Call onUpdate without args.
        ptrStateful->onUpdate();
    }

    TEST_F(AbstractsUnitTestCpp, dataTransferOnUpdate) {
        /// @brief The interface to the mock stateful object.
        ::std::unique_ptr<IStateful> ptrStateful = ::std::make_unique<MockStateful>();
        /// @brief The expected data value.
        int expectedData = 23;

        // Update.
        ptrStateful->onUpdate(::std::make_unique<MockUpdateData>(expectedData));

        // Extract `MockStateful` instance pointer.
        MockStateful* ptrMockStateful = dynamic_cast<MockStateful*>(ptrStateful.get());
        if (ptrMockStateful == nullptr) {
            // Fail the test as something wrong with the test implementation.
            GTEST_ASSERT_TRUE(false);
        }

        // Check if data was updated.
        GTEST_ASSERT_EQ(expectedData, ptrMockStateful->lastDataReceived());
    }
}