#!/usr/bin/env bash

COMPILER="gcc"
DIR=$(realpath $(dirname $0))

build_core() {
    
    mkdir -p "${DIR}/lib/obj"

    for FILE in ${DIR}/src/core/*.c; do
        
        local FILENAME=$(basename ${FILE} .c)
        ${COMPILER} "${DIR}/src/core/${FILENAME}.c" -fpic -c -I "${DIR}/include/core" -o "${DIR}/lib/obj/${FILENAME}.o"

    done

    ${COMPILER} ${DIR}/lib/obj/core_*.o -shared -o ${DIR}/lib/libcore.so 

    return $?

}

build_http() {

    mkdir -p "${DIR}/lib/obj"

    for FILE in ${DIR}/src/http/*.c; do
        
        local FILENAME=$(basename ${FILE} .c)
        ${COMPILER} "${DIR}/src/http/${FILENAME}.c" -fpic -c -I "${DIR}/include/core" -I "${DIR}/include/http" -o "${DIR}/lib/obj/${FILENAME}.o"

    done

    ${COMPILER} ${DIR}/lib/obj/http_*.o -shared -lcore -L ${DIR}/lib -o ${DIR}/lib/libhttp.so

    return $?

}

compile() {

    mkdir -p "${DIR}/bin"

    ${COMPILER} ${DIR}/src/main.c -o ${DIR}/bin/http-server -I ${DIR}/include/core -lcore -I ${DIR}/include/http -lhttp -Wl,-rpath,${DIR}/lib -L ${DIR}/lib 

    return $?

}

build_core
build_http
compile
