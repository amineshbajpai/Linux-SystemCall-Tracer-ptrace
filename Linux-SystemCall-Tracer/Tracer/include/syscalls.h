#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <sys/user.h>

const char *get_syscall_name(long syscall);

int has_path_argument(long syscall);

unsigned long get_path_argument(long syscall, struct user_regs_struct *regs);

const char *get_syscall_name(long syscall_no);

#endif
