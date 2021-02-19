#include "fileparser_test.h"

#include <string.h>
#include <unistd.h>
#include "fileparser.h"

void test_get_filenames(void)
{
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/tests/unit/queue");

    queue_t *filenames = get_filenames(cwd);

    CU_ASSERT_EQUAL(filenames->size, 3);
    CU_ASSERT_STRING_EQUAL(((string_t*)(filenames->front->value))->data, "127.0.0.1.0.id1");
    CU_ASSERT_STRING_EQUAL(((string_t*)(filenames->front->next->value))->data, "::.1.id2");
    CU_ASSERT_STRING_EQUAL(((string_t*)(filenames->back->value))->data, "yandex.ru.2.id2");

    queue_clear(filenames);
}

void test_get_addr_type0(void)
{
    char *cfilename = "127.0.0.1.0.001";
    string_t *filename = string_init2(cfilename, strlen(cfilename));
    int type;
    char *addr = get_addr(filename, &type);

    CU_ASSERT_EQUAL(type, 0);
    CU_ASSERT_STRING_EQUAL(addr, "127.0.0.1");

    string_clear(filename);
    free(addr);
}

void test_get_addr_type1(void)
{
    char *cfilename = "::.1.001";
    string_t *filename = string_init2(cfilename, strlen(cfilename));
    int type;
    char *addr = get_addr(filename, &type);

    CU_ASSERT_EQUAL(type, 1);
    CU_ASSERT_STRING_EQUAL(addr, "::");

    string_clear(filename);
    free(addr);
}

void test_get_addr_type2(void)
{
    char *cfilename = "yandex.ru.2.001";
    string_t *filename = string_init2(cfilename, strlen(cfilename));
    int type;
    char *addr = get_addr(filename, &type);

    CU_ASSERT_EQUAL(type, 2);
    CU_ASSERT_STRING_EQUAL(addr, "yandex.ru");

    string_clear(filename);
    free(addr);
}

void test_parse_message_type0(void)
{
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/tests/unit/queue");
    char *cfilename = "127.0.0.1.0.id1";
    string_t *filename = string_init2(cfilename, strlen(cfilename));

    SMTP_message_t *msg = parse_message(cwd, filename);

    CU_ASSERT_STRING_EQUAL(msg->from_addr, "<aa@yandex.ru>");
    CU_ASSERT_EQUAL(msg->recipients_count, 1);
    CU_ASSERT_STRING_EQUAL(msg->recipients_addr[0], "<bb@[127.0.0.1]>");
    CU_ASSERT_EQUAL(msg->msg_lines, 1);
    CU_ASSERT_STRING_EQUAL(msg->msg[0]->data, "1234");

    SMTP_message_clear(msg);
}

void test_parse_message_type1(void)
{
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/tests/unit/queue");
    char *cfilename = "::.1.id2";
    string_t *filename = string_init2(cfilename, strlen(cfilename));

    SMTP_message_t *msg = parse_message(cwd, filename);


    CU_ASSERT_STRING_EQUAL(msg->from_addr, "<aa@yandex.ru>");
    CU_ASSERT_EQUAL(msg->recipients_count, 1);
    CU_ASSERT_STRING_EQUAL(msg->recipients_addr[0], "<bb@[IPv6::]>");
    CU_ASSERT_EQUAL(msg->msg_lines, 1);
    CU_ASSERT_STRING_EQUAL(msg->msg[0]->data, "1234");

    SMTP_message_clear(msg);
}

void test_parse_message_type2(void)
{
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/tests/unit/queue");
    char *cfilename = "yandex.ru.2.id2";
    string_t *filename = string_init2(cfilename, strlen(cfilename));

    SMTP_message_t *msg = parse_message(cwd, filename);

    CU_ASSERT_STRING_EQUAL(msg->from_addr, "<aa@yandex.ru>");
    CU_ASSERT_EQUAL(msg->recipients_count, 1);
    CU_ASSERT_STRING_EQUAL(msg->recipients_addr[0], "<cc@yandex.ru>");
    CU_ASSERT_EQUAL(msg->msg_lines, 1);
    CU_ASSERT_STRING_EQUAL(msg->msg[0]->data, "1234");

    SMTP_message_clear(msg);
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
    if (!CU_add_test(suite, "Test get_addr func (addr type 0)", test_get_addr_type0))
    {
        return CU_get_error();
    }
    if (!CU_add_test(suite, "Test get_addr func (addr type 1)", test_get_addr_type1))
    {
        return CU_get_error();
    }
    if (!CU_add_test(suite, "Test get_addr func (addr type 2)", test_get_addr_type2))
    {
        return CU_get_error();
    }
    if (!CU_add_test(suite, "Test parse message func (addr type 0)", test_parse_message_type0))
    {
        return CU_get_error();
    }
    if (!CU_add_test(suite, "Test parse message func (addr type 1)", test_parse_message_type1))
    {
        return CU_get_error();
    }
    if (!CU_add_test(suite, "Test parse message func (addr type 2)", test_parse_message_type2))
    {
        return CU_get_error();
    }
    return CUE_SUCCESS;
}



