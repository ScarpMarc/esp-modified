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

#include <chrono>

#include <unistd.h>

#include <filesystem>

long long unsigned int total_sim_duration;

/**
 * @brief Calculate signal-to-noise ratio in dB to compare our output with the golden one
 *
 * @param reference Golden image
 * @param test Our generated image
 * @param num_elements Amount of values to compare
 * @return double Accumulated signal-to-noise ratio in dB
 */
double calculate_snr(
    const complex *reference,
    const complex *test,
    size_t num_elements)
{
    double num = 0.0, den = 0.0;
    size_t i;

    for (i = 0; i < num_elements; ++i)
    {
        den += (reference[i].real_part - test[i].real_part) *
               (reference[i].real_part - test[i].real_part);
        den += (reference[i].imaginary_part - test[i].imaginary_part) *
               (reference[i].imaginary_part - test[i].imaginary_part);
        num += reference[i].real_part * reference[i].real_part +
               reference[i].imaginary_part * reference[i].imaginary_part;
    }

    if (den == 0)
    {
        /*
         * The test and reference sets are identical. Just
         * return a large number (in dB) rather than +infinity.
         *
         * THIS IS A CONSTANT DEFINED IN espacc.h
         */
        return large_signal_to_noise_ratio;
    }
    else
    {
        return 10.0 * log10(num / den);
    }
}

/**
 * @brief Used for reading the golden output data file
 *
 * @param data Output array
 * @param filename Target file name
 * @param directory Directory name
 * @param num_bytes Amount of bytes to read
 */
void read_data_file(
    char *data,
    const char *filename,
    const char *directory,
    size_t num_bytes)
{
    size_t nread = 0;
    FILE *fp = NULL;
    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN];

    assert(data != NULL);
    assert(filename != NULL);
    assert(directory != NULL);

    concat_dir_and_filename(
        dir_and_filename,
        directory,
        filename);

    fp = fopen(dir_and_filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Unable to open golden input file %s for reading.\n",
                dir_and_filename);
        exit(EXIT_FAILURE);
    }

    nread = fread(data, sizeof(char), num_bytes, fp);
    if (nread != num_bytes)
    {
        fprintf(stderr, "Error: read failure on %s. "
                        "Expected %lu bytes, but only read %lu.\n",
                dir_and_filename, num_bytes, nread);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
}

/**
 * @brief Used for reading the input data file
 *
 * @param input_filename Input file name
 * @param input_directory Input directory
 * @param buffer Buffer to store the data
 */
