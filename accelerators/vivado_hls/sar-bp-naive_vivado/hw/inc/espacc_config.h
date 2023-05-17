// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef INC_ESPACC_CONFIG_H
#define INC_ESPACC_CONFIG_H

// User defined constants
#define INPUT_SIZE_SMALL 1
#define INPUT_SIZE_MEDIUM 2
#define INPUT_SIZE_LARGE 3

#ifndef INPUT_SIZE
    #define INPUT_SIZE INPUT_SIZE_MEDIUM
#endif

#if INPUT_SIZE == INPUT_SIZE_SMALL
    #define N_RANGE (512)
    #define N_PULSES (512)
    #define BP_NPIX_X (512)
    #define BP_NPIX_Y (512)
    #define PFA_NOUT_RANGE (512)
    #define PFA_NOUT_AZIMUTH (512)
#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    #define N_RANGE (1024)
    #define N_PULSES (1024)
    #define BP_NPIX_X (1024)
    #define BP_NPIX_Y (1024)
    #define PFA_NOUT_RANGE (1024)
    #define PFA_NOUT_AZIMUTH (1024)
#elif INPUT_SIZE == INPUT_SIZE_LARGE
    #define N_RANGE (2048)
    #define N_PULSES (2048)
    #define BP_NPIX_X (2048)
    #define BP_NPIX_Y (2048)
    #define PFA_NOUT_RANGE (2048)
    #define PFA_NOUT_AZIMUTH (2048)
#else
    #error "Unhandled value for INPUT_SIZE"
#endif
// From SAR suite
#define RANGE_UPSAMPLE_FACTOR (8)

#define N_RANGE_UPSAMPLED (N_RANGE * RANGE_UPSAMPLE_FACTOR)


// Data type

#define IS_TYPE_FIXED_POINT 0
#define FRAC_BITS 0
#define IS_TYPE_UINT 0
#define IS_TYPE_INT 1
#define IS_TYPE_FLOAT 0

// In/out arrays

#define SIZE_IN_CHUNK_DATA 67115008

#define SIZE_OUT_CHUNK_DATA 8388608

#endif
