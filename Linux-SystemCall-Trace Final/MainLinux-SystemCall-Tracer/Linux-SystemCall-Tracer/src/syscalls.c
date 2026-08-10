#include "../include/syscalls.h"
#include<sys/user.h>

const char *get_syscall_name(long syscall)
{
    switch(syscall)
    {
        case 0: return "read";
        case 1: return "write";
        case 2: return "open";
        case 3: return "close";
        case 9: return "mmap";
        case 10: return "mprotect";
        case 11: return "munmap";
        case 12: return "brk";
        case 16: return "ioctl";
        case 21: return "access";
        case 39: return "getpid";
        case 41: return "socket";
        case 42: return "connect";
        case 43: return "accept";
        case 57: return "fork";
        case 58: return "vfork";
        case 59: return "execve";
        case 60: return "exit";
        case 61: return "wait4";
        case 62: return "kill";
        case 72: return "fcntl";
        case 80: return "chdir";
        case 83: return "mkdir";
        case 87: return "unlink";
        case 89: return "readlink";
        case 158: return "arch_prctl";
        case 202: return "futex";
        case 217: return "getdents64";
        case 218: return "set_tid_address";
        case 219: return "restart_syscall";
        case 228: return "clock_gettime";
        case 230: return "clock_nanosleep";
        case 231: return "exit_group";
        case 257: return "openat";
        case 262: return "newfstatat";
        case 273: return "set_robust_list";
        case 302: return "prlimit64";
        case 318: return "getrandom";
	case 56:  return "clone";
        case 258: return "mkdirat";
        case 263: return "unlinkat";
        case 267: return "readlinkat";
        case 322: return "execveat";
        case 332: return "statx";
        case 437: return "openat2";

        default:
            return "Unknown";
    }
}
    int has_path_argument(long syscall)
{
    switch (syscall)
    {
        case 2:     /* open */
        case 59:    /* execve */
        case 80:    /* chdir */
        case 83:    /* mkdir */
        case 87:    /* unlink */
        case 89:    /* readlink */
        case 257:   /* openat */
        case 258:   /* mkdirat */
        case 263:   /* unlinkat */
        case 267:   /* readlinkat */
        case 322:   /* execveat */
        case 437:   /* openat2 */
            return 1;

        default:
            return 0;
    }
}

unsigned long get_path_argument(long syscall,
                                struct user_regs_struct *regs)
{
    switch (syscall)
    {
        case 2:     /* open */
        case 59:    /* execve */
        case 80:    /* chdir */
        case 83:    /* mkdir */
        case 87:    /* unlink */
        case 89:    /* readlink */
            return regs->rdi;

        case 257:   /* openat */
        case 258:   /* mkdirat */
        case 263:   /* unlinkat */
        case 267:   /* readlinkat */
        case 437:   /* openat2 */
            return regs->rsi;

        case 322:   /* execveat */
            return regs->rsi;

        default:
            return 0;
    }
}
