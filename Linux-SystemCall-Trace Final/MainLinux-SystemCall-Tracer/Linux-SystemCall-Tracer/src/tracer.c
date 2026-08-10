#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/user.h>
#include "../include/syscalls.h"
#include "../include/process_info.h"
#include "../include/process_table.h"

static void handle_ptrace_event(pid_t current_pid, int status);
static void handle_syscall(pid_t pid,struct user_regs_struct *regs);

static int attach_process(pid_t pid);
static void trace_loop(void);
static void detach_process(pid_t pid);
static void close_log(void);
static void write_json_string(const char *value);
static void write_timestamp(void);
static void get_executable_path(pid_t pid, char *path, size_t path_size);
static void log_process_event(const char *event, pid_t parent_pid, pid_t child_pid);
static void log_signal_delivery(pid_t pid, int signal_number);
static const char *signal_name(int signal_number);

static void print_entry(pid_t pid,
                        struct user_regs_struct *regs);

static void print_exit(pid_t pid, long syscall, struct user_regs_struct *regs);
static FILE *log = NULL;
static int first_event = 1;

int main(void)
{
    log = fopen("logs/trace_log.json", "w");
if (log == NULL) {
    perror("Cannot open log file");
    return EXIT_FAILURE;
}
    int pid_input;

    printf("=====================================\n");
    printf(" Linux Real-Time System Call Tracer\n");
    printf("=====================================\n\n");

    printf("Enter Target Process PID : ");
    if (scanf("%d", &pid_input) != 1 || pid_input <= 0) {
        fprintf(stderr, "Please enter a positive process ID.\n");
        close_log();
        return EXIT_FAILURE;
    }

    pid_t pid = (pid_t)pid_input;
    fprintf(log,
        "{\n"
        "  \"trace_info\": {\n"
        "    \"tracer\": \"Linux System Call Tracer\",\n"
        "    \"started_at\": ");
    write_timestamp();
    fprintf(log,
        ",\n"
        "    \"root_pid\": %d\n"
        "  },\n"
        "  \"events\": [\n",
        pid);
    fflush(log);

    init_process_table();

    if(attach_process(pid)!=0) {
        close_log();
        return EXIT_FAILURE;
    }

    if (add_process(pid) != 0) {
        fprintf(stderr, "Unable to track process %d.\n", pid);
        detach_process(pid);
        close_log();
        return EXIT_FAILURE;
    }

    char executable[512];
    get_executable_path(pid, executable, sizeof(executable));
    fprintf(log,
        "    {\n"
        "      \"type\": \"process_start\",\n"
        "      \"pid\": %d,\n"
        "      \"executable\": ", pid);
    write_json_string(executable);
    fprintf(log, "\n    }");
    first_event = 0;
    fflush(log);

    trace_loop();

    detach_process(pid);
    close_log();

    return EXIT_SUCCESS;
}

static int attach_process(pid_t pid)
{
    printf("\nTrying to attach to PID %d...\n",pid);

    if(ptrace(PTRACE_ATTACH,pid,NULL,NULL)==-1)
    {
        printf("Error : %s\n",strerror(errno));
        return -1;
    }

    int status;

    waitpid(pid,&status,0);

    if (ptrace(PTRACE_SETOPTIONS,
           pid,
           NULL,
	   PTRACE_O_TRACESYSGOOD |
           PTRACE_O_TRACEFORK |
           PTRACE_O_TRACEVFORK |
	   PTRACE_O_TRACEVFORKDONE |
           PTRACE_O_TRACECLONE |
           PTRACE_O_TRACEEXEC |
           PTRACE_O_TRACEEXIT) == -1)
{
    perror("PTRACE_SETOPTIONS");
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    return -1;
}

    printf("Successfully attached!\n");
    printf("Process Stopped.\n");
    printf("Starting System Call Tracing...\n");

	if(ptrace(PTRACE_SYSCALL, pid, NULL, NULL)==-1)
{
    perror("PTRACE_SYSCALL");
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    return -1;
}

    return 0;
}

static void detach_process(pid_t pid)
{
    if (find_process(pid) != NULL &&
        ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1 && errno != ESRCH)
        perror("PTRACE_DETACH");
    else
        printf("\nDetached Successfully\n");
}

