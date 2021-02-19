#pragma once

#include <netinet/in.h>	// sockaddr_in

#include "queue.h"
#include "mstring.h"

#define MAX_BUFFER_SIZE 1024

#define FDT_SOCKET '0'
#define FDT_PIPE '1'

/**
 * @brief Структура соединения на низком уровне для непосредственно чтения/записи из/в файловый дескриптор.
 * Процессы не работают с буферами напрямую, а для отправки/чтения данных по/из сокету/а или каналу/а формируют строку и добавляют её в очередь.
**/
typedef struct peer__t 
{
    int fd; ///< файловый дескриптор
    char type; ///< тип (сокет или pipe)

    queue_t *messages_in; ///< очередь еще не добавленных полностью в буфер строк, которые процесс хочет отправить
    size_t message_in_begin; ///< первый символ головы из messages_in, начиная с которого голова еще не в буфере
    char buffer_in[MAX_BUFFER_SIZE]; ///< входной буфер соединения
    size_t buffer_in_size; ///< текущий размер входного буфер соединения

    queue_t *messages_out; ///< очередь строк, которые получены по сокету или каналу, удалены из буфера, но еще не обработаны процессом
    char buffer_out[MAX_BUFFER_SIZE]; ///< выходной буфер соединения
    size_t buffer_out_size; ///< текущий размер выходного буфер соединения
} peer_t;

peer_t* peer_init(int fd, char type);
void peer_clear(void *peer);
void* peer_copy(const void *peer);

int add_message(peer_t *peer, const string_t *message, const char *end_marker);

int peer_send(peer_t *peer);
int peer_receive(peer_t *peer);
 
int fill_messages_out(peer_t *peer, const char *end_marker);