void read_bp_data_file(
    const char *input_filename,
    const char *input_directory,
    float *buffer
    // float *fc,
    // float *R0,
    // float *dR
)
{
    FILE *fp = NULL;
    const size_t num_data_elements = N_RANGE_UPSAMPLED * N_PULSES;
    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN];
    size_t n;

    assert(input_filename != NULL);
    assert(input_directory != NULL);
    assert(buffer != NULL);
    // assert(fc != NULL);
    // assert(R0 != NULL);
    // assert(dR != NULL);

    double fc_d;
    double R0_d;
    double dR_d;

    // double buffer_d[3 * N_PULSES + // Positions
    //     2 * num_data_elements // Complex values
    //];

    double *buffer_d = (double *)malloc(sizeof(double) * (SINGLE_PLATFORM_POS_DATA_SIZE * N_PULSES));

    concat_dir_and_filename(
        dir_and_filename,
        input_directory,
        input_filename);

    printf("Reading file %s.\nExpected:\n- %u pulses\n- Platform positions from position %u, %u total\n- Range bins from position %u, %u total (%u per pulse)\n", dir_and_filename, N_PULSES,
           BUFFER_PLATFORM_POS_STARTING_IDX, N_PULSES,
           BUFFER_RANGE_BIN_STARTING_IDX, N_RANGE_UPSAMPLED * N_PULSES, N_RANGE_UPSAMPLED);

    fp = fopen(dir_and_filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Unable to open input file %s.\n",
                dir_and_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }

    printf("Reading fc\n");
    fflush(stdout);
    if (fread(&fc_d, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter fc from %s.\n",
                input_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }

    buffer[PARAM_FC_IDX] = (float)fc_d;

    printf("Reading R0\n");
    fflush(stdout);
    if (fread(&R0_d, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter R0 from %s.\n",
                input_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }

    buffer[PARAM_R0_IDX] = (float)R0_d;

    printf("Reading dR\n");
    fflush(stdout);
    if (fread(&dR_d, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter dR from %s.\n",
                input_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }

    buffer[PARAM_dR_IDX] = (float)dR_d;

    printf("Reading position data\n");
    fflush(stdout);
    if ((n = fread((buffer_d), sizeof(position_d), N_PULSES, fp)) != N_PULSES)
    {
        fprintf(stderr, "Error: Unable to read platform positions from %s.\n",
                input_filename);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }
    printf("Read %d positions\n", n);

    for (unsigned int i = 0; i < N_PULSES; ++i)
    {
        for (unsigned int j = 0; j < 3; ++j)
        {
            buffer[PLATFORM_POS_STARTING_IDX(i) + j] = (float)buffer_d[SINGLE_PLATFORM_POS_DATA_SIZE * (i) + j];
            assert(SINGLE_PLATFORM_POS_DATA_SIZE * (i) + j < _buffer_d_size);
            assert(PLATFORM_POS_STARTING_IDX(i) + j < _buffer_size);
        }
    }

    printf("Reading phase history data\n");
    fflush(stdout);
    if ((n = fread((buffer + BUFFER_RANGE_BIN_STARTING_IDX), sizeof(complex), num_data_elements, fp)) !=
        num_data_elements)
    {
        fprintf(stderr, "Error: Unable to read phase history data from %s. Managed to read %u bytes, expected %u\n",
                input_filename, n, num_data_elements);
        free(buffer_d);
        exit(EXIT_FAILURE);
    }
    printf("Read %d complex values\n", n);

    /*for (unsigned int i = 0; i < num_data_elements; ++i)
    {
        for (unsigned int j = 0; j < 2; ++j)
        {
            buffer[RANGE_BIN_STARTING_IDX(i) + j] = (float)buffer_d[N_PULSES * SINGLE_PLATFORM_POS_DATA_SIZE + SINGLE_PULSE_DATA_SIZE * (i) + j];
        }
    }*/

    printf("All saved to buffer\n");
    fflush(stdout);

    free(buffer_d);
    fclose(fp);
}

int main(int argc, char **argv)
{

   printf("****start*****\n");
#ifndef NDEBUG
    printf("--- DEBUG MODE ---\n");
#endif
    fflush(stdout);
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

    in_words_adj = round_up(BUFFER_PLATFORM_POS_STARTING_IDX + n_pulses * ((n_range_bins * 2) + 3), VALUES_PER_WORD);
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

#ifndef NDEBUG
    _buffer_size = in_size;
    _dma_size = dma_size;
    _dma_in_size = dma_in_size;
#endif

    /*read_bp_data_file(
    const char *input_filename,
    const char *input_directory,
    const std::array<complex, N_RANGE_UPSAMPLED>(*upsampled_data),
    double *fc,
    double *R0,
    double *dR)*/
    printf("Attempting to read data file...\n");
    // printf("Input buffer size (bytes): %x\n", in_size * sizeof(word_t));

    read_bp_data_file(
        input_filename,
        "/home/marco/Repos/ACAProject2022-2023/inout/", // folder
        inbuff
        //&fc,
        //&R0,
        //&dR
    );
    printf("Data file read successfully.\n");
    fflush(stdout);

    printf("Reading golden output.\n");
    fflush(stdout);

    read_data_file(
        (char *)outbuff_gold,
        golden_output_filename,
        "/home/marco/Repos/ACAProject2022-2023/inout/",
        sizeof(complex) * (out_size * out_size));

    printf("Golden output file read successfully.\n");
    fflush(stdout);    

    for (unsigned i = 0; i < dma_in_size; i++)
    {
        for (unsigned k = 0; k < VALUES_PER_WORD; k++)
        {
            mem[i].word[k] = inbuff[i * VALUES_PER_WORD + k];
            assert(i * VALUES_PER_WORD + k < in_size);
        }
    }
    printf("Generated input buffer.\n");
    fflush(stdout);

    auto top_start = std::chrono::high_resolution_clock::now();

    // Call the TOP function
    top(mem, mem,
        /* <<--args-->> */
        n_range_bins,
        out_size,
        n_pulses,
        load, store);

    auto top_end = std::chrono::high_resolution_clock::now();
    total_sim_duration = std::chrono::duration_cast<std::chrono::microseconds>(top_end - top_start).count() / 1000; // Milliseconds

    printf("Top function took %llu ms\n", total_sim_duration);

    // long long unsigned int total_sim_duration

    // Validate
    uint32_t out_offset = dma_in_size;
    for (unsigned i = 0; i < dma_out_size; i++)
        for (unsigned k = 0; k < VALUES_PER_WORD; k++)
            outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

    int errors = 0;
    /*for (unsigned i = 0; i < 1; i++)
        for (unsigned j = 0; j < 2 * out_size * out_size; j++)
            if (outbuff[i * out_words_adj + j] != outbuff_gold[i * out_words_adj + j])
                errors++;*/

    printf("Comparing outputs...\n");
    fflush(stdout);

    double snr = calculate_snr(
        (complex *)outbuff_gold,
        (complex *)outbuff,
        (out_size * out_size));
    printf("\nImage correctness SNR: %.2f\n", snr);

    if (snr < min_valid_signal_to_noise_ratio)
        errors = 1;

    if (errors)
        std::cout << "Test FAILED with " << errors << " errors." << std::endl;
    else
        std::cout << "Test PASSED." << std::endl;

    // Free memory

    // free(mem);
    free(inbuff);
    free(outbuff);
    free(outbuff_gold);

    return 0;
}
