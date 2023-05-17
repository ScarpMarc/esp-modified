#include "../inc/utils.h"

complex cconj(complex x)
{
    complex xconj = x;
    xconj.im *= -1.0f;
    return xconj;
}

complex cmult(complex lhs, complex rhs)
{
    complex prod;
    prod.re = lhs.re * rhs.re - lhs.im * rhs.im;
    prod.im = lhs.re * rhs.im + lhs.im * rhs.re;
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