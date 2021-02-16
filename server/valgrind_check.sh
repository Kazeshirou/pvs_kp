#!/usr/bin/bash

valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         ./server.elf -l/tmp/mysmtp -c/tmp/mysmtp_clien

         