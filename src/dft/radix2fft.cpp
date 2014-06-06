
#include <iostream>
#include "radix2fft.h"

static inline unsigned reverseBits(unsigned input, unsigned b) {
    unsigned result = 0;
    for (unsigned i = 0; i < b / 2 + 1; i++) {
        result |= (input & (1 << i)) << (b - 2 * i - 1);
        result |= (input & (1 << (b - i - 1))) >> (b - 2 * i - 1);
    }
    return result;
}

Radix2Fft::Radix2Fft(unsigned sampleCount) : Dft(sampleCount) {
    _log2sc = std::log2(sampleCount);
    if (sampleCount != std::pow(2, _log2sc)) {
        std::cout << "sample count should be a power of 2, but it's " << sampleCount << std::endl;
        throw std::exception();
    }

    _indices = std::vector<unsigned>(sampleCount);
    for (unsigned i = sampleCount; i--; ) {
        _indices[i] = reverseBits(i, _log2sc);
    }
}

std::vector<std::complex<float> > Radix2Fft::compute(const std::vector<float> &samples) {
    unsigned N = sampleCount();
    if (samples.size() < N) {
        std::cout << "sample count is: " << samples.size() << ", expected: " << N << std::endl;
        throw std::exception();
    }

    // Create some constants
    const float pi = std::acos(-1.0f);
    const std::complex<float> j(0, 1.0f);

    // Create result array
    std::vector<std::complex<float> > result(N);
    for (unsigned i = 0; i < N; i++) {
        result[_indices[i]] = samples[i];
    }

    unsigned pow2 = 1;
    for (unsigned level = 0; level < _log2sc; level++, pow2 *= 2) {

        // Calculate exponential multipliers
        std::vector<std::complex<float> > multipliers(pow2);
        for (unsigned i = 0; i < pow2; i++) {
            multipliers[i] = std::exp((-j * 2.0f * pi / (float)(pow2 * 2)) * (float)i);
        }

        for (unsigned a = 0; a < N; a += pow2 * 2) {
            // All the butterflies for a stage
            for (unsigned b = 0; b < pow2; b++) {
                unsigned i = a + b;

                // A single butterfly
                auto u = result[i];
                auto v = result[i + pow2] * multipliers[b];
                result[i] = u + v;
                result[i + pow2] = u - v;
            }
        }
    }

    return result;
}