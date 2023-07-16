#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void diagnostics(char* program_output, int n, int m)
{
    char c;

    printf("Diagnostics:\n");

    for (int i = m; i > 0; i--)
    {
        c = program_output[n - i];

        if (isprint(c))
        {
            printf("program_output[n-%d] = %02hhx %c\n", i, c, c);
        }
            
        else
        {
            printf("program_output[n-%d] = %02hhx %s\n", i, c, "n/a");
        }
    }
}

int main(int argc, char** argv)
{
    int fds[2];

    pipe(fds);

    if (fork() == 0)
    {
        dup2(fds[1], fileno(stdout));
        close(fds[0]);
        execv("bin/ls", argv);
        return 1;
    }

    close(fds[1]);
    sleep(1);

    char program_output[1024];
    int n;

    while (1)
    {
        n = read(fds[0], program_output, 1024);

        printf("%s", program_output);
        // fwrite(program_output, 1, n, stdout);

        if (program_output[n-1] == EOF)
        {
            break;
        }
    }

    diagnostics(program_output, n, 9);

    return 0;
}