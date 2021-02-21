/**
 * @file fileparser.c
 * @brief Функции работы с файлом
 */

#include "fileparser.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SMTP_constants.h"
#include "errors.h"
#include "global.h"

#define MAX_LINE_LENGTH 1024

/**
 * Получение списка названий всех файлов в директории
 * @param path_to_dir путь к директории
 * @return список названий обычных файлов
 */
queue_t* get_filenames(const char *path_to_dir)
{
    DIR *dir = opendir(path_to_dir);
    if (dir == NULL) 
    {
        sprintf(g_log_message, "Ошибка открытия директории %s\n", path_to_dir);
        send_log();
        return NULL;
    }

    struct dirent* entry = NULL;
    string_t *filename;
    int ret = 0;

    queue_t *filenames = QUEUE_INIT(string_t, string_copy, string_clear);
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_REG)
            continue;
        filename = string_init2(entry->d_name, strlen(entry->d_name));
        ret      = queue_push_back(filenames, filename);
        string_clear(filename);
        if (ret != SUCCESS) {
            continue;
        }
    }

    closedir(dir);

    return filenames;
}

/**
 * Получение хоста получателей из названия файла письма
 * @param filename название файла письма
 * @param type возвращаемый параметр -- тип адреса (0 -- IPv4, 1 -- IPv6, 2 -- доменное имя)
 * @return хост получателей
 */
char* get_addr(const string_t* filename, int* type) {
    int meta_start;
    int filename_len = filename->size;

    for (meta_start = filename_len;
         meta_start >= 0 && filename->data[meta_start] != SERVER_ADDR_SEP;
         meta_start--)
        ;

    if (meta_start < 4) {
        sprintf(g_log_message, "Ошибка определения хоста/ip %s",
                filename->data);
        send_log();
        return NULL;
    }

    meta_start--;
    *type = filename->data[meta_start] - '0';
    meta_start--;

    const size_t max_size = meta_start + 1;
    char*        addr     = (char*)calloc(max_size, sizeof(char));
    if (addr == NULL) {
        sprintf(g_log_message, "Ошибка выделения памяти: get_addr()");
        send_log();
        return NULL;
    }

    memcpy(addr, filename->data, meta_start);
    return addr;
}

void free_addr(char* addr) {
    free(addr);
}

/**
 * Получение количества получателей
 * @param line строка файла письма, где указаны получатели
 * @return общее количество
 */
size_t get_recipients_count(const char* line) {
    size_t count = 0;
    int    i;
    for (i = 0; i < strlen(line); i++)
        if (line[i] == TO_SEPARATOR)
            count++;
    return count + 1;
}

/**
 * Проверка, нужно ли отправлять получателю письмо (или это сделано будет по другому соединению)
 * @param rcpt почта получателя
 * @param addr хост из названия файла
 * @param a_type тип адреса (0 -- IPv4, 1 -- IPv6, 2 -- доменное имя)
 * @return 1 -- нужно, 0 -- нет
 */
int check_recipient(char* rcpt, const char* addr, int a_type) {
    int at_idx;
    for (at_idx = 0; at_idx < strlen(rcpt) && rcpt[at_idx] != '@'; at_idx++)
        ;

    if (memcmp(rcpt + at_idx + 1, DOMAIN_NAME, strlen(DOMAIN_NAME)) == 0)
        return 0;

    if (a_type == 2) {
        if (memcmp(rcpt + at_idx + 1, addr, strlen(addr)) == 0)
            return 1;
    }
    if (a_type == 0) {
        if (rcpt[at_idx + 1] != '[')
            return 0;
        //if (strlen(addr) != (strlen(rcpt) - at_idx - 4))
        //    return 0;
        if (memcmp(rcpt + at_idx + 2, addr, strlen(addr)) == 0)
            return 1;
    }
    if (a_type == 1) {
        if (rcpt[at_idx + 1] != '[')
            return 0;
        //if (strlen(addr) != (strlen(rcpt) - at_idx - 8))
        //    return 0;
        if (memcmp(rcpt + at_idx + 6, addr, strlen(addr)) == 0)
            return 1;
    }
    return 0;
}

