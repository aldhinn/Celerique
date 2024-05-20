/*

File: ./core/tests/header.cmocka.c
Author: Aldhinn Espinas
Description: This test ensures no error when including celerique.h

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique.h>
#include <stdlib.h>

#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void successfulBuildAndRun() {}

/// @brief Test entry point.
/// @param argc The number of arguments.
/// @param argv The array of arguments in C string.
/// @return Exit code.
int main(int argc, char** argv) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(successfulBuildAndRun)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}