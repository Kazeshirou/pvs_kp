#!/bin/bash

trap 'kill -SIGINT $CLIENT; wait $CLIENT; CODE=$?; exit $CODE' SIGINT

../../client.elf -q ./tmp/server/ -l ./tmp/log.log -c 100 &
CLIENT=$!
wait $CLIENT;
