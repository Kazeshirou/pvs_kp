#!/usr/bin/bash

SERVER=0
FLAG=1

trap 'kill -SIGINT $SERVER; FLAG=0' SIGINT

./server.elf -ltests/tmp/server/ -ctests/tmp/client/ -Ltests/tmp/log.csv &

SERVER=$!
while [ $FLAG -ne 0 ]; do
    sleep 1
done
wait $SERVER
CODE=$?
exit $CODE