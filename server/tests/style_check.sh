#!/bin/bash

files=$(find src/ -name '*.c' -not -path "src/checkoptn.c"; find include/ -name '*.h' -not -path "include/client-fsm.h" -not -path "include/checkoptn.h")

errors=()
for file in $files; do
    if ! cmp -s <(cat ${file}) <(clang-format --style=file ${file}) ; then
    errors+=("${file}")
    fi
done


if [ -n "${errors}" ]; then
    echo Тест стиля провален. Ошибки стиля найдены в следующих файлах:
    printf "%s\n" "${errors[@]}"
    exit 1
else
    echo Тест стиля пройден.
fi
