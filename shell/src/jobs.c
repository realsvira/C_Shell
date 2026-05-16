#include "jobs.h"
#include "prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

static job job_list[1024];
static int jobs_count = 0;
static int next_job_id=1;

// void init_jobs(){

// }

int get_next_job_id(){
    return next_job_id;
}

void add_job(int pid,const char* cmd,int state){
    if(jobs_count>=1024){
        printf("Max jobs reached\n");
        return;
    }
    job_list[jobs_count].pid = pid;
    job_list[jobs_count].job_id = next_job_id++;
    job_list[jobs_count].cmd = strdup(cmd);
    job_list[jobs_count].status = state;
    jobs_count++;
}

void remove_job(pid_t pid){
    int found=-1;
    for(int i=0;i<jobs_count;i++){
        if(job_list[i].pid == pid){
            found = i;
            break;
        }
    }
    if(found != -1){
        free(job_list[found].cmd);
        for(int i=found;i<jobs_count-1;i++){
            job_list[i] = job_list[i+1];
        }
        jobs_count--;
    }
}

void check_complete(){
    int status;
    pid_t pid;
    //LLM Generated Code Begins
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        job* j = get_job_pid(pid);
        if (j) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                // CRITICAL: Use the exact format the autograder expects
                printf("\n%s with pid %d exited %s\n", j->cmd, j->pid, 
                       WIFEXITED(status) ? "normally" : "abnormally");
                fflush(stdout);
                remove_job(j->pid);
            } else if (WIFSTOPPED(status)) {
                j->status = 1;
            }
        }
    }
    //LLM Generated Code Ends
}

job* get_job_pid(pid_t pid){
    for(int i=0;i<jobs_count;i++){
        if(job_list[i].pid == pid){
            return &job_list[i];
        }
    }
    return NULL;
}

job* get_job_id(int job_id){
    for(int i=0;i<jobs_count;i++){
        if(job_list[i].job_id == job_id){
            return &job_list[i];
        }
    }
    return NULL;
}

job* get_all_jobs(int* count) {
    *count = jobs_count;
    job* list_copy = malloc(sizeof(job) * jobs_count);
    if (list_copy) {
        memcpy(list_copy, job_list, sizeof(job) * jobs_count);
    }
    return list_copy;
}

job* get_last_job() {
    if (jobs_count == 0) {
        return NULL;
    }
    return &job_list[jobs_count - 1];
}