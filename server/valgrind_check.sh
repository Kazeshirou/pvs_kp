#!/usr/bin/bash

valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         --error-exitcode=1 \
         ./server.elf -l/tmp/mysmtp/ -c/tmp/mysmtp_client/ 2> /dev/null

if [ $? -eq 0 ]
then
  exit 0
else
  exit 1
fi