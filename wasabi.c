// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

// Defines
#define WASABI_RL_BUFSIZE 1024
#define WASABI_TOK_DELIM " \t\r\n\a"

/*** Check this to learn more about execvp 
     and fork: http://www.cs.ecu.edu/karl/4630/sum01/example1.html ***/
// Launch shell process
int wasabi_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("wasabi");
        }
        exit(EXIT_FAILURE);        
    }
    else if (pid < 0) {
        // Error forking
        perror("wasabi");
    }
    else {
        // Parent Process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

char **wasabi_split_line(char *line) {
    int bufsize = WASABI_RL_BUFSIZE; 
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char));
    char *token;

    if (!tokens) {
        fprintf(stderr, "wasabi: allocation error.\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, WASABI_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += WASABI_RL_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char));
            if (!tokens) {
                fprintf(stderr, "wasabi: allocation error.\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, WASABI_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

char *wasabi_read_line(void) {
    int bufsize = WASABI_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(bufsize * sizeof(char));
    int c;
    if (!buffer) {
        fprintf(stderr, "wasabi: allocation error.\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        /*** If we are hit with a EOF character, we replace it with
        a NULL character and return it ***/ 
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        else {
            buffer[position] = c;
        }
        position++;

        /*** If we have exceeded buffer length, we reallocate
             memory with a realloc() function ***/
        if (position > bufsize) {
            bufsize += WASABI_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "wasabi: allocation error.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void wasabi_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = wasabi_read_line();
        args = wasabi_split_line();
        status = wasabi_execute(args);

        free(line);
        free(args);
    } while(status);
}

int main(int argc, char **argv) {
    // Load config files [if any]

    // Run command loop
    wasabi_loop();

    // Perform any shutdown/cleanup

    return EXIT_SUCCESS;;
}