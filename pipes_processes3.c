#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <search-string>\n", argv[0]);
        return 1;
    }

    int pipe1[2];   // cat -> grep
    int pipe2[2];   // grep -> sort

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        return 1;
    }

    // --- FIRST CHILD: grep <search> ---
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        return 1;
    }

    if (pid1 == 0) {
        // stdin from pipe1 read end
        if (dup2(pipe1[0], 0) == -1) {
            perror("dup2 grep stdin");
            _exit(1);
        }

        // stdout to pipe2 write end
        if (dup2(pipe2[1], 1) == -1) {
            perror("dup2 grep stdout");
            _exit(1);
        }

        // close all pipe fds (no longer needed after dup2)
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        execlp("grep", "grep", argv[1], NULL);
        perror("execlp grep");
        _exit(1);
    }

    // --- SECOND CHILD: sort ---
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        return 1;
    }

    if (pid2 == 0) {
        // stdin from pipe2 read end
        if (dup2(pipe2[0], 0) == -1) {
            perror("dup2 sort stdin");
            _exit(1);
        }

        // close all pipe fds (no longer needed after dup2)
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        execlp("sort", "sort", NULL);
        perror("execlp sort");
        _exit(1);
    }

    // --- PARENT: cat scores ---
    // stdout to pipe1 write end
    if (dup2(pipe1[1], 1) == -1) {
        perror("dup2 cat stdout");
        return 1;
    }

    // close all pipe fds in parent (only stdout=1 is needed now)
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    execlp("cat", "cat", "scores", NULL);
    perror("execlp cat");
    return 1;
}