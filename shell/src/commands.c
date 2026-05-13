#include "commands.h"
#include "jobs.h"
#include "signals.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h> // For strcasecmp
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>

static int cmp_ascii(const void *a,const void *b){
    const char* s1 = *(const char* const*)a;
    const char* s2 = *(const char* const*)b;
    return strcmp(s1,s2);
}

int cmp_jobs(const void *a, const void *b) {
    job *job_a = (job *)a;
    job *job_b = (job *)b;
    return strcmp(job_a->cmd, job_b->cmd);
}

void execute_hop(int argc, char* argv[], const char* shell_home, char* prev_cwd) {
    char current_cwd[1025];
    if (getcwd(current_cwd, sizeof(current_cwd)) == NULL) {
        printf("No such directory!\r\n");
        return;
    }

    if (argc == 1) {
        strcpy(prev_cwd, current_cwd);
        if (chdir(shell_home) != 0) {
            printf("No such directory!\r\n");
        }
        return;
    }

    for (int i = 1; i < argc; i++) {
        if (getcwd(current_cwd, sizeof(current_cwd)) == NULL) {
            printf("No such directory!\r\n");
            break;
        }

        char* target = argv[i];
        int chdir_result = -1;

        if (strcmp(target, "~") == 0) {
            strcpy(prev_cwd, current_cwd);
            chdir_result = chdir(shell_home);
        }
        else if (strcmp(target, ".") == 0) {
            chdir_result = 0;
        }
        else if (strcmp(target, "..") == 0) {
            strcpy(prev_cwd, current_cwd);
            chdir_result = chdir("..");
        }
        else if (strcmp(target, "-") == 0) {
            if (strlen(prev_cwd) == 0) {
                fprintf(stderr, "hop: OLD_PWD not set\r\n");
                chdir_result = 0;
            }
            else {
                char temp_cwd[1025];
                strcpy(temp_cwd, current_cwd);
                chdir_result = chdir(prev_cwd);
                if (chdir_result == 0) {
                    strcpy(prev_cwd, temp_cwd);
                }
            }
        }
        else {
            strcpy(prev_cwd, current_cwd);
            chdir_result = chdir(target);
        }

        if (chdir_result != 0) {
            printf("No such directory!\r\n");
            break;
        }
    }
}


void execute_reveal(int argc,char* argv[],const char* shell_home, const char* prev_cwd){
    int show_a = 0;
    int show_l = 0;
    char* path_arg = NULL;

    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            if(strcmp(argv[i],"-") == 0){
                path_arg = argv[i];
                continue;
            }
            for(int j = 1; argv[i][j] != '\0'; j++){
                if(argv[i][j] == 'a'){
                    show_a = 1;
                } else if (argv[i][j] == 'l') {
                    show_l = 1;
                }
            }
        }
        else{
            if(path_arg != NULL){
                printf("reveal: Invalid Syntax!\r\n");
                return;
            }
            path_arg=argv[i];
        }
    }
    char target_dir[1025];
    if(path_arg==NULL){
        strcpy(target_dir,".");
    }
    else if(strcmp(path_arg,"~")==0){
        strcpy(target_dir,shell_home);
    }
    else if(strcmp(path_arg,"-")==0){
        if(strlen(prev_cwd)==0){
            printf("No such directory!\r\n");
            return;
        }
        else{
            strcpy(target_dir,prev_cwd);
        }
    }
    else{
        strcpy(target_dir,path_arg);
    }
    //LLM Generated Code Begins
    DIR* direc=opendir(target_dir);
    if(direc==NULL){
        printf("No such directory!\r\n");
        return;
    }
    struct dirent* entry;
    char* entries[4097];
    int count=0;
    while((entry=readdir(direc))!=NULL && count < 4096){
        if(show_a==0 && entry->d_name[0]=='.'){
            continue;
        }
        entries[count++]=strdup(entry->d_name);
    }
    closedir(direc);

    qsort(entries, count, sizeof(char*), cmp_ascii);

    if (show_l) {
        for(int i=0;i<count;i++){
            printf("%s\r\n", entries[i]);
            free(entries[i]);
        }
    } else {
        for(int i=0;i<count;i++){
            printf("%s", entries[i]);
            if(i < count-1) printf(" ");
            free(entries[i]);
        }
        if(count>0) printf("\r\n");
    }
    //LLM Generated Code Ends
}


void execute_activities(){
    int count;
    job* jobs = get_all_jobs(&count);
    if(count==0){
        return;
    }
    qsort(jobs, count, sizeof(job), cmp_jobs);
    for(int i=0;i<count;i++){
        printf("[%d] : %s - %s\r\n", jobs[i].pid, jobs[i].cmd, jobs[i].status == 0 ? "Running" : "Stopped");
    }
    free(jobs);
}

void execute_ping(int argc, char* argv[]){
    if(argc!=3){
        printf("ping: Invalid Syntax!\r\n");
        return;
    }

    pid_t pid=atoi(argv[1]);
    int signal=atoi(argv[2]);

    if(kill(pid,0)==-1){
        printf("No such process found\r\n");
        return;
    }
    //LLM Generated Code Begins
    int temp=signal%32;
    if(kill(pid,temp)==0){
        printf("Sent signal %d to process with pid %d\r\n",temp,pid);
    }
    else{
        perror("ping: kill");
    }
    //LLM Generated Code Ends
}

void execute_fg(int argc, char* argv[]) {
    job* j=NULL;
    if(argc==1){
        j=get_last_job();
    }
    else if(argc==2){
        int job_id=atoi(argv[1]);
        j=get_job_id(job_id);
    }
    else{
        printf("fg: Invalid Syntax\r\n");
        return;
    }
    if(j==NULL){
        printf("No such job\r\n");
        return;
    }
    printf("%s\r\n", j->cmd);

    tcsetpgrp(STDIN_FILENO, j->pid);
    if(j->status==1){
        //LLM Generated Code Begins
        if(kill(-j->pid, SIGCONT)==-1){
            perror("fg: kill (SIGCONT)");
            tcsetpgrp(STDIN_FILENO, getpgrp());
            return;
        }
    }
    foregrnd_pid=j->pid;
    pid_t job_pid = j->pid;
    char* job_cmd = strdup(j->cmd);
    remove_job(j->pid);
    int status;
    if(waitpid(-job_pid, &status, WUNTRACED)<0){
        perror("waitpid");
    }

    tcsetpgrp(STDIN_FILENO, getpgrp());
    if(WIFSTOPPED(status)){
        add_job(job_pid, job_cmd, 1);
        job* new_j=get_job_pid(job_pid);
        if(new_j){
            printf("\n[%d] Stopped %s\n", new_j->job_id, new_j->cmd);
            printf("<user@system:~> ");
            fflush(stdout);
        }
    }
    free(job_cmd);
    foregrnd_pid=0;
    //LLM Generated Code Ends
}

void execute_bg(int argc,char* argv[]){
    if(argc!=2){
        printf("bg: Invalid Syntax\r\n");
        return;
    }
    int job_id=atoi(argv[1]);
    job* j=get_job_id(job_id);
    if(!j){
        printf("No such job\r\n");
       return;
    }
    if(j->status==0){
        printf("Job already running\r\n");
        return;
    }
    if(kill(-j->pid, SIGCONT)<0){
        perror("bg: kill (SIGCONT)");
        return;
    }
    j->status=0;
    printf("[%d] %s &\n", j->job_id, j->cmd);
}