static void close_log(void)
{
    if (log == NULL)
        return;

    fprintf(log, "\n  ]\n}\n");
    fclose(log);
    log = NULL;
}

static void write_json_string(const char *value)
{
    const unsigned char *current = (const unsigned char *)(value ? value : "");

    fputc('"', log);
    while (*current != '\0') {
        switch (*current) {
            case '"':  fputs("\\\"", log); break;
            case '\\': fputs("\\\\", log); break;
            case '\b': fputs("\\b", log); break;
            case '\f': fputs("\\f", log); break;
            case '\n': fputs("\\n", log); break;
            case '\r': fputs("\\r", log); break;
            case '\t': fputs("\\t", log); break;
            default:
                if (*current < 0x20)
                    fprintf(log, "\\u%04x", *current);
                else
                    fputc(*current, log);
        }
        current++;
    }
    fputc('"', log);
}

static void write_timestamp(void)
{
    time_t now = time(NULL);
    struct tm utc;
    char timestamp[32];

    if (gmtime_r(&now, &utc) == NULL ||
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &utc) == 0)
        strcpy(timestamp, "unknown");

    write_json_string(timestamp);
}

static void get_executable_path(pid_t pid, char *path, size_t path_size)
{
    char proc_link[64];
    ssize_t length;

    if (path_size == 0)
        return;

    snprintf(proc_link, sizeof(proc_link), "/proc/%d/exe", pid);
    length = readlink(proc_link, path, path_size - 1);
    if (length < 0)
        strcpy(path, "unknown");
    else
        path[length] = '\0';
}

static const char *signal_name(int signal_number)
{
    switch (signal_number) {
        case SIGKILL: return "SIGKILL";
        case SIGTERM: return "SIGTERM";
        case SIGSEGV: return "SIGSEGV";
        case SIGABRT: return "SIGABRT";
        case SIGINT: return "SIGINT";
        case SIGQUIT: return "SIGQUIT";
        default: return "UNKNOWN_SIGNAL";
    }
}

static void log_process_event(const char *event, pid_t parent_pid, pid_t child_pid)
{
    if (log == NULL)
        return;

    if (!first_event)
        fprintf(log, ",\n");

    fprintf(log,
        "    {\n"
        "      \"type\": \"process_event\",\n"
        "      \"event\": ");
    write_json_string(event);
    fprintf(log, ",\n      \"parent_pid\": %d", parent_pid);
    if (child_pid > 0)
        fprintf(log, ",\n      \"child_pid\": %d", child_pid);
    fprintf(log, "\n    }");

    first_event = 0;
    fflush(log);
}

static void log_signal_delivery(pid_t pid, int signal_number)
{
    if (log == NULL)
        return;

    if (!first_event)
        fprintf(log, ",\n");

    fprintf(log,
        "    {\n"
        "      \"type\": \"process_event\",\n"
        "      \"event\": \"signal_received\",\n"
        "      \"pid\": %d,\n"
        "      \"signal\": \"%s\",\n"
        "      \"signal_number\": %d\n"
        "    }",
        pid, signal_name(signal_number), signal_number);
    first_event = 0;
    fflush(log);
}

