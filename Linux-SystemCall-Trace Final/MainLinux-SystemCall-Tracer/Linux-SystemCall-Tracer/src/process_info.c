#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <errno.h>

#include "../include/process_info.h"

/*----------------------------------------------------------
    Print Executable Path and Current Working Directory
-----------------------------------------------------------*/

void print_process_info(pid_t pid)
{
    char proc_link[64];
    char path[512];
    ssize_t len;

    /* Executable Path */

    snprintf(proc_link,
             sizeof(proc_link),
             "/proc/%d/exe",
             pid);

    len = readlink(proc_link,
                   path,
                   sizeof(path) - 1);

    if (len != -1)
    {
        path[len] = '\0';

        printf("Executable      : %s\n", path);
    }
    else
    {
        printf("Executable      : Unknown\n");
    }

    /* Current Working Directory */

    snprintf(proc_link,
             sizeof(proc_link),
             "/proc/%d/cwd",
             pid);

    len = readlink(proc_link,
                   path,
                   sizeof(path) - 1);

    if (len != -1)
    {
        path[len] = '\0';

        printf("Current Dir     : %s\n", path);
    }
    else
    {
        printf("Current Dir     : Unknown\n");
    }
}

/*----------------------------------------------------------
    Read String From Target Process Memory
-----------------------------------------------------------*/

void read_process_string(pid_t pid,
                         unsigned long addr,
                         char *buffer,
                         size_t size)
{
    union
    {
        long value;
        char chars[sizeof(long)];
    } data;

    size_t copied = 0;

    if (size == 0)
        return;

    buffer[0] = '\0';

    while (copied < size - 1)
    {
        errno = 0;

        data.value = ptrace(PTRACE_PEEKDATA,
                            pid,
                            (void *)(addr + copied),
                            NULL);

        if (errno != 0)
            break;

        size_t remaining = size - 1 - copied;
        size_t chunk_size = remaining < sizeof(long) ? remaining : sizeof(long);

        memcpy(buffer + copied, data.chars, chunk_size);

        if (memchr(data.chars, '\0', chunk_size) != NULL)
            break;

        copied += chunk_size;
    }

    buffer[size - 1] = '\0';
}
