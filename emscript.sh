#!/bin/bash
trap "exit 255" SIGINT SIGTERM
EMROOT=../emsdk_portable
EMCC=${EMROOT}/emscripten/1.16.0/emcc

case "$1" in
    activate)
        ${EMROOT}/emsdk activate latest
    ;;
    bind)
        EMCC_FAST_COMPILER=0 ${EMCC} --bind \
            -Wno-warn-absolute-paths \
            -I./libea/include \
            -I/usr/local/include \
            examples/emscript_bound.cpp \
            -o html/emscript_bound.js \
            -DLIBEA_CHECKPOINT_OFF \
            -DBOOST_PARAMETER_MAX_ARITY=7
    ;;
    build)
        ${EMCC} \
            -Wno-warn-absolute-paths \
            -I./libea/include \
            -I/usr/local/include \
            examples/emscript.cpp \
            -o html/emscript.html \
            --embed-file ./etc@/ \
           -DLIBEA_CHECKPOINT_OFF \
           -DBOOST_PARAMETER_MAX_ARITY=7
       ;;
    *)
        echo "Usage: $0 {activate|bind|build}"
        exit 1
esac
