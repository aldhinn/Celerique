/*

File: ./core/tests/types.check.c
Author: Aldhinn Espinas
Description: This tests the expected type behaviours.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/types.h>

#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>

void ensureBooleanCorrectness() {
    assert_true(true);
    assert_false(false);
    assert_true(!false);
    assert_false(!true);
    assert_true(!!true);
    assert_false(!!false);
    assert_true(true == true);
    assert_true(false == false);
    assert_false(true != true);
    assert_false(false != false);
    assert_true(true != false);
    assert_true(false != true);
    assert_false(true == false);
    assert_false(false == true);
}

/// @brief Test entry point.
/// @param argc The number of arguments.
/// @param argv The array of arguments in C string.
/// @return Exit code.
int main(int argc, char** argv) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(ensureBooleanCorrectness)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}