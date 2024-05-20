/*

File: ./core/tests/logging.cmocka.c
Author: Aldhinn Espinas
Description: This test ensures no error when including celerique.h

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/logging.h>

#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void consoleLog() {
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

/// @brief Test entry point.
/// @param argc The number of arguments.
/// @param argv The array of arguments in C string.
/// @return Exit code.
int main(int argc, char** argv) {
    const struct CMUnitTest tests[] = {
    cmocka_unit_test(consoleLog)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}