#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>
typedef struct{
    int job_id;
    pid_t pid;
    char* cmd;
    int status;
}job;

// void init_jobs();

void add_job(pid_t pid,const char* cmd, int state);
void check_complete();
void remove_job(pid_t pid);
job* get_job_pid(pid_t pid);
job* get_job_id(int job_id);
int get_next_job_id();
job* get_all_jobs(int* count);
job* get_last_job();

#endif