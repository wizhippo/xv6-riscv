#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

typedef int payload_t;

const payload_t payload = 42;

int
main(int argc, char* argv[])
{
    int pipe_fd[2];

    if (pipe(pipe_fd) < 0)
    {
        printf("failed to create pipe\n");
        exit(1);
    }

    int child_pid = fork();
    if (child_pid < 0)
    {
        printf("failed to fork\n");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        exit(1);
    }

    if (child_pid == 0)
    {
        close(pipe_fd[1]);

        payload_t i;
        read(pipe_fd[0], &i, sizeof(payload_t));

        if (i != payload)
        {
            printf("error: pipe returned %d expected %d\n", i, payload);
        }
        else
        {
            printf("%d: received ping\n", getpid());
        }

        close(pipe_fd[0]);
    }
    else
    {
        close(pipe_fd[0]);

        write(pipe_fd[1], &payload, sizeof(payload));
        wait(0);

        printf("%d: received pong\n", getpid());

        close(pipe_fd[1]);
    }

    exit(0);
}
