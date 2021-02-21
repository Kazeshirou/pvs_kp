#!/bin/bash

trap 'CHILDREN=$(ps -o pid= --ppid $CLIENT); kill -SIGINT $CLIENT; kill -SIGINT $CHILDREN; wait $CLIENT; CODE=$?; exit $CODE' SIGINT

valgrind --leak-check=full \
         --error-exitcode=1 \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=./tmp/valgrind-out.txt \
         ../../client.elf -q ./tmp/server/ -l ./tmp/log.log -c 100 &
CLIENT=$!
wait $CLIENT;
