// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

// Defines
#define WASABI_RL_BUFSIZE 1024
#define WASABI_PATH_SIZE 1024
#define WASABI_FOLDER_NAME_LIMIT 1024
#define WASABI_TOK_DELIM " \t\r\n\a"

// Function declarations
int wasabi_cd(char **args);
int wasabi_ls(char **args);
int wasabi_cwd(char **args);
int wasabi_mkdir(char **args);
int wasabi_rmdir(char **args);
int wasabi_help(char **args);
int wasabi_exit(char **args);

// Built-in commands and their corresponding functions
char *builtin_str[] = {
    "cd",
    "ls",
    "cwd",
    "mkdir",
    "rmdir",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &wasabi_cd,
    &wasabi_ls,
    &wasabi_cwd,
    &wasabi_mkdir,
    &wasabi_rmdir,
    &wasabi_help,
    &wasabi_exit
};

int wasabi_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// Built-in function implementations
int wasabi_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "wasabi: expected argument to \"cd\"\n");
    }
    else {
        if (chdir(args[1]) != 0) {
            perror("wasabi");
        }
    }
    return 1;
}

int wasabi_ls(char **args) {
    struct dirent **filelist;
    int n;
    n = scandir(".", &filelist, NULL);
    
    if (n == -1) {
        perror("scandir");
    }

    while (n--) {
        if (filelist[n]->d_name[0] != '.') {
            printf("%s\n", filelist[n]->d_name);
            free(filelist[n]);
        } 
    }

    free(filelist);

    return 1;
}

int wasabi_cwd(char **args) {
    int bufsize = WASABI_PATH_SIZE * sizeof(char);
    char *buf = malloc(bufsize);
    
    // Implement functionality: resize buf if exceeds memory.
    if (getcwd(buf, bufsize) == NULL) {
        perror("wasabi: something went wrong.");
    }
    else {
        printf("%s\n", buf);
    }

    free(buf);

    return 1;
}

int wasabi_mkdir(char **args) {
    
    int bufsize = WASABI_PATH_SIZE * sizeof(char);
    char *cwd = malloc(bufsize);
    getcwd(cwd, bufsize);

    char app = '/';

    strcat(cwd, &app);
    strcat(cwd, args[1]);

    if (mkdir(cwd, 77777) == -1) {
        if (errno == EEXIST) {
            printf("Error: Folder already exists.\n");
        }
    }

    free(cwd);
    
    return 1;
}

int wasabi_rmdir(char **args) {
    
    int bufsize = WASABI_PATH_SIZE * sizeof(char);
    char *cwd = malloc(bufsize);
    getcwd(cwd, bufsize);

    char app = '/';

    strcat(cwd, &app);
    strcat(cwd, args[1]);

    if (rmdir(cwd) == -1) {
        if (errno == ENOTEMPTY) {
            printf("Error: Folder not empty.\n");
        }
        else if (errno == ENOTDIR) {
            printf("Provided name is not a folder.\n");
        }
        else if (errno == ENOENT) {
            printf("Folder doesn't exist.\n");
        }
    }
    
    free(cwd);
    
    return 1;
}

int wasabi_help(char **args) {
    int i;
    printf("Imtiaz Ahmed's Wasabi.\n");
    printf("A small shell writen in C.\n");
    printf("The following are built-in:\n");
    
    for (i = 0; i < wasabi_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int wasabi_exit(char **args) {
    return 0;
}

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

// Execute 
int wasabi_execute(char **args) {
    int i;
    
    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < wasabi_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return wasabi_launch(args);
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
        args = wasabi_split_line(line);
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