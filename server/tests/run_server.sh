#!/usr/bin/bash

trap 'kill -SIGINT $SERVER; wait $SERVER; CODE=$?; exit $CODE' SIGINT

./server.elf -ltests/tmp/server/ -ctests/tmp/client/ -Ltests/tmp/log.csv &
SERVER=$!
while [ 1 ]; do
    sleep 1
done