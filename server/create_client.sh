#!/bin/bash

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 /tmp/mysmtp/ user1" 
fi

FOLDER=$1
USER=$2

mkdir $FOLDER/$USER
mkdir $FOLDER/$USER/new


