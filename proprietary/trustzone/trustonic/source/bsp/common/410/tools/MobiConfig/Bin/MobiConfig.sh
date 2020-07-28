#!/bin/bash

jar_file=${0%.*}.jar

java -h &>/dev/null
if [ $? -ne 0 ]; then
    echo "Cannot find java virtual machine 'java' in path."
    exit 1
fi

java -jar "$jar_file" "$@"

