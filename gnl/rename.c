/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gnl.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mosakura <mosakura@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/22 16:11:53 by mosakura          #+#    #+#             */
/*   Updated: 2025/11/22 18:01:16 by mosakura         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"
#include <fcntl.h>           // open関数
#include <stdio.h>           // printf関数
#include <stdlib.h>          // free, EXIT_FAILURE
#include <unistd.h>          // close関数

#define TEST_FILE1 "gnl.txt"
#define TEST_FILE2 "gnl2.txt"

void safe_free(char **line)
{
    if (*line)
    {
        free(*line);
        *line = NULL;
    }
}

int main(void)
{
    int     fd1;
    int     fd2;
    int     fd_error = -1;

    char    *line1;
    char    *line2;

    fd1 = open(TEST_FILE1, O_RDONLY);
    fd2 = open(TEST_FILE2, O_RDONLY);

    if (fd1 == -1 || fd2 == -1)
    {
        perror("Error opening one or more test files. Make sure "
               TEST_FILE1 " and " TEST_FILE2 " exist.");
        if (fd1 != -1) close(fd1);
        if (fd2 != -1) close(fd2);
        return (EXIT_FAILURE);
    }

    printf("--- Multiple FD Reading Test ---\n");
    printf("FD1 (%s): %d | FD2 (%s): %d | FD_Error: %d\n",
           TEST_FILE1, fd1, TEST_FILE2, fd2, fd_error);
    printf("--------------------------------\n");

    line1 = get_next_line(fd1);
    line2 = get_next_line(fd2);

    if (line1)
        printf("FD1 (Line 1): %s", line1);
    else
        printf("FD1 (Line 1): NULL (Unexpected)\n");

    if (line2)
        printf("FD2 (Line 1): %s", line2);
    else
        printf("FD2 (Line 1): NULL (Unexpected)\n");

    safe_free(&line1);
    safe_free(&line2);

    line1 = get_next_line(fd1);
    line2 = get_next_line(fd2);

    if (line1)
        printf("FD1 (Line 2): %s", line1);
    else
        printf("FD1 (Line 2): NULL (End of File or Error)\n");

    if (line2)
        printf("FD2 (Line 2): %s", line2);
    else
        printf("FD2 (Line 2): NULL (End of File or Error)\n");

    safe_free(&line1);
    safe_free(&line2);

    line1 = get_next_line(fd1);
    line2 = get_next_line(fd2);

    if (line1)
        printf("FD1 (Line 3): %s", line1);
    else
        printf("FD1 (Line 3): NULL (End of File or Error)\n");

    if (line2)
        printf("FD2 (Line 3): %s", line2);
    else
        printf("FD2 (Line 3): NULL (End of File or Error)\n");

    safe_free(&line1);
    safe_free(&line2);

    printf("\n--- Error Check (fd: %d) ---\n", fd_error);
    char *line_error = get_next_line(fd_error);
    if (line_error == NULL)
        printf("Result for fd %d: NULL (Correct)\n", fd_error);
    else
    {
        printf("Result for fd %d: Unexpected return value (%s)\n",
               fd_error, line_error);
        safe_free(&line_error);
    }

    if (close(fd1) == -1 || close(fd2) == -1)
    {
        perror("Error closing files");
        return (EXIT_FAILURE);
    }
    printf("--------------------------------\n");
    printf("Test finished successfully.\n");

    return (0);
}


void test_one_fd(const char *filename)
{
    int fd;
    char *line;

    printf("\n>>> Starting 1 FD Test: %s (BUFFER_SIZE: %d) <<<\n", filename, BUFFER_SIZE);

    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening test file for 1 FD test");
        return;
    }
    printf("Opened FD: %d\n", fd);
    printf("----------------------------------------\n");

    // Line 1
    line = get_next_line(fd);
    if (line)
        printf("FD %d | Line 1: %s", fd, line);
    else
        printf("FD %d | Line 1: NULL (Unexpected)\n", fd);
    safe_free(&line);

    // Line 2
    line = get_next_line(fd);
    if (line)
        printf("FD %d | Line 2: %s", fd, line);
    else
        printf("FD %d | Line 2: NULL (Unexpected)\n", fd);
    safe_free(&line);

    // Line 3
    line = get_next_line(fd);
    if (line)
        printf("FD %d | Line 3: %s", fd, line);
    else
        printf("FD %d | Line 3: NULL (End of File/Error)\n", fd);
    safe_free(&line);

    // Line 4 (Should be EOF/NULL)
    line = get_next_line(fd);
    if (line == NULL)
        printf("FD %d | Line 4: NULL (Correct EOF)\n", fd);
    else
        printf("FD %d | Line 4: Unexpected return: %s\n", fd, line);
    safe_free(&line);

    // Final EOF check
    line = get_next_line(fd);
    printf("FD %d | Line 5: NULL (Post-EOF check)\n", fd);
    safe_free(&line);

    if (close(fd) == -1)
        perror("Error closing file in 1 FD test");

    printf("----------------------------------------\n");
    printf(">>> 1 FD Test finished. <<<\n");
}


/**
 * @brief Tests reading lines interleavingly from five file descriptors.
 * Manually calls GNL round-robin for three rounds, followed by EOF/Error check.
 */
