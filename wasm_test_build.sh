emcc -O3 -s WASM=1 wasm_test.c -sEXPORTED_FUNCTIONS=_wasm_test -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -o wasm_test.js