/**
 * Парсинг списка почт получателей
 * @param line строка из файла письма, где указаны получатели
 * @param count возвращаемый параметр -- количество получателей
 * @param addr хост из названия файла
 * @param a_type тип адреса (0 -- IPv4, 1 -- IPv6, 2 -- доменное имя)
 * @return список почт, куда надо отправить письмо
 */
char** parse_recipients(const char* line, size_t* count, const char* addr,
                        int a_type) {
    char* header            = X_DOMAIN_TO;
    char* rcpt              = NULL;
    int   start             = strlen(header);
    int   end               = strlen(line);
    int   rcpt_start        = 0;
    int   rcpt_len          = 0;
    *count                  = get_recipients_count(line);
    size_t current_rcpt_num = 0;
    int    i;
    char** recipients = (char**)malloc(sizeof(char*) * *count);
    for (i = start; i < end; i += 2) {
        rcpt_start = i;
        while (line[i] != TO_SEPARATOR && i < end)
            i++;
        rcpt_len = i - rcpt_start;
        rcpt = (char*)malloc(sizeof(char) * rcpt_len);
        memcpy(rcpt, line + rcpt_start, rcpt_len);
        rcpt[rcpt_len] = '\0';

        if (!check_recipient(rcpt, addr, a_type)) {
            (*count)--;
            free(rcpt);
        } else {
            recipients[current_rcpt_num++] = rcpt;
        }
    }
    return recipients;
}

/**
 * Парсинг почты отправителя
 * @param line строка из файла письма, где указаны получатели
 * @return почта отправителя
 */
char* parse_sender(const char* line) {
    char* header     = X_DOMAIN_FROM;
    int   start      = strlen(header);
    int   end        = strlen(line);
    int   sender_len = end - start;
    char* sender     = (char*)malloc(sizeof(char) * sender_len);
    if (!sender) {
        sprintf(g_log_message, "Ошибка выделения памяти: parse_sender()");
        send_log();
        return NULL;
    }
    memcpy(sender, line + start, sender_len);
    sender[sender_len] = '\0';
    return sender;
}

char *trim (char *s) {
    int i = strlen(s)-1;
    while ((i > 0) && (s[i] == '\n' || s[i] == '\r'))
    {
        s[i] = '\0';
        i--;
    }
    return s;
}

/**
 * Основная функция парсинга письма
 * @param queue_dir путь к директории с файлом письма
 * @param filename название файла письма
 * @return письмо
 */
SMTP_message_t* parse_message(const char* queue_dir, const string_t* filename) {
    int   a_type;
    char* addr = get_addr(filename, &a_type);
    if (!addr)
        return NULL;

    SMTP_message_t* message = SMTP_message_init();
    if (!message) {
        free(addr);
        return NULL;
    }
    int       line_num                  = 0;
    char      line[MAX_LINE_LENGTH + 1] = {0};
    string_t* queue_dir_str = string_init2(queue_dir, strlen(queue_dir));
    string_t* full_filename =
        concat_with_sep(queue_dir_str, filename, FILENAME_SEP);
    FILE* f             = fopen(full_filename->data, "r");
    int   max_msg_lines = 64;
    message->msg        = (string_t**)malloc(sizeof(string_t*) * max_msg_lines);

    string_clear(queue_dir_str);
    string_clear(full_filename);

    if (f) {
        while (fgets(line, MAX_LINE_LENGTH, f)) {
            trim(line);
            if (line_num == 0) {
                message->recipients_addr = parse_recipients(
                    line, &(message->recipients_count), addr, a_type);
                if (message->recipients_count <= 0)
                    break;
            } else if (line_num == 1)
                message->from_addr = parse_sender(line);
            else {
                if (max_msg_lines == message->msg_lines) {
                    max_msg_lines *= 2;
                    message->msg = (string_t**)realloc(
                        message->msg, sizeof(string_t*) * max_msg_lines);
                }
                message->msg[message->msg_lines++] =
                    string_init2(line, strlen(line));
            }
            line_num++;
        }
        fclose(f);
    } else {
        sprintf(g_log_message, "Ошибка открытия файла: %s",
                full_filename->data);
        send_log();
    }

    if (message->recipients_count <= 0) {
        sprintf(g_log_message,
                "Количество получателей с адресом %s равно 0: %s", addr, line);
        send_log();
        free(message);
        free(addr);
        return NULL;
    }
    free(addr);
    return message;
}