void test_five_fds(void)
{
    printf("\n>>> Starting 5 FDs Interleaving Test (BUFFER_SIZE: %d) <<<\n", BUFFER_SIZE);

    const char *filenames[] = {TEST_FILE1, TEST_FILE2, TEST_FILE3, TEST_FILE4, TEST_FILE5};
    int fds[5];
    char *lines[5];
    int i, j;
    int fd_error = -1; // For error case test

    // Open all 5 files
    for (i = 0; i < 5; i++)
    {
        fds[i] = open(filenames[i], O_RDONLY);
        if (fds[i] == -1)
        {
            perror("Error opening one of the 5 test files");
            for (j = 0; j < i; j++)
                close(fds[j]);
            return;
        }
    }

    printf("Files opened: FD 1: %d, FD 2: %d, FD 3: %d, FD 4: %d, FD 5: %d\n",
           fds[0], fds[1], fds[2], fds[3], fds[4]);
    printf("----------------------------------------\n");

    // --- Round 1: Line 1 for all FDs ---
    printf("--- Round 1 ---\n");
    for (i = 0; i < 5; i++)
    {
        lines[i] = get_next_line(fds[i]);
        if (lines[i])
            printf("FD %d | L1: %s", fds[i], lines[i]);
        else
            printf("FD %d | L1: NULL (Unexpected)\n", fds[i]);
        safe_free(&lines[i]);
    }

    // --- Round 2: Line 2 for all FDs ---
    printf("--- Round 2 ---\n");
    for (i = 0; i < 5; i++)
    {
        lines[i] = get_next_line(fds[i]);
        if (lines[i])
            printf("FD %d | L2: %s", fds[i], lines[i]);
        else
            printf("FD %d | L2: NULL (File %d likely only has 1 line)\n", fds[i], i + 1);
        safe_free(&lines[i]);
    }

    // --- Round 3: Line 3 for all FDs ---
    printf("--- Round 3 ---\n");
    for (i = 0; i < 5; i++)
    {
        lines[i] = get_next_line(fds[i]);
        if (lines[i])
            printf("FD %d | L3: %s", fds[i], lines[i]);
        else
            printf("FD %d | L3: NULL (End of File/Error)\n", fds[i]);
        safe_free(&lines[i]);
    }

    // --- Error Check (fd: -1) ---
    char *line_error = get_next_line(fd_error);
    printf("\n--- Error Check (fd: %d) ---\n", fd_error);
    if (line_error == NULL)
        printf("Result for fd %d: NULL (Correct)\n", fd_error);
    else
    {
        printf("Result for fd %d: Unexpected return value (%s)\n", fd_error, line_error);
        safe_free(&line_error);
    }

    // Close all files
    for (i = 0; i < 5; i++)
    {
        if (close(fds[i]) == -1)
            perror("Error closing file in 5 FDs test");
    }
    printf("----------------------------------------\n");
    printf(">>> 5 FDs Test finished. <<<\n");
}