static void trace_loop(void)
{
    int status;
    pid_t current_pid;
    struct user_regs_struct regs;

    while(active_processes() > 0)
    {
        current_pid = waitpid(-1, &status, __WALL);

        if(current_pid == -1)
        {
            if(errno == EINTR)
                continue;

            perror("waitpid");
            break;
        }

        /* Process exited normally */
        if(WIFEXITED(status))
	{
    	traced_process_t *proc = find_process(current_pid);

    	if(proc && proc->state == SYSCALL_EXIT)
    	{
        	printf("\n");
        	printf("Process %d exited while executing syscall %s\n", current_pid, get_syscall_name(proc->current_syscall));
    	}

    		printf("Process %d exited.\n", current_pid);

    		remove_process(current_pid);

    		continue;
	}

        /* Process killed by signal */
        if(WIFSIGNALED(status))
	{
    		traced_process_t *proc = find_process(current_pid);

    	if(proc && proc->state == SYSCALL_EXIT)
    	{
        	printf("\n");
        	printf("Process %d killed while executing syscall %s\n", current_pid, get_syscall_name(proc->current_syscall));
    	}

		printf("Process %d killed by signal %d\n", current_pid, WTERMSIG(status));

            if (log) {
                if (!first_event)
                    fprintf(log, ",\n");

                fprintf(log,
                    "    {\n"
                    "      \"type\": \"process_event\",\n"
                    "      \"event\": \"%s\",\n"
                    "      \"pid\": %d,\n"
                    "      \"signal\": \"%s\",\n"
                    "      \"signal_number\": %d",
                    (proc && proc->state == SYSCALL_EXIT)
                        ? "terminated_during_syscall" : "terminated",
                    current_pid,
                    signal_name(WTERMSIG(status)),
                    WTERMSIG(status));
                if (proc && proc->state == SYSCALL_EXIT)
                    fprintf(log, ",\n      \"syscall\": \"%s\"",
                            get_syscall_name(proc->current_syscall));
                fprintf(log, "\n    }");
                first_event = 0;
                fflush(log);
            }

    		remove_process(current_pid);

    		continue;
	}

        if(!WIFSTOPPED(status))
            continue;

        int sig = WSTOPSIG(status);
        unsigned int event = (unsigned int)status >> 16;

        /*
         * Handle ptrace events first
         * (fork/clone/vfork/exec/exit)
         */
        if(event != 0)
        {
            handle_ptrace_event(current_pid, status);

            if(ptrace(PTRACE_SYSCALL,
                      current_pid,
                      NULL,
                      NULL) == -1)
            {
                if(errno == ESRCH)
                    remove_process(current_pid);

                continue;
            }

            continue;
        }

        /*
         * Ignore initial SIGSTOP from new children.
         */
        if(sig == SIGSTOP)
	{
    	/*
     	* First SIGSTOP after attaching.
     	* Ignore it.
     	*/
            if(!is_attached(current_pid))
    {
        set_attached(current_pid, 1);

        if (ptrace(PTRACE_SYSCALL, current_pid, NULL, NULL) == -1 && errno != ESRCH)
            perror("PTRACE_SYSCALL");

        	continue;
    	}

    	/*
     	* Real user SIGSTOP.
     	*/
    		if(!is_stopped(current_pid))
    	{
        	printf("\n");
        	printf("============================================================\n");
        	printf("                [ PROCESS PAUSED ]\n");
        	printf("============================================================\n");

        	printf("PID : %d\n", current_pid);
            if (log) {
    if (!first_event)
        fprintf(log, ",\n");

    fprintf(log,
        "    {\n"
        "      \"type\": \"process_event\",\n"
        "      \"event\": \"signal\",\n"
        "      \"pid\": %d,\n"
        "      \"signal\": \"SIGSTOP\",\n"
        "      \"signal_number\": 19,\n"
        "      \"action\": \"paused\"\n"
        "    }",
        current_pid);

    first_event = 0;
    fflush(log);
}
        	set_stopped(current_pid, 1);
    	}

    		ptrace(PTRACE_SYSCALL, current_pid, NULL, SIGSTOP);

    		continue;
	}

        /*
         * Only syscall stops.
         */


	if(sig == SIGCONT)
	{
    		if(is_stopped(current_pid))
    	{
        	printf("\n");
        	printf("============================================================\n");
        	printf("                [ PROCESS RESUMED ]\n");
        	printf("============================================================\n");

        	printf("PID : %d\n", current_pid);
            if (log) {
    if (!first_event)
        fprintf(log, ",\n");

    fprintf(log,
        "    {\n"
        "      \"type\": \"process_event\",\n"
        "      \"event\": \"signal\",\n"
        "      \"pid\": %d,\n"
        "      \"signal\": \"SIGCONT\",\n"
        "      \"signal_number\": 18,\n"
        "      \"action\": \"resumed\"\n"
        "    }",
        current_pid);

    first_event = 0;
    fflush(log);
}
        	set_stopped(current_pid, 0);
    	}

    		ptrace(PTRACE_SYSCALL, current_pid, NULL, SIGCONT);

    		continue;
	}
        if(sig == (SIGTRAP | 0x80))
        {
            if(ptrace(PTRACE_GETREGS,
                      current_pid,
                      NULL,
                      &regs) == -1)
            {
                if(errno == ESRCH)
                {
                    remove_process(current_pid);
                    continue;
                }

                perror("PTRACE_GETREGS");
                continue;
            }

            handle_syscall(current_pid, &regs);
        }

        /*
         * Deliver real signals.
         */
        if(sig != (SIGTRAP | 0x80) &&
           sig != SIGTRAP &&
           sig != SIGSTOP)
        {
            log_signal_delivery(current_pid, sig);
            ptrace(PTRACE_SYSCALL,
                   current_pid,
                   NULL,
                   sig);
        }
        else
        {
            ptrace(PTRACE_SYSCALL,
                   current_pid,
                   NULL,
                   NULL);
        }
    }
}

static void print_entry(pid_t pid,
                        struct user_regs_struct *regs)
{
    char filename[512];

    printf("\n");
    printf("============================================================\n");
    printf("                      [ ENTRY ]\n");
    printf("============================================================\n");

    printf("PID             : %d\n",pid);

    print_process_info(pid);

    printf("System Call     : %s\n",
            get_syscall_name(regs->orig_rax));

    printf("Syscall Number  : %lld\n",
            regs->orig_rax);

    memset(filename,0,sizeof(filename));

    if(has_path_argument(regs->orig_rax))
    {
        unsigned long addr=
            get_path_argument(regs->orig_rax,regs);

        if(addr)
        {
            read_process_string(pid,
                                addr,
                                filename,
                                sizeof(filename));

            printf("Path            : %s\n",
                    filename);
        }
    }

    printf("\nArguments\n");
    printf("------------------------------------------------------------\n");

    printf("RDI : 0x%llx\n",regs->rdi);
    printf("RSI : 0x%llx\n",regs->rsi);
    printf("RDX : 0x%llx\n",regs->rdx);
    printf("R10 : 0x%llx\n",regs->r10);
    printf("R8  : 0x%llx\n",regs->r8);
    printf("R9  : 0x%llx\n",regs->r9);
    if (log) {
    if (!first_event)
        fprintf(log, ",\n");

    fprintf(log,
        "    {\n"
        "      \"type\": \"syscall\",\n"
        "      \"phase\": \"entry\",\n"
        "      \"pid\": %d,\n"
        "      \"syscall\": \"%s\",\n"
        "      \"syscall_number\": %lld",
        pid,
        get_syscall_name(regs->orig_rax),
        regs->orig_rax);
    if (filename[0] != '\0') {
        fprintf(log, ",\n      \"path\": ");
        write_json_string(filename);
    }
    fprintf(log,
        ",\n"
        "      \"arguments\": {\n"
        "        \"rdi\": \"0x%llx\",\n"
        "        \"rsi\": \"0x%llx\",\n"
        "        \"rdx\": \"0x%llx\",\n"
        "        \"r10\": \"0x%llx\",\n"
        "        \"r8\": \"0x%llx\",\n"
        "        \"r9\": \"0x%llx\"\n"
        "      }\n"
        "    }",
        regs->rdi, regs->rsi, regs->rdx,
        regs->r10, regs->r8, regs->r9);

    first_event = 0;
    fflush(log);
}
}

static void print_exit(pid_t pid, long syscall, struct user_regs_struct *regs)
{
    printf("\n");
    printf("------------------------------------------------------------\n");
    printf("                      [ EXIT ]\n");
    printf("------------------------------------------------------------\n");

    if (log) {
        if (!first_event)
            fprintf(log, ",\n");

        fprintf(log,
            "    {\n"
            "      \"type\": \"syscall\",\n"
            "      \"phase\": \"exit\",\n"
            "      \"pid\": %d,\n"
            "      \"syscall\": \"%s\",\n"
            "      \"syscall_number\": %ld,\n"
            "      \"return_value\": %lld,\n"
            "      \"status\": \"%s\"\n"
            "    }",
            pid,
            get_syscall_name(syscall),
            syscall,
            regs->rax,
            ((long long)regs->rax < 0) ? "FAILED" : "SUCCESS");

        first_event = 0;
        fflush(log);
    }

    printf("Return Value    : %lld\n", regs->rax);

    if ((long long)regs->rax < 0)
        printf("Status          : FAILED\n");
    else
        printf("Status          : SUCCESS\n");

    printf("============================================================\n");
}

