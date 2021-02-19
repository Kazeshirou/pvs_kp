#include "fileparser_test.h"

#include <string.h>
#include <unistd.h>
#include "peer.h"

void test_add_message(void)
{
    peer_t *peer = peer_init(-1, FDT_PIPE);
    char *cmessage = "hello\0";
    string_t *message = string_init2(cmessage, strlen(cmessage));

    add_message(peer, message, "123");

    CU_ASSERT_EQUAL(peer->messages_in->size, 1);
    CU_ASSERT_STRING_EQUAL(((string_t*)(peer->messages_in->back))->data, "hello123");

    string_clear(message);
    peer_clear(peer);
}

int init_suite_peer(void) 
{
    return 0;
}

int cleanup_suite_peer(void) 
{
    return 0;
}

int fill_suite_with_tests_peer(CU_pSuite suite) 
{
    if (!CU_add_test(suite, "Test add message func", test_add_message))
    {
        return CU_get_error();
    }

    return CUE_SUCCESS;
}


 
//int fill_messages_out(peer_t *peer, const char *end_marker);

