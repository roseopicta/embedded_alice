// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// Simple unit tests. More to come!

#include <gtest/gtest.h>

#include "testdata_path.h"

extern "C" {
#include "dsp/dsp_rng.h"
#include "dsp/dsp_zc_generator.h"
}

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const vector<iq_sample_t> kReferenceValues = {
    {260, -6977}, {-3742, 648},    {12147, -7094}, {3974, -5621},
    {-99, -8625}, {-10329, -2099}, {-4427, -2524}, {15953, 665},
    {5426, 2814}, {5462, 5788}};

vector<iq_sample_t> LoadTestData(const string& filename) {
    vector<iq_sample_t> data;
    std::ifstream f(std::string(TESTDATA_DIR) + "/" + filename);
    string line;
    while (getline(f, line)) {
        istringstream ss(line);
        int i, q;
        ss >> i >> q;
        data.emplace_back(iq_sample_t{.i = static_cast<sample_t>(i),
                                      .q = static_cast<sample_t>(q)});
    }
    return data;
}

TEST(RNG, FirstSamples) {
    const size_t num_samples = 10;
    vector<iq_sample_t> samples(num_samples);

    rng_state_t state;
    dsp_rng_init(&state, 7500, 0x7fff, false, 1, 0);
    dsp_rng_generate_icdf(&state, samples.data(), num_samples);

    for (size_t i = 0; i < num_samples; ++i) {
        EXPECT_EQ(samples[i].i, kReferenceValues[i].i)
            << "Mismatch at index " << i << " (I component)";
        EXPECT_EQ(samples[i].q, kReferenceValues[i].q)
            << "Mismatch at index " << i << " (Q component)";
    }
}

TEST(ZCGenerator, FullSequence) {
    size_t sr = 200000000;
    size_t zc_rate = 200000000 / 5;
    zc_generator_state_t zc;

    iq_sample_t lut_phasor[LUT_PHASOR_SIZE];

    dsp_zc_generator_init(&zc, &lut_phasor[0], 3989, 5, 0, zc_rate, sr);
    size_t num_samples = (sr / zc_rate) * zc.length;
    vector<iq_sample_t> samples(num_samples);
    dsp_zc_generator_process(&zc, samples.data(), num_samples);

    vector<iq_sample_t> reference = LoadTestData("zc.tsv");
    EXPECT_EQ(samples.size(), reference.size());
    for (size_t i = 0; i < num_samples; ++i) {
        EXPECT_EQ(int(samples[i].i), reference[i].i)
            << "Mismatch at index " << i << " (I component)";
        EXPECT_EQ(int(samples[i].q), reference[i].q)
            << "Mismatch at index " << i << " (Q component)";
    }
}
