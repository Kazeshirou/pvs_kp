#include "mail_writer.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int is_to_local(mail_writer_t* mw, const char* domain) {
    if (memcmp(mw->domain, domain, strlen(domain)) == 0) {
        return 1;
    }
    if (memcmp(mw->domain, "[127.0.0.1]", strlen(domain)) == 0) {
        return 1;
    }
    return 0;
}

static void generate_local_file_name(mail_writer_t* mw, char* filename,
                                     size_t filename_size) {
    time_t t = time(NULL);
    snprintf(filename, filename_size, "%ld.P%dT%dN%d.%s", t, mw->pid, mw->tid,
             *(mw->N), mw->hostname);
}
static void generate_for_client_file_name(mail_writer_t*    mw,
                                          const receiver_t* receiver,
                                          char*             filename,
                                          size_t            filename_size) {
    time_t t = time(NULL);
    snprintf(filename, filename_size, "%s.%d.T%dN%dM%ld", receiver->domain.text,
             receiver->domain_type, mw->tid, *(mw->N), t);
}
static void generate_file_path(const char* path, const char* folder,
                               const char* filename, char* filepath,
                               size_t filepath_size) {
    snprintf(filepath, filepath_size, "%s%s%s", path, folder, filename);
}

static error_code_t create_link_to_file(mail_writer_t* mw,
                                        const char*    old_file_path,
                                        const char*    local_filename,
                                        receiver_t     receiver) {
    char file_path[1000]      = {0};
    char client_filename[256] = {0};
    char local_filepath[256]  = {0};
    if (is_to_local(mw, receiver.domain.text)) {
        snprintf(local_filepath, sizeof(local_filepath), "%s/new/%s",
                 receiver.local_part.text, local_filename);
        generate_file_path(mw->local_maildir, "", local_filepath, file_path,
                           sizeof(file_path));
    } else {
        generate_for_client_file_name(mw, &receiver, client_filename,
                                      sizeof(client_filename));
        generate_file_path(mw->client_maildir, "", client_filename, file_path,
                           sizeof(file_path));
    }

    link(old_file_path, file_path);
    return CE_SUCCESS;
}

error_code_t write_mail(mail_writer_t* mw, receiver_t* to, size_t to_count,
                        msg_t* text) {
    char local_filename[256] = {0};
    generate_local_file_name(mw, local_filename, sizeof(local_filename));
    char tmp_path[1000] = {0};
    generate_file_path(mw->local_maildir, "tmp/", local_filename, tmp_path,
                       sizeof(tmp_path));
    FILE* tmp_file = fopen(tmp_path, "w");
    if (!tmp_file) {
        return CE_COMMON;
    }
    fprintf(tmp_file, "X-mysmtp-to:");
    char x_to[200];
    for (size_t i = 0; i < to_count - 1; i++) {
        snprintf(x_to, sizeof(x_to), " <%s@%s>,", to[i].local_part.text,
                 to[i].domain.text);
        fprintf(tmp_file, "%s", x_to);
    }
    snprintf(x_to, sizeof(x_to), " <%s@%s>\r\n",
             to[to_count - 1].local_part.text, to[to_count - 1].domain.text);
    fprintf(tmp_file, "%s", x_to);
    fwrite(text->text, 1, text->size, tmp_file);
    fclose(tmp_file);
    for (size_t i = 0; i < to_count; i++) {
        create_link_to_file(mw, tmp_path, local_filename, to[i]);
        (*(mw->N))++;
    }
    remove(tmp_path);

    return CE_SUCCESS;
}