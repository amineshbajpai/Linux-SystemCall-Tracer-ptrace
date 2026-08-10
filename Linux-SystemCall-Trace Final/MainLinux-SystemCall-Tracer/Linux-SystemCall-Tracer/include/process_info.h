#ifndef PROCESS_INFO_H
#define PROCESS_INFO_H

#include <sys/types.h>
#include <stddef.h>

/* Print executable path and current working directory */
void print_process_info(pid_t pid);

/* Read a string from the traced process memory */
void read_process_string(pid_t pid,
                         unsigned long addr,
                         char *buffer,
                         size_t size);

#endif
