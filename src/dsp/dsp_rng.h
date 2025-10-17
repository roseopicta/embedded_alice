// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// Digital signal processing routines - Random number generator.

#ifndef DSP_DSP_RNG_H_
#define DSP_DSP_RNG_H_

#include "dsp/dsp_types.h"

// The RNG in the early prototype is a little strange since the
// multiplier a is such that a - 1 is not divisible by 4, a condition
// required to get maximal periodicity
// https://en.wikipedia.org/wiki/Linear_congruential_generator

//#define USE_LEGACY_RNG

typedef struct {
    uint32_t state;
    uint32_t mask;
    uint32_t scale;
    uint32_t max_magnitude;
    bool clamp;
} rng_state_t;

void dsp_rng_init(
    rng_state_t* state,
    uint32_t scale,
    uint32_t max_magnitude,
    bool clamp,
    uint32_t seed,
    uint32_t mask);

void dsp_rng_reset(
    rng_state_t* state,
    uint32_t seed,
    uint32_t mask);

#ifdef USE_LEGACY_RNG

#define RNG_RAND_MAX 32767
#define RNG_SHIFT_LEFT 17

inline uint32_t dsp_rng_rand(rng_state_t* state) {
	state->state = state->state * 995893231 + 93281;
	return ((uint32_t)(state->state / 65536) & RNG_RAND_MAX) ^ state->mask;
}

// Return a float in the interval [0, 1]
inline float dsp_rng_uniform_float(rng_state_t* state) {
    return (float)dsp_rng_rand(state) / ((float)(RNG_RAND_MAX));
}

#else

#define RNG_RAND_MAX 0x7fffffff
#define RNG_SHIFT_LEFT 1

inline uint32_t dsp_rng_rand(rng_state_t* state) {
    state->state = (state->state * 1103515245 + 12345) & RNG_RAND_MAX;
    return state->state ^ state->mask;
}

// Return a float in the interval [0, 1[
inline float dsp_rng_uniform_float(rng_state_t* state) {
    return (float)dsp_rng_rand(state) / ((float)(RNG_RAND_MAX) + 1.0f);
}

#endif  // USE_LEGACY_RNG

inline uint32_t dsp_rng_uniform_u32(rng_state_t* state) {
    return dsp_rng_rand(state) << RNG_SHIFT_LEFT;
}

// Generate gaussian samples using the Box-Muller transform.
void dsp_rng_generate_box_muller(
    rng_state_t* state, iq_sample_t* out, size_t size);

// Generate gaussian samples by linear interpolation from a LUT containing
// the ICDF at four resolutions: first 4096-tile, 256-tile, 16-tile, and
// 16-tiles #2 to #8. The rest is deduced by symmetry.
//
// 2.5x faster than BM, and uses only fixed-point arithmetic.
void dsp_rng_generate_icdf(
    rng_state_t* state, iq_sample_t* out, size_t size);

#endif  // DSP_DSP_RNG_H_
