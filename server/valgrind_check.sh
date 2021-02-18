#!/usr/bin/bash

SERVER=0
FLAG=1

trap 'kill -SIGINT $SERVER; FLAG=0' SIGINT

valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=tests/tmp/valgrind-out.txt \
         --error-exitcode=1 \
         ./server.elf -Ltests/tmp/log.csv -ltests/tmp/server/ -ctests/tmp/client/ 2> /dev/null &

SERVER=$!
while [ $FLAG -ne 0 ]; do
    sleep 1
done
wait $SERVER
CODE=$?
exit $CODE