docker run \
    --rm \
    -v $(pwd):/src \
    -u $(id -u):$(id -g) \
    emscripten/emsdk \
emcc -O3 -s WASM=1 -s AUDIO_WORKLET=1 -s WASM_WORKERS=1 -s ASSERTIONS=1 wasm_test.c -sEXPORTED_FUNCTIONS=_wasm_test,_process_rms -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -o wasm_test.js
exit $?