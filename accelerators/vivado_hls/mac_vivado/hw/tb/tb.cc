// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{

    printf("****start*****\n");

    /* <<--params-->> */
    const unsigned mac_n = 1;
    const unsigned mac_len = 64;
    const unsigned mac_vec = 100;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;

    in_words_adj = round_up(6400, VALUES_PER_WORD);
    out_words_adj = round_up(100, VALUES_PER_WORD);
    in_size = in_words_adj * (16);
    out_size = out_words_adj * (16);

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
    for (unsigned i = 0; i < mac_n; i++)
        for (unsigned j = 0; j < mac_len * mac_vec; j++)
            inbuff[i * in_words_adj + j] = (word_t)j % mac_vec;

    for (unsigned i = 0; i < dma_in_size; i++)
        for (unsigned k = 0; k < VALUES_PER_WORD; k++)
            mem[i].word[k] = inbuff[i * VALUES_PER_WORD + k];

    // Set golden output
    for (int i = 0; i < mac_n; i++)
        for (int j = 0; j < mac_vec; j++)
        {
            outbuff_gold[i * out_words_adj + j] = 0;
            for (int k = 0; k < mac_len; k += 2)
                outbuff_gold[i * out_words_adj + j] +=
                    inbuff[i * in_words_adj + j * mac_len + k] * inbuff[i * in_words_adj + j * mac_len + k + 1];
        }

    // Call the TOP function
    top(mem, mem,
        /* <<--args-->> */
        mac_n,
        mac_len,
        mac_vec,
        load, store);

    // Validate
    uint32_t out_offset = dma_in_size;
    for (unsigned i = 0; i < dma_out_size; i++)
        for (unsigned k = 0; k < VALUES_PER_WORD; k++)
            outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

    int errors = 0;
    for (unsigned i = 0; i < 16; i++)
        for (unsigned j = 0; j < 100; j++)
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
