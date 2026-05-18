#include "prompt.h"
#include "input.h"
#include "parser.h"
#include "commands.h"
#include "history.h"
#include "executor.h"
#include "jobs.h"
#include "signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
    char shell_home[1025];
    char prev_cwd[1025] = ""; 
    if(getcwd(shell_home, sizeof(shell_home)) == NULL){
        perror("getcwd");
        exit(1);
    }

    load_history();
    // init_jobs();
    
    pid_t shell_pgid = getpid();
    if(setpgid(shell_pgid, shell_pgid) < 0){
        perror("setpgid");
    }
    tcsetpgrp(STDIN_FILENO, shell_pgid);
    set_signal_handlers();
    
    while(1){
        // Check for completed jobs before showing prompt
        check_complete();
        
        // Check the signal flag and reset it
        if (child_exited) {
            child_exited = 0;
            check_complete();
        }
        
        char* prompt = get_prompt(shell_home);
        printf("%s ", prompt);
        fflush(stdout);
        free(prompt);

        char* input = get_user_input();
        check_complete();
        // Check again after input in case signal arrived during input
        if (child_exited) {
            child_exited = 0;
            check_complete();
            // If input was interrupted by signal, continue to next iteration
            if (input == NULL && !feof(stdin)) {
                continue;
            }
        }
        
        if(input == NULL){ 
            if (feof(stdin)) {
                printf("logout\n");
                break; 
            } else {
                continue;
            }
        }

        if (strncmp(input, "log execute ", 12) == 0) {
            int index = atoi(input + 12);
            char* history_cmd = get_history_command(index);
            if (history_cmd) {
                free(input);
                input = strdup(history_cmd);
                printf("%s\n", input);
            } else {
                printf("log: invalid history index\n");
                free(input);
                continue;
            }
        }

        if(strlen(input) > 0){
            add_to_history(input);
        } else {
            free(input);
            continue;
        }

        parsed* parsed_cmd = parse_input(input);

        if (parsed_cmd == NULL) {
            printf("Invalid Syntax!\n");
            free(input);
            continue;
        } 
        
        if (parsed_cmd->num_commands > 0) {
            check_complete();
            int cmd_idx = 0;
            //LLM Generated Code Begins
            while (cmd_idx < parsed_cmd->num_commands) {
                int pipeline_start_idx = cmd_idx;
                if(parsed_cmd->commands[cmd_idx] == NULL){
                    cmd_idx++;
                    continue;
                }
                while (cmd_idx < parsed_cmd->num_commands - 1 && strcmp(parsed_cmd->separators[cmd_idx], "|") == 0) {
                    cmd_idx++;
                }
                
                int num_in_pipeline = cmd_idx - pipeline_start_idx + 1;
                operators** pipeline_cmds = &parsed_cmd->commands[pipeline_start_idx];
                
                int is_bg = 0;
                if (cmd_idx < parsed_cmd->num_commands - 1 && strcmp(parsed_cmd->separators[cmd_idx], "&") == 0) {
                    is_bg = 1;
                } else if (cmd_idx == parsed_cmd->num_commands - 1 && parsed_cmd->background) {
                    is_bg = 1;
                }
                //LLM Generated Code Ends
                // Handle special commands that MUST affect the main shell process
                if (num_in_pipeline == 1 && !is_bg) {
                    operators* cmd = pipeline_cmds[0];
                    if (strcmp(cmd->argv[0], "exit") == 0) {
                        free(input);
                        free_parsed_input(parsed_cmd);
                        save_history();
                        return 0;
                    }
                    if (strcmp(cmd->argv[0], "hop") == 0) {
                        execute_hop(cmd->argc, cmd->argv, shell_home, prev_cwd);
                        cmd_idx++;
                        continue;
                    }
                    if (strcmp(cmd->argv[0], "fg") == 0) {
                        execute_fg(cmd->argc, cmd->argv);
                        cmd_idx++;
                        continue;
                    }
                    if (strcmp(cmd->argv[0], "bg") == 0) {
                        execute_bg(cmd->argc, cmd->argv);
                        cmd_idx++;
                        continue;
                    }
                }
                
                // For simple built-ins with no redirection, run directly
                //LLM Generated Code Begins
                if (num_in_pipeline == 1 && !is_bg && 
                    !pipeline_cmds[0]->input_file && !pipeline_cmds[0]->output_file) {
                    operators* cmd = pipeline_cmds[0];
                    
                //LLM Generated Code Ends
                    if (strcmp(cmd->argv[0], "reveal") == 0) {
                        execute_reveal(cmd->argc, cmd->argv, shell_home, prev_cwd);
                        cmd_idx++;
                        continue;
                    }
                    if (strcmp(cmd->argv[0], "log") == 0) {
                        execute_log(cmd->argc, cmd->argv);
                        cmd_idx++;
                        continue;
                    }
                    if (strcmp(cmd->argv[0], "activities") == 0) {
                        execute_activities();
                        cmd_idx++;
                        continue;
                    }
                    if (strcmp(cmd->argv[0], "ping") == 0) {
                        execute_ping(cmd->argc, cmd->argv);
                        cmd_idx++;
                        continue;
                    }
                }

                // All other cases: pipelines, background jobs, or built-ins with redirection
                pid_t pgid = execute_pipe(pipeline_cmds, num_in_pipeline, is_bg, shell_home, prev_cwd);
                
                if (pgid > 0) {
                    if (is_bg) {
                        add_job(pgid, input, 0);
                        // Immediately print background job number and PID as required
                        job* j = get_job_pid(pgid);
                        if (j) {
                            printf("[%d] %d\r\n", j->job_id, j->pid);
                            fflush(stdout);
                        }
                    }  else {
                        foregrnd_pid = pgid;
                        tcsetpgrp(STDIN_FILENO, pgid);
                        
                        // CORRECTED WAITING LOGIC
                        //LLM Generated Code Begins
                        int remaining_procs = num_in_pipeline;
                        int stopped_flag=0;
                        while (remaining_procs > 0) {
                            int status;
                            pid_t child_pid = waitpid(-pgid, &status, WUNTRACED);
                            if (child_pid < 0) {
                                // Error or no more children in group
                                break;
                            }

                            if (WIFSTOPPED(status)) {
                                add_job(pgid, pipeline_cmds[0]->argv[0], 1);
                                stopped_flag = 1;
                                break;
                            }
                            if (WIFEXITED(status) || WIFSIGNALED(status))
                                remaining_procs--;
                        }
                        //LLM Generated Code Ends
                        
                        tcsetpgrp(STDIN_FILENO, shell_pgid);
                        foregrnd_pid = 0;
                        if (stopped_flag) {
                            job* new_j = get_job_pid(pgid);
                            if (new_j) {
                                printf("\n[%d] Stopped %s\n", new_j->job_id, new_j->cmd);
                                fflush(stdout);
                            }
                            // char* new_prompt = get_prompt(shell_home);
                            // printf("%s ", new_prompt);
                            // fflush(stdout);
                            // free(new_prompt);
                        }
                    }
                }
                
                cmd_idx++;
            }
        }
        free_parsed_input(parsed_cmd);
        free(input);
    }

    save_history();
    return 0;
}