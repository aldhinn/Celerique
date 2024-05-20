/*

File: ./core/tests/logging.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the logging functionalities.

License: Affero GNU General Public License. (See ./LICENSE).

*/

#include <gtest/gtest.h>
#include <celerique/logging.h>

namespace celerique {
    /// @brief The GTest unit test suite for logging.
    class LoggingUnitTestCpp : public ::testing::Test {};

    TEST_F(LoggingUnitTestCpp, consoleLog) {
        // Write logs.
        celeriqueLogTrace("Hello from trace.");
        celeriqueLogDebug("Hello from debug.");
        celeriqueLogInfo("Hello from info.");
        celeriqueLogWarning("Hello from warning.");
        celeriqueLogError(
            "Error message. "
            "(Nothing actually went wrong. This is just a log message.)"
        );
        celeriqueLogFatal(
            "Fatal message. "
            "(Nothing actually went wrong. This is just a log message.)"
        );
    }

    TEST_F(LoggingUnitTestCpp, stringConcatenations) {
        int intValue = 69;
        celeriqueLogInfo("The number is " + ::std::to_string(intValue));
    }
}