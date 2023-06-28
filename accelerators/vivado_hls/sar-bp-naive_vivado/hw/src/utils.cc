#include "../inc/utils.h"
#include <cassert>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

complex cconj(complex x)
{
    complex xconj = x;
    xconj.imaginary_part = -xconj.imaginary_part;
    return xconj;
}

complex_d cconj(complex_d x)
{
    complex_d xconj = x;
    xconj.imaginary_part = -xconj.imaginary_part;
    return xconj;
}

complex cmult(complex lhs, complex rhs)
{
    complex prod;
    prod.real_part = lhs.real_part * rhs.real_part - lhs.imaginary_part * rhs.imaginary_part;
    prod.imaginary_part = lhs.real_part * rhs.imaginary_part + lhs.imaginary_part * rhs.real_part;
    return prod;
}

complex_d cmult(complex_d lhs, complex_d rhs)
{
    complex_d prod;
    prod.real_part = lhs.real_part * rhs.real_part - lhs.imaginary_part * rhs.imaginary_part;
    prod.imaginary_part = lhs.real_part * rhs.imaginary_part + lhs.imaginary_part * rhs.real_part;
    return prod;
}

void concat_dir_and_filename(
    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN],
    const char *directory,
    const char *filename)
{
    assert(dir_and_filename != NULL);
    assert(directory != NULL);
    assert(filename != NULL);

    /* C89 lacks snprintf */
    if (strlen(directory) + strlen(filename) + 2 > MAX_DIR_AND_FILENAME_LEN)
    {
        fprintf(stderr, "Error: input directory (%s) too long.\n",
                directory);
        exit(EXIT_FAILURE);
    }
    dir_and_filename[0] = '\0';
    strncpy(dir_and_filename, directory, MAX_DIR_AND_FILENAME_LEN - 1);
    dir_and_filename[MAX_DIR_AND_FILENAME_LEN - 1] = '\0';
    strncat(dir_and_filename, "/",
            MAX_DIR_AND_FILENAME_LEN - strlen(dir_and_filename) - 1);
    strncat(dir_and_filename, filename,
            MAX_DIR_AND_FILENAME_LEN - strlen(dir_and_filename) - 1);
}
