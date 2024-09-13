#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


#define FD_INVALID -1
#define PIPE_IN 0
#define PIPE_OUT 1


__attribute__ ((noreturn)) void sieve(int fdin)
{
    int fdout = FD_INVALID;

    for (;;)
    {
        int p, n;

        if (read(fdin, &p, sizeof(p)) <= 0)
        {
            close(fdin);
            if (fdout != FD_INVALID)
            {
                close(fdout);
                wait(0);
            }
            exit(0);
        }
        printf("prime %d\n", p);

        while (read(fdin, &n, sizeof(n)) > 0)
        {
            if (n % p != 0)
            {
                if (fdout == FD_INVALID)
                {
                    int spipe[2];
                    if (pipe(spipe) < 0)
                    {
                        fprintf(2, "sieve: failed to create pipe\n");
                        close(fdin);
                        exit(1);
                    }

                    int pid = fork();
                    if (pid < 0)
                    {
                        fprintf(2, "sieve: failed to fork\n");
                        close(spipe[PIPE_IN]);
                        close(spipe[PIPE_OUT]);
                        close(fdin);
                        exit(1);
                    }

                    if (pid == 0)
                    {
                        close(spipe[PIPE_OUT]);
                        close(fdin);
                        fdin = spipe[PIPE_IN];
                        break; // to forever loop
                    }

                    close(spipe[PIPE_IN]);
                    fdout = spipe[PIPE_OUT];
                }

                write(fdout, &n, sizeof(n));
            }
        }
    }
}

int main(int argc, char* argv[])
{
    int parent_pipe[2];
    if (pipe(parent_pipe) < 0)
    {
        fprintf(2, "failed to create pipe\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0)
    {
        fprintf(2, "failed to fork\n");
        exit(1);
    }

    if (pid == 0)
    {
        // Chile process
        close(parent_pipe[PIPE_OUT]);
        sieve(parent_pipe[PIPE_IN]); // Does not return
    }

    // Parent process
    close(parent_pipe[PIPE_IN]);
    for (int i = 2; i <= 280; i++)
    {
        write(parent_pipe[PIPE_OUT], &i, sizeof(i));
    }
    close(parent_pipe[PIPE_OUT]);
    wait(0);

    exit(0);
}
