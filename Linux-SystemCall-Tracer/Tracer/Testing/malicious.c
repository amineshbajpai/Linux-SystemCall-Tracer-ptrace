#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main()
{
    pid_t child;
    int fd;
    int sock;
    char buffer[256];
    void *memory;

    printf("\n========================================\n");
    printf("     SECURITY TRACER TEST PROGRAM\n");
    printf("========================================\n");

    printf("PID  : %d\n", getpid());
    printf("PPID : %d\n", getppid());

    fflush(stdout);

    /*
     * Keep the process alive so the tracer
     * has enough time to attach.
     */
    printf("\nWaiting 10 seconds for tracer...\n");
    sleep(10);

    /*
     * File activity
     */
    printf("\n[1] Creating temporary file...\n");

    fd = open("/tmp/tracer_test_file.txt",
              O_CREAT | O_RDWR | O_TRUNC,
              0644);

    if (fd < 0)
    {
        perror("open");
    }
    else
    {
        strcpy(buffer, "System Call Tracer Security Test\n");

        write(fd, buffer, strlen(buffer));

        lseek(fd, 0, SEEK_SET);

        memset(buffer, 0, sizeof(buffer));

        read(fd, buffer, sizeof(buffer) - 1);

        printf("File contents: %s", buffer);

        close(fd);
    }

    sleep(3);

    /*
     * Memory activity
     */
    printf("\n[2] Allocating memory...\n");

    memory = mmap(NULL,
                  4096,
                  PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS,
                  -1,
                  0);

    if (memory != MAP_FAILED)
    {
        strcpy((char *)memory, "Tracer memory test");

        printf("Memory contains: %s\n",
               (char *)memory);

        munmap(memory, 4096);
    }

    sleep(3);

    /*
     * Create a child process.
     */
    printf("\n[3] Creating child process...\n");

    child = fork();

    if (child < 0)
    {
        perror("fork");
    }
    else if (child == 0)
    {
        printf("CHILD PROCESS\n");
        printf("Child PID  : %d\n", getpid());
        printf("Parent PID : %d\n", getppid());

        sleep(5);

        printf("Child exiting...\n");

        exit(0);
    }
    else
    {
        printf("Parent created child PID: %d\n",
               child);

        wait(NULL);

        printf("Child finished.\n");
    }

    sleep(2);

    /*
     * Socket activity.
     */
    printf("\n[4] Creating Unix socket...\n");

    sock = socket(AF_UNIX,
                  SOCK_STREAM,
                  0);

    if (sock < 0)
    {
        perror("socket");
    }
    else
    {
        printf("Socket created successfully.\n");
        close(sock);
    }

    sleep(3);

    /*
     * Repeated activity.
     */
    printf("\n[5] Entering monitoring loop...\n");

    for (int i = 0; i < 20; i++)
    {
        printf("Activity iteration %d - PID %d\n",
               i + 1,
               getpid());

        sleep(2);
    }

    /*
     * Cleanup.
     */
    printf("\nCleaning up...\n");

    unlink("/tmp/tracer_test_file.txt");

    printf("Test program finished.\n");

    return 0;
}