static void handle_syscall(pid_t pid, struct user_regs_struct *regs)
{
    traced_process_t *process = find_process(pid);

    if(process == NULL)
        return;

    if(process->state == SYSCALL_ENTRY)
      {
    	process->current_syscall = regs->orig_rax;

    	print_entry(pid, regs);

    	process->state = SYSCALL_EXIT;
      }
     
	else
      {
    	print_exit(pid, process->current_syscall, regs);

    	process->current_syscall = -1;

    	process->state = SYSCALL_ENTRY;
      }
}

static void handle_ptrace_event(pid_t current_pid, int status)
{
    unsigned int event;
    unsigned long new_pid;

    event = (unsigned int)status >> 16;

    switch(event)
    {
        case PTRACE_EVENT_FORK:
        {
            if (ptrace(PTRACE_GETEVENTMSG,
                       current_pid,
                       NULL,
                       &new_pid) == -1)
            {
                perror("PTRACE_GETEVENTMSG");
                return;
            }

            printf("\n");
            printf("============================================================\n");
            printf("                    FORK EVENT DETECTED\n");
            printf("============================================================\n");
            printf("Parent PID : %d\n", current_pid);
            printf("Child PID  : %lu\n", new_pid);
            printf("============================================================\n");

            log_process_event("fork", current_pid, (pid_t)new_pid);
    
            if(add_process((pid_t)new_pid) == 0)
            {
                traced_process_t *child = find_process((pid_t)new_pid);

                if(child)
                {
                    child->state = SYSCALL_ENTRY;
                    child->stopped = 0;
                }

            }

            break;
        }

        case PTRACE_EVENT_VFORK:
        {
            if (ptrace(PTRACE_GETEVENTMSG,
                       current_pid,
                       NULL,
                       &new_pid) == -1)
            {
                perror("PTRACE_GETEVENTMSG");
                return;
            }

            printf("\n");
            printf("============================================================\n");
            printf("                   VFORK EVENT DETECTED\n");
            printf("============================================================\n");
            printf("Parent PID : %d\n", current_pid);
            printf("Child PID  : %lu\n", new_pid);
            printf("============================================================\n");

            log_process_event("vfork", current_pid, (pid_t)new_pid);

            if(add_process((pid_t)new_pid) == 0)
            {
                traced_process_t *child = find_process((pid_t)new_pid);

                if(child)
                {
                    child->state = SYSCALL_ENTRY;
                    child->stopped = 0;
                }

            }

            break;
        }

        case PTRACE_EVENT_CLONE:
        {
            if (ptrace(PTRACE_GETEVENTMSG,
                       current_pid,
                       NULL,
                       &new_pid) == -1)
            {
                perror("PTRACE_GETEVENTMSG");
                return;
            }

            printf("\n");
            printf("============================================================\n");
            printf("                   CLONE EVENT DETECTED\n");
            printf("============================================================\n");
            printf("Parent PID : %d\n", current_pid);
            printf("Child PID  : %lu\n", new_pid);
            printf("============================================================\n");

            log_process_event("clone", current_pid, (pid_t)new_pid);

            if(add_process((pid_t)new_pid) == 0)
            {
                traced_process_t *child = find_process((pid_t)new_pid);

                if(child)
                {
                    child->state = SYSCALL_ENTRY;
                    child->stopped = 0;
                }

            }

            break;
        }

        case PTRACE_EVENT_EXEC:
        {
            printf("\n");
            printf("============================================================\n");
            printf("                    EXEC EVENT DETECTED\n");
            printf("============================================================\n");
            printf("PID : %d\n", current_pid);
            printf("============================================================\n");
            log_process_event("exec", current_pid, -1);
            break;
        }

        case PTRACE_EVENT_EXIT:
        {
            printf("\n");
            printf("============================================================\n");
            printf("                    EXIT EVENT DETECTED\n");
            printf("============================================================\n");
            printf("PID : %d\n", current_pid);
            printf("============================================================\n");
            log_process_event("exit", current_pid, -1);
            break;
        }

        default:
            break;
    }
}
