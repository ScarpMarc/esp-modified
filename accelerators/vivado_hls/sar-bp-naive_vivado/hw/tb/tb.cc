// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "../inc/utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cmath>

void read_bp_data_file(
    const char *input_filename,
    const char *input_directory,
    const std::array<complex, N_RANGE_UPSAMPLED>(*upsampled_data),
    position *platpos,
    double *fc,
    double *R0,
    double *dR)
{
    FILE *fp = NULL;
    const size_t num_data_elements = N_RANGE_UPSAMPLED * N_PULSES;
    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN];
    size_t n;

    assert(input_filename != NULL);
    assert(input_directory != NULL);
    assert(upsampled_data != NULL);
    assert(platpos != NULL);
    assert(fc != NULL);
    assert(R0 != NULL);
    assert(dR != NULL);

    concat_dir_and_filename(
        dir_and_filename,
        input_directory,
        input_filename);

    fp = fopen(dir_and_filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Unable to open input file %s.\n",
                input_filename);
        exit(EXIT_FAILURE);
    }

    if (fread(fc, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter fc from %s.\n",
                input_filename);
        exit(EXIT_FAILURE);
    }

    if (fread(R0, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter R0 from %s.\n",
                input_filename);
        exit(EXIT_FAILURE);
    }

    if (fread(dR, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter dR from %s.\n",
                input_filename);
        exit(EXIT_FAILURE);
    }

    if (fread(platpos, sizeof(position), N_PULSES, fp) != N_PULSES)
    {
        fprintf(stderr, "Error: Unable to read platform positions from %s.\n",
                input_filename);
        exit(EXIT_FAILURE);
    }

    if ((n = fread((void *)upsampled_data, sizeof(complex), num_data_elements, fp)) !=
        num_data_elements)
    {
        fprintf(stderr, "Error: Unable to read phase history data from %s.\n",
                input_filename);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
}

int main(int argc, char **argv)
{

    printf("****start*****\n");

    /*
        DEFINE SOME CONSTANTS
    */
    dxdy = dR;
    dR /= RANGE_UPSAMPLE_FACTOR;
    ku = 2.0 * M_PI * fc / SPEED_OF_LIGHT;

    /* <<--params-->> */
    const unsigned n_range_bins = 8192;
    const unsigned out_size = 1024;
    const unsigned n_pulses = 1024;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;

    in_words_adj = round_up(n_pulses * ((n_range_bins * 2) + 3), VALUES_PER_WORD);
    out_words_adj = round_up(2 * out_size * out_size, VALUES_PER_WORD);
    in_size = in_words_adj * (1);
    out_size = out_words_adj * (1);

    dma_in_size = in_size / VALUES_PER_WORD;
    dma_out_size = out_size / VALUES_PER_WORD;
    dma_size = dma_in_size + dma_out_size;

    dma_word_t *mem = (dma_word_t *)malloc(dma_size * sizeof(dma_word_t));
    word_t *inbuff = (word_t *)malloc(in_size * sizeof(word_t));
    word_t *outbuff = (word_t *)malloc(out_size * sizeof(word_t));
    word_t *outbuff_gold = (word_t *)malloc(out_size * sizeof(word_t));
    dma_info_t load;
    dma_info_t store;

    // Prepare input data
    for (unsigned i = 0; i < 1; i++)
        for (unsigned j = 0; j < n_pulses * ((n_range_bins * 2) + 3); j++)
            inbuff[i * in_words_adj + j] = (word_t)j;

    for (unsigned i = 0; i < dma_in_size; i++)
        for (unsigned k = 0; k < VALUES_PER_WORD; k++)
            mem[i].word[k] = inbuff[i * VALUES_PER_WORD + k];

    // Set golden output
    for (unsigned i = 0; i < 1; i++)
        for (unsigned j = 0; j < 2 * out_size * out_size; j++)
            outbuff_gold[i * out_words_adj + j] = (word_t)j;

    // Call the TOP function
    top(mem, mem,
        /* <<--args-->> */
        n_range_bins,
        out_size,
        n_pulses,
        load, store);

    // Validate
    uint32_t out_offset = dma_in_size;
    for (unsigned i = 0; i < dma_out_size; i++)
        for (unsigned k = 0; k < VALUES_PER_WORD; k++)
            outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

    int errors = 0;
    for (unsigned i = 0; i < 1; i++)
        for (unsigned j = 0; j < 2 * out_size * out_size; j++)
            if (outbuff[i * out_words_adj + j] != outbuff_gold[i * out_words_adj + j])
                errors++;

    if (errors)
        std::cout << "Test FAILED with " << errors << " errors." << std::endl;
    else
        std::cout << "Test PASSED." << std::endl;

    // Free memory

    free(mem);
    free(inbuff);
    free(outbuff);
    free(outbuff_gold);

    return 0;
}
