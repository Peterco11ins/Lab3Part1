// Two–way communication between parent and child using pipes
// Requirements:
// 1) P1 -> P2: send first string
// 2) P2: append "howard.edu", print Output
//        then prompt for second string, append it
//        send back to P1
// 3) P1: append "gobison.org" and print final Output

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd1[2];   // pipe: P1 writes, P2 reads
    int fd2[2];   // pipe: P2 writes, P1 reads

    char fixed_str_child[]  = "howard.edu";
    char fixed_str_parent[] = "gobison.org";

    char first_input[100];

    pid_t p;

    if (pipe(fd1) == -1) {
        fprintf(stderr, "Pipe Failed\n");
        return 1;
    }
    if (pipe(fd2) == -1) {
        fprintf(stderr, "Pipe Failed\n");
        return 1;
    }

    /* First input comes from the parent */
    printf("Other string is: %s\n", fixed_str_child);
    printf("Input : ");
    scanf("%99s", first_input);

    p = fork();

    if (p < 0) {
        fprintf(stderr, "fork Failed\n");
        return 1;
    }

    /* Parent process: P1 */
    if (p > 0) {
        char from_child[300];

        // P1 uses fd1[1] to send, fd2[0] to receive
        close(fd1[0]);   // not reading from fd1
        close(fd2[1]);   // not writing to fd2

        /* Send first input to child */
        write(fd1[1], first_input, strlen(first_input) + 1);
        close(fd1[1]);   // done writing

        /* Wait for the string back from child:
           first_input + "howard.edu" + second_input */
        read(fd2[0], from_child, sizeof(from_child));
        close(fd2[0]);

        /* Append gobison.org on the parent side */
        strcat(from_child, fixed_str_parent);

        printf("Output : %s\n", from_child);

        wait(NULL);   // wait for child to finish
    }
    /* Child process: P2 */
    else {
        // P2 uses fd1[0] to read, fd2[1] to write
        close(fd1[1]);   // not writing to fd1
        close(fd2[0]);   // not reading from fd2

        char original[100];
        char concat_str[300];
        char second_input[100];

        /* Read the first input from the parent */
        read(fd1[0], original, sizeof(original));
        close(fd1[0]);

        /* Build first concatenated string: first_input + "howard.edu" */
        strcpy(concat_str, original);
        strcat(concat_str, fixed_str_child);

        /* Print first result */
        printf("Output : %s\n\n", concat_str);

        /* Get the second input from the user (in child) */
        printf("Input : ");
        scanf("%99s", second_input);

        /* Append second input to the concatenated string */
        strcat(concat_str, second_input);

        /* Send back to parent: first_input + "howard.edu" + second_input */
        write(fd2[1], concat_str, strlen(concat_str) + 1);
        close(fd2[1]);

        exit(0);
    }

    return 0;
}