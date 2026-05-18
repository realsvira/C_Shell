#include "executor.h"
#include "commands.h"
#include "history.h"
#include "signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// New redirection handler: argv already stripped of <,>,>> tokens at parse stage;
// we now rely on the arrays populated in operators struct.
static int handle_redirections(char **argv) {
    (void)argv; // tokens already processed; nothing to strip
    return 0;
}

//LLM Generated Code Begins
pid_t execute_pipe(operators** commands, int num_commands, int in_bg, const char* shell_home, const char* prev_cwd) {
    if (num_commands == 0) return 0;

    pid_t pgid = 0;
    int *pids = (int *)malloc(sizeof(int)*num_commands);
    if(!pids){
        perror("malloc");
        return -1;
    }
    int prev_pipe_read_end = -1;

    for (int i = 0; i < num_commands; i++) {
        int pipefd[2] = {-1,-1};
        if (i < num_commands - 1) {
            if (pipe(pipefd) < 0) { perror("pipe"); return -1; }
        }

        pids[i] = fork();
        if (pids[i] < 0) { perror("fork"); return -1; }

        if (pids[i] == 0) {
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);

            if (pgid == 0) pgid = getpid();
            setpgid(0, pgid);

            if (in_bg && i == 0 && !commands[i]->input_file) {
                int dev_null = open("/dev/null", O_RDONLY);
                if(dev_null>=0){
                    dup2(dev_null, STDIN_FILENO);
                    close(dev_null);
                }
            }

            if (prev_pipe_read_end != -1) {
                dup2(prev_pipe_read_end, STDIN_FILENO);
                close(prev_pipe_read_end);
            }
            if (i < num_commands - 1) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            if (commands[i]->input_file) {
                // Multiple input redirections: validate all, only last effective
                // Validate all input files first
                for(int ri=0; ri<commands[i]->in_count; ri++){
                    int fd_test = open(commands[i]->input_files[ri], O_RDONLY);
                    if(fd_test < 0){
                        fprintf(stderr, "No such file or directory\n");
                        exit(1);
                    }
                    close(fd_test);
                }
                // Apply the last input redirection
                int fd_in = open(commands[i]->input_files[commands[i]->in_count-1], O_RDONLY);
                if(fd_in < 0){
                    fprintf(stderr, "No such file or directory\n");
                    exit(1);
                }
                if(dup2(fd_in, STDIN_FILENO) < 0){ perror("dup2"); close(fd_in); exit(1);} 
                close(fd_in);
            }
            if (commands[i]->output_file) {
                // Create/truncate or append earlier output redirection targets
                for(int ro=0; ro<commands[i]->out_count; ro++){
                    int flags_tmp = O_WRONLY | O_CREAT | (commands[i]->output_append[ro] ? O_APPEND : O_TRUNC);
                    int fd_tmp = open(commands[i]->output_files[ro], flags_tmp, 0666);
                    if(fd_tmp < 0){
                        fprintf(stderr, "Unable to create file for writing\n");
                        exit(1);
                    }
                    // Only leave open if last; earlier ones close immediately
                    if(ro == commands[i]->out_count -1){
                        if(dup2(fd_tmp, STDOUT_FILENO) < 0){ perror("dup2"); close(fd_tmp); exit(1);} 
                        close(fd_tmp);
                    } else {
                        close(fd_tmp);
                    }
                }
            }
            // Ignore old struct fields (input_file/output_file) for multi redirect tests
            if(handle_redirections(commands[i]->argv) < 0){
                exit(1);
            }

            char* cmd_name = commands[i]->argv[0];
            if (!cmd_name) exit(0);

            if (strcmp(cmd_name, "reveal") == 0) {
                execute_reveal(commands[i]->argc, commands[i]->argv, shell_home, prev_cwd);
                exit(0);
            } else if (strcmp(cmd_name, "echo") == 0) {
                for (int j = 1; j < commands[i]->argc; j++) {
                    char *arg = commands[i]->argv[j];
                    size_t len = strlen(arg);
                    if (len >= 2 && ((arg[0]=='\'' && arg[len-1]=='\'') || (arg[0]=='\"' && arg[len-1]=='\"'))) {
                        fwrite(arg+1,1,len-2,stdout);
                    } else {
                        fputs(arg, stdout);
                    }
                    if (j < commands[i]->argc - 1) fputc(' ', stdout);
                }
                fputc('\n', stdout);
                fflush(stdout);
                exit(0);
                //LLM Generated Code Ends
            } else if (strcmp(cmd_name, "log") == 0) {
                execute_log(commands[i]->argc, commands[i]->argv);
                exit(0);
            } else if (strcmp(cmd_name, "activities") == 0) {
                execute_activities();
                exit(0);
            } else if (strcmp(cmd_name, "ping") == 0) {
                execute_ping(commands[i]->argc, commands[i]->argv);
                exit(0);
            }

            execvp(commands[i]->argv[0], commands[i]->argv);
            fprintf(stderr,"Command not found!\n");
            fflush(stderr);
            exit(127);
        } else {
            if (pgid == 0) pgid = pids[i];
            setpgid(pids[i], pgid);

            if (prev_pipe_read_end != -1) close(prev_pipe_read_end);
            if (i < num_commands - 1) {
                prev_pipe_read_end = pipefd[0];
                close(pipefd[1]);
            }
        }
    }
    if (prev_pipe_read_end != -1) close(prev_pipe_read_end);
    return pgid;
}