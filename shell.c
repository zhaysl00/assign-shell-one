
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define BSIZE 1024

// Handle SIGINT (Ctrl+C)
void handleSignal(int signal) {
    if (signal == SIGINT) {
        write(STDOUT_FILENO, "\n", 1);
    }
}

int main(int argc, char *argv[]) {
    char buffer[BSIZE];
    pid_t child;
    int status;
    char *args[20];

    // Register SIGINT handler
    signal(SIGINT, handleSignal);

    while (1) {
        printf("shell> ");
        fflush(stdout);

        if (fgets(buffer, BSIZE, stdin) == NULL) {
            if (errno == EINTR) {
                clearerr(stdin);
                continue;
            }
            break;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        // Parse command and arguments
        int i = 0;
        char *token = strtok(buffer, " ");

        while (token != NULL && i < 19) {
            args[i] = token;
            i++;
            token = strtok(NULL, " ");
        }

        args[i] = NULL;

        // Handle empty input
        if (args[0] == NULL) {
            continue;
        }

        // Internal exit command
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        // Internal cd command
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                if (chdir(getenv("HOME")) != 0) {
                    perror("cd");
                }
            } else {
                if (chdir(args[1]) != 0) {
                    perror("cd");
                }
            }
            continue;
        }

        // External command
        child = fork();

        if (child == 0) {
            // Child Process
            signal(SIGINT, SIG_DFL);

            execvp(args[0], args);

            perror("execvp");
            exit(1);

        } else if (child > 0) {
            // Parent Process
            while (waitpid(child, &status, 0) == -1) {
                if (errno == EINTR) {
                    continue;
                }

                perror("waitpid");
                break;
            }

        } else {
            perror("fork");
        }
    }

    return 0;
}
