// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "../inc/utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cmath>
#include <unistd.h>

#include <unistd.h>

#include <filesystem>

float fc = 0.0f, dR = 0.0f, R0 = 0.0f;
float dxdy = 0.0f; // = dR;
float ku = 0.0f;   // = 2.0 * M_PI * fc / SPEED_OF_LIGHT;

void read_bp_data_file(
    const char *input_filename,
    const char *input_directory,
    float *buffer,
    float *fc,
    float *R0,
    float *dR)
{
    FILE *fp = NULL;
    const size_t num_data_elements = N_RANGE_UPSAMPLED * N_PULSES;
    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN];
    size_t n;

    assert(input_filename != NULL);
    assert(input_directory != NULL);
    assert(buffer != NULL);
    assert(fc != NULL);
    assert(R0 != NULL);
    assert(dR != NULL);

    double fc_d;
    double R0_d;
    double dR_d;

    // double buffer_d[3 * N_PULSES + // Positions
    //     2 * num_data_elements // Complex values
    //];

    double *buffer_d = (double *)malloc(sizeof(double) * (3 * N_PULSES + 2 * num_data_elements));

    printf("Reading file:\n- %u pulses\n- Platform positions from position %u, %u total\n- Range bins from position %u, %u total\n", N_PULSES,
           BUFFER_PLATFORM_POS_STARTING_IDX, N_PULSES,
           BUFFER_RANGE_BIN_STARTING_IDX, N_RANGE);

    concat_dir_and_filename(
        dir_and_filename,
        input_directory,
        input_filename);

    printf("\t\tReading from file %s\n", dir_and_filename);
    fp = fopen(dir_and_filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Unable to open input file %s.\n",
                dir_and_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }

    if (fread(&fc_d, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter fc from %s.\n",
                input_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }

    *fc = (float)fc_d;

    if (fread(&R0_d, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter R0 from %s.\n",
                input_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }

    *R0 = (float)R0_d;

    if (fread(&dR_d, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter dR from %s.\n",
                input_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }

    *dR = (float)dR_d;

    if (fread((buffer_d + BUFFER_PLATFORM_POS_STARTING_IDX), sizeof(position_d), N_PULSES, fp) != N_PULSES)
    {
        fprintf(stderr, "Error: Unable to read platform positions from %s.\n",
                input_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }

    for (unsigned int i = 0; i < N_PULSES; ++i)
    {
        for (unsigned int j = 0; j < 3; ++j)
        {
            buffer[PLATFORM_POS_STARTING_IDX(i) + j] = (float)buffer_d[PLATFORM_POS_STARTING_IDX(i) + j];
        }
    }

    if ((n = fread((buffer_d + BUFFER_RANGE_BIN_STARTING_IDX), sizeof(complex_d), num_data_elements, fp)) !=
        num_data_elements)
    {
        fprintf(stderr, "Error: Unable to read phase history data from %s.\n",
                input_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }

    for (unsigned int i = 0; i < N_RANGE; ++i)
    {
        for (unsigned int j = 0; j < 2; ++j)
        {
            buffer[RANGE_BIN_STARTING_IDX(i) + j] = (float)buffer_d[RANGE_BIN_STARTING_IDX(i) + j];
        }
    }
    free(buffer_d);
    fclose(fp);
}

int main(int argc, char **argv)
{

    printf("****start*****\n");
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("\tCurrent working dir: %s\n", cwd);
    }
    else
    {
        perror("\tgetcwd() error");
        return 1;
    }

    /*
        DEFINE SOME CONSTANTS
    */

    /* <<--params-->> */
    const unsigned n_range_bins = N_RANGE_UPSAMPLED;
    const unsigned out_size = PFA_NOUT_RANGE;
    const unsigned n_pulses = N_PULSES;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size_;
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;

    in_words_adj = round_up(n_pulses * ((n_range_bins * 2) + 3), VALUES_PER_WORD);
    out_words_adj = round_up(2 * out_size * out_size, VALUES_PER_WORD);
    in_size = in_words_adj * (1);
    out_size_ = out_words_adj * (1);

    dma_in_size = in_size / VALUES_PER_WORD;
    dma_out_size = out_size_ / VALUES_PER_WORD;
    dma_size = dma_in_size + dma_out_size;

    dma_word_t *mem = (dma_word_t *)malloc(dma_size * sizeof(dma_word_t));
    word_t *inbuff = (word_t *)malloc(in_size * sizeof(word_t));
    word_t *outbuff = (word_t *)malloc(out_size_ * sizeof(word_t));
    word_t *outbuff_gold = (word_t *)malloc(out_size_ * sizeof(word_t));
    dma_info_t load;
    dma_info_t store;

    /*read_bp_data_file(
    const char *input_filename,
    const char *input_directory,
    const std::array<complex, N_RANGE_UPSAMPLED>(*upsampled_data),
    double *fc,
    double *R0,
    double *dR)*/
    printf("\tAttempting to read data file...\n");
    printf("\tInput buffer size (bytes): %x\n", in_size * sizeof(word_t));

    read_bp_data_file(
        input_filename,
        ".", // folder
        inbuff,
        &fc,
        &R0,
        &dR);

    printf("\tData file read successfully.\n");

    dxdy = dR;
    dR /= RANGE_UPSAMPLE_FACTOR;
    ku = 2.0 * M_PI * fc / SPEED_OF_LIGHT;

    // Call the TOP function
    top(mem, mem,
        /* <<--args-->> */
        n_range_bins,
        out_size_,
        n_pulses,
        load, store);

    // Validate
    uint32_t out_offset = dma_in_size;
    for (unsigned i = 0; i < dma_out_size; i++)
        for (unsigned k = 0; k < VALUES_PER_WORD; k++)
            outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

    int errors = 0;
    for (unsigned i = 0; i < 1; i++)
        for (unsigned j = 0; j < 2 * out_size_ * out_size_; j++)
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
