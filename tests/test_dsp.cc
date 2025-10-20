// Author: Emilie Gillet (emilie.gillet@etu.sorbonne-universite.fr)
//
// -----------------------------------------------------------------------------
//
// Simple unit tests. More to come!

#include <gtest/gtest.h>

#include "testdata_path.h"

extern "C" {
#include "dsp/dsp_rng.h"
#include "dsp/dsp_rrc_filter.h"
#include "dsp/dsp_zc_generator.h"
}

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const vector<iq_sample_t> kReferenceValuesRNG = {
    {260, -6977}, {-3742, 648},    {12147, -7094}, {3974, -5621},
    {-99, -8625}, {-10329, -2099}, {-4427, -2524}, {15953, 665},
    {5426, 2814}, {5462, 5788}};

const vector<iq_sample_t> kReferenceValuesRRC = {
    {0, 0},        {0, 0},        {0, 130},      {0, 27},      {0, -166},
    {0, 180},      {65, -27},     {13, -501},    {-83, 681},   {90, 857},
    {-79, -2326},  {-265, -1136}, {423, 9317},   {338, 16383}, {-1150, 9317},
    {-318, -1136}, {4318, -2326}, {7763, 857},   {5821, 681},  {0, -501},
    {-5822, -27},  {-7764, 180},  {-4318, -166}, {317, 27},    {1149, 0},
    {-339, 0},     {-424, 0},     {264, 0},      {13, 0},      {-90, 0},
    {82, 0},       {-14, 0}};

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

void CheckArray(const vector<iq_sample_t>& a, const vector<iq_sample_t>& b,
                sample_t tolerance) {
    EXPECT_EQ(a.size(), b.size()) << "Array size mismatch";
    for (size_t i = 0; i < a.size(); ++i) {
        EXPECT_NEAR(a[i].i, b[i].i, tolerance)
            << "Mismatch at index " << i << " (I component)";
        EXPECT_NEAR(a[i].q, b[i].q, tolerance)
            << "Mismatch at index " << i << " (Q component)";
    }
}

TEST(RNG, FirstSamples) {
    const size_t num_samples = 10;
    vector<iq_sample_t> samples(num_samples);

    rng_state_t state;
    dsp_rng_init(&state, 7500, 0x7fff, false, 1, 0);
    dsp_rng_generate_icdf(&state, samples.data(), num_samples);
    CheckArray(samples, kReferenceValuesRNG, 0);
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
    CheckArray(samples, reference, 1);
}

TEST(RRCFilter, SimpleTest) {
    sample_t lut_rrc[LUT_RRC_SIZE];
    rrc_filter_state_t rrc_state;
    dsp_rrc_filter_init(&rrc_state, &lut_rrc[0], 0.3f, 1, 2);
    vector<iq_sample_t> symbols = {{0, 16384}, {0, 0}, {8192, 0}, {0, 0},
                                   {-8192, 0}, {0, 0}, {0, 0},    {0, 0},
                                   {0, 0},     {0, 0}, {0, 0},    {0, 0},
                                   {0, 0},     {0, 0}, {0, 0},    {0, 0}};
    vector<iq_sample_t> samples(32);
    size_t consumed = dsp_rrc_filter_process(&rrc_state, symbols.data(),
                                             samples.data(), samples.size());
    EXPECT_EQ(consumed, 16) << "Wrong number of consumed samples";
    CheckArray(samples, kReferenceValuesRRC, 1);
}
