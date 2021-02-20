#include "fileparser_test.h"

#include <string.h>
#include <unistd.h>
#include "peer.h"

void test_add_message_with_marker(void)
{
    peer_t *peer = peer_init(-1, FDT_PIPE);
    char *cmessage = "hello";
    string_t *message = string_init2(cmessage, strlen(cmessage));

    add_message(peer, message, "123");

    CU_ASSERT_EQUAL(peer->messages_in->size, 1);
    CU_ASSERT_STRING_EQUAL(((string_t*)(peer->messages_in->back->value))->data, "hello123");

    string_clear(message);
    peer_clear(peer);
}

void test_add_message_without_marker(void)
{
    peer_t *peer = peer_init(-1, FDT_PIPE);
    char *cmessage = "hello";
    string_t *message = string_init2(cmessage, strlen(cmessage));

    add_message(peer, message, NULL);

    CU_ASSERT_EQUAL(peer->messages_in->size, 1);
    CU_ASSERT_STRING_EQUAL(((string_t*)(peer->messages_in->back->value))->data, "hello");

    string_clear(message);
    peer_clear(peer);
}

void test_fill_messages_out(void)
{
    peer_t *peer = peer_init(-1, FDT_PIPE);
    char *msg = "hello\r\n\0world\r\n!";
    memcpy(peer->buffer_out, msg, 16);
    peer->buffer_out_size = 16;

    fill_messages_out(peer, "\r\n");

    CU_ASSERT_EQUAL(peer->messages_out->size, 2);
    CU_ASSERT_STRING_EQUAL(((string_t*)(peer->messages_out->front->value))->data, "hello");
    CU_ASSERT_STRING_EQUAL(((string_t*)(peer->messages_out->front->next->value))->data, "world");
    CU_ASSERT_EQUAL(peer->buffer_out[0], '!');
    CU_ASSERT_EQUAL(peer->buffer_out_size, 1);

    peer_clear(peer);
}

void test_fill_buffer_in(void)
{
    peer_t *peer = peer_init(-1, FDT_PIPE);
    char *cmessage1 = "hello";
    string_t *message1 = string_init2(cmessage1, strlen(cmessage1));
    char *cmessage2 = "world";
    string_t *message2 = string_init2(cmessage2, strlen(cmessage2));
    queue_push_back(peer->messages_in, message1);
    queue_push_back(peer->messages_in, message2);

    fill_buffer_in(peer);
    
    CU_ASSERT_EQUAL(peer->messages_in->size, 0);
    CU_ASSERT_EQUAL(peer->buffer_in_size, 10);
    int cmp = memcmp(peer->buffer_in, "helloworld", 10);
    CU_ASSERT_EQUAL(cmp, 0);

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
    if (!CU_add_test(suite, "Test add message func", test_add_message_with_marker))
    {
        return CU_get_error();
    }
    if (!CU_add_test(suite, "Test add message func (without end marker)", test_add_message_without_marker))
    {
        return CU_get_error();
    }
    if (!CU_add_test(suite, "Test fill messages out func", test_fill_messages_out))
    {
        return CU_get_error();
    }
    if (!CU_add_test(suite, "Test fill buffer in func", test_fill_buffer_in))
    {
        return CU_get_error();
    }

    return CUE_SUCCESS;
}
