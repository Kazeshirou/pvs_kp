#!/bin/bash

trap 'CHILDREN=$(ps -o pid= --ppid $CLIENT); kill -SIGINT $CLIENT; kill -SIGINT $CHILDREN; wait $CLIENT; CODE=$?; exit $CODE' SIGINT

../../client.elf -q ./tmp/server/ -l ./tmp/log.log -c 100 &
CLIENT=$!
wait $CLIENT;
