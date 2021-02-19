#include <CUnit/Basic.h>
#include "fileparser_test.h"


int main(void) 
{
    int ret;
    CU_pSuite fileparser_suite = NULL;

    if (CU_initialize_registry() != CUE_SUCCESS) 
    {
        printf("Can't initialize cu_registry\n");
        return CU_get_error();
    }

    fileparser_suite = CU_add_suite("Bytes module tests",
                              init_suite_fileparser, cleanup_suite_fileparser);
    if (!fileparser_suite) 
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((ret = fill_suite_with_tests_fileparser(fileparser_suite)) != CUE_SUCCESS) 
    {
        CU_cleanup_registry();
        return ret;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    printf("Exiting tests\n");
    return CU_get_error();
}
