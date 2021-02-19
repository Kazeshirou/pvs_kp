#include <CUnit/Basic.h>
#include "fileparser_test.h"
#include "peer_test.h"
#include "../include/global.h"

char            g_log_message[MAX_G_LOG_MESSAGE];
worker_config_t g_config;
peer_t*         g_logger;

int main(void) 
{
    int ret;
    CU_pSuite fileparser_suite = NULL;
    CU_pSuite peer_suite = NULL;

    if (CU_initialize_registry() != CUE_SUCCESS) 
    {
        printf("Can't initialize cu_registry\n");
        return CU_get_error();
    }

    fileparser_suite = CU_add_suite("Fileparser module tests",
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

    peer_suite = CU_add_suite("Peer module tests",
                              init_suite_peer, cleanup_suite_peer);
    if (!peer_suite) 
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((ret = fill_suite_with_tests_peer(peer_suite)) != CUE_SUCCESS) 
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
