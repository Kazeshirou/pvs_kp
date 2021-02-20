#include <CUnit/Basic.h>

#include "logger.h"
#include "msg_tests.h"

volatile sig_atomic_t while_true = 1;

logger_t  logger;
logger_t* logger_ = &logger;

int main(void) {
    int       ret;
    CU_pSuite msg_suite = NULL;

    if (CU_initialize_registry() != CUE_SUCCESS) {
        printf("Can't initialize cu_registry\n");
        return CU_get_error();
    }

    msg_suite =
        CU_add_suite("Msg module tests", init_suite_msg, cleanup_suite_msg);
    if (!msg_suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((ret = fill_suite_with_tests_msg(msg_suite)) != CUE_SUCCESS) {
        CU_cleanup_registry();
        return ret;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    ret = CU_get_number_of_failures();
    CU_cleanup_registry();
    printf("Exiting tests\n");
    return ret;
}
