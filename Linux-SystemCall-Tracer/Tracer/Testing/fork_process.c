#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

void do_file_work(const char *filename, const char *message)
{
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if(fd >= 0)
    {
        write(fd, message, strlen(message));
        close(fd);
    }

    fd = open(filename, O_RDONLY);

    if(fd >= 0)
    {
        char buffer[128];

        read(fd, buffer, sizeof(buffer));

        close(fd);
    }
}

int main()
{
    printf("\n=====================================\n");
    printf(" Process Tree Test Program\n");
    printf("=====================================\n");

    printf("Parent PID : %d\n", getpid());

    /*
        Gives enough time to attach your tracer.
    */

    sleep(30);

    pid_t child1 = fork();

    if(child1 == 0)
    {
        printf("\nChild 1 PID : %d\n", getpid());

        mkdir("child1_dir",0755);

        do_file_work("child1.txt",
                     "Hello from Child 1\n");

        sleep(2);

        /*
            Create Grandchild
        */

        pid_t grandchild = fork();

        if(grandchild == 0)
        {
            printf("\nGrandchild PID : %d\n",
                    getpid());

            do_file_work("grandchild.txt",
                         "Hello from Grandchild\n");

            execl("/bin/echo",
                  "echo",
                  "Grandchild executed execve()",
                  NULL);

            exit(0);
        }

        wait(NULL);

        unlink("child1.txt");

        exit(0);
    }

    /*
        Parent creates second child
    */

    pid_t child2 = fork();

    if(child2 == 0)
    {
        printf("\nChild 2 PID : %d\n",
                getpid());

        mkdir("child2_dir",0755);

        do_file_work("child2.txt",
                     "Hello from Child 2\n");

        sleep(3);

        unlink("child2.txt");

        exit(0);
    }

    /*
        Parent work
    */

    do_file_work("parent.txt",
                 "Hello from Parent\n");

    wait(NULL);
    wait(NULL);

    printf("\nParent Finished\n");

    return 0;
}
