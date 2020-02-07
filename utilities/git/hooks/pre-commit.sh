#!/bin/bash

set -oue pipefail

if [ ! $(which clang-format) ]
then
    echo "Error: program 'clang-format' not found!"
    exit 1
fi

if [ "$(clang-format --version | sed 's/.*version //;s/\..*//')" -lt "7" ]
then
   echo "Error: program 'clang-format' must be version 7 or later!"
   exit 1
fi

clang-format \
        -i \
        -style=file \
        src/*.h \
        src/*.cpp \
        src/test/*.h \
        src/test/*.cpp \
        tests/interface/*/*.cpp
