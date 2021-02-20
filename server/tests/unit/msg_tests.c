#include "msg_tests.h"

#include "custom_errors.h"
#include "msg.h"

#define test_def "ABC"

void test_msg(void) {
    const char   test_str1[] = "ABC";
    const char   test_str2[] = "D";
    const char   result[]    = "ABCD";
    msg_t        msg;
    error_code_t cerr = msg_init(&msg, 1);
    CU_ASSERT_EQUAL(cerr, CE_SUCCESS);
    CU_ASSERT_EQUAL(msg.size, 0);
    cerr = msg_add_text(&msg, test_str1, sizeof(test_str1) - 1);
    CU_ASSERT_EQUAL(cerr, CE_SUCCESS);
    CU_ASSERT_EQUAL(msg.size, sizeof(test_str1) - 1);
    cerr = msg_add_text(&msg, test_str2, sizeof(test_str2));
    CU_ASSERT_EQUAL(cerr, CE_SUCCESS);
    CU_ASSERT_EQUAL(strlen(msg.text), strlen(result));
    CU_ASSERT_EQUAL(msg.size, sizeof(result));
    CU_ASSERT_STRING_EQUAL(msg.text, result);
    msg_clean(&msg);
    CU_ASSERT_EQUAL(strlen(msg.text), 0);
    CU_ASSERT_EQUAL(msg.size, 0);
    msg_destroy(&msg);
}


int init_suite_msg(void) {
    return 0;
}

int cleanup_suite_msg(void) {
    return 0;
}

int fill_suite_with_tests_msg(CU_pSuite suite) {
    if (!CU_add_test(suite, "Test msg func", test_msg)) {
        return CU_get_error();
    }

    return CUE_SUCCESS;
}
