#!/usr/bin/bash

trap 'kill -SIGINT $SERVER;  wait $SERVER; CODE=$?; exit $CODE' SIGINT

valgrind --leak-check=full \
         --error-exitcode=1 \
         --show-leak-kinds=all \
         --track-origins=yes \
         --exit-on-first-error=yes \
         --verbose \
         --log-file=tests/tmp/valgrind-out.txt \
         ./server.elf -Ltests/tmp/log.csv -ltests/tmp/server/ -ctests/tmp/client/ &
SERVER=$!
while [ 1 ]; do
    sleep 1
done