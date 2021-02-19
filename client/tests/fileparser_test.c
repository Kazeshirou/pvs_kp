#include "fileparser_test.h"

#include <string.h>
#include <unistd.h>
#include "fileparser.h"

void test_get_filenames(void)
{
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/tests/queue");

    queue_t *filenames = get_filenames(cwd);

    CU_ASSERT_EQUAL(filenames->size, 3);
    CU_ASSERT_STRING_EQUAL(((string_t*)(filenames->front->value))->data, "127.0.0.1.0.id1");
    CU_ASSERT_STRING_EQUAL(((string_t*)(filenames->front->next->value))->data, "::.1.id2");
    CU_ASSERT_STRING_EQUAL(((string_t*)(filenames->back->value))->data, "yandex.ru.2.id2");
}

int init_suite_fileparser(void) 
{
    return 0;
}

int cleanup_suite_fileparser(void) 
{
    return 0;
}

int fill_suite_with_tests_fileparser(CU_pSuite suite) 
{
    if (!CU_add_test(suite, "Test get_filenames func", test_get_filenames))
    {
        return CU_get_error();
    }
    return CUE_SUCCESS;
}



