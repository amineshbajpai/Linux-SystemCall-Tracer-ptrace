#include<stddef.h>
#include "../include/process_table.h"

static traced_process_t process_table[MAX_PROCESSES];

void init_process_table(void)
{
    for(int i = 0; i < MAX_PROCESSES; i++)
    {
        process_table[i].active = 0;
        process_table[i].pid = -1;
        process_table[i].state = SYSCALL_ENTRY;
        process_table[i].stopped = 0;
    }
}

void set_attached(pid_t pid, int value)
{
    traced_process_t *proc = find_process(pid);

    if(proc)
        proc->attached = value;
}

int is_attached(pid_t pid)
{
    traced_process_t *proc = find_process(pid);

    if(proc)
        return proc->attached;

    return 0;
}

int add_process(pid_t pid)
{
    for(int i = 0; i < MAX_PROCESSES; i++)
    {
        if(process_table[i].active == 0)
        {
            process_table[i].active = 1;
            process_table[i].pid = pid;
            process_table[i].state = SYSCALL_ENTRY;
	    process_table[i].attached=0;
            process_table[i].stopped = 0;
            process_table[i].current_syscall = -1;


            return 0;
        }
    }

    return -1;
}

void remove_process(pid_t pid)
{
    for(int i = 0; i < MAX_PROCESSES; i++)
    {
        if(process_table[i].active &&
           process_table[i].pid == pid)
        {
            process_table[i].active = 0;
            process_table[i].pid = -1;
            process_table[i].state = SYSCALL_ENTRY;
	    process_table[i].attached=0;
	    process_table[i].current_syscall = -1;
            process_table[i].stopped = 0;
            return;
        }
    }
}

traced_process_t *find_process(pid_t pid)
{
    for(int i = 0; i < MAX_PROCESSES; i++)
    {
        if(process_table[i].active &&
           process_table[i].pid == pid)
            return &process_table[i];
    }

    return NULL;
}

int active_processes(void)
{
    int count=0;

    for(int i=0;i<MAX_PROCESSES;i++)
    {
        if(process_table[i].active)
            count++;
    }

    return count;
}

void set_stopped(pid_t pid, int value)
{
    traced_process_t *p = find_process(pid);

    if(p)
        p->stopped = value;
}

int is_stopped(pid_t pid)
{
    traced_process_t *p = find_process(pid);

    if(p)
        return p->stopped;

    return 0;
}
