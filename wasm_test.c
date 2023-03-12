#include <math.h>
#include <emscripten.h>

int wasm_test() {
    return 6;
}

#define MAX(a, b) (a > b ? a : b)

double process_rms(double prior_volume, double samples[], int sample_count) {
    double sum = 0.0;
    double rms = 0.0;

    for (int i = 0; i < sample_count; ++i) {
        EM_ASM({
            console.log('sample: ' + $0);
        }, samples[i]);

        sum += samples[i];
    }
    rms = sqrt(sum / sample_count);
    return MAX(rms, prior_volume * 0.5);
}