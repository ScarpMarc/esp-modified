#ifndef INC_UTILS_H
#define INC_UTILS_H

#include "espacc.h"

/* appends filename to directory */
void concat_dir_and_filename(
    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN],
    const char *directory,
    const char *filename);

/* complex conjugate */
complex cconj(complex x);

/* complex multiplication */
complex cmult(complex lhs, complex rhs);

//void reshape_data_to_fpga_format(word_t *inout_data);
//void reshape_data_to_original_format(word_t *inout_data);

extern long long unsigned int total_sim_duration;

#endif