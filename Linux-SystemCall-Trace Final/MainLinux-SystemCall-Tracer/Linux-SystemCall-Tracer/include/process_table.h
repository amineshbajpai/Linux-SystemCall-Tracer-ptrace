#ifndef PROCESS_TABLE_H
#define PROCESS_TABLE_H

#include <sys/types.h>

#define MAX_PROCESSES 1024

typedef enum
{
    SYSCALL_ENTRY,
    SYSCALL_EXIT
} syscall_state_t;

typedef struct
{
    pid_t pid;

    syscall_state_t state;

    int active;

    int stopped;

    int attached;

    long current_syscall;

} traced_process_t;

void set_stopped(pid_t pid, int value);

int is_stopped(pid_t pid);

void set_attached(pid_t pid, int value);

int is_attached(pid_t pid);

void init_process_table(void);

int add_process(pid_t pid);

void remove_process(pid_t pid);

traced_process_t *find_process(pid_t pid);

int active_processes(void);



#endif
