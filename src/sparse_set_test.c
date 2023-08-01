// vim: set colorcolumn=85
// vim: fdm=marker

#include "munit.h"
#include "koh_sparse_set.h"
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MunitResult test_init_shutdown(
    const MunitParameter params[], void* data
) {
    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
  {
    (char*) "/init_shutdown",
    test_init_shutdown,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "sparse_set", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char **argv) {
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
