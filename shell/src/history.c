#include "history.h"
#include "commands.h"
#include "parser.h"
#include "executor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>



static char* history[15];
static int count=0;

void history_filepath(char* path,int size){
    struct passwd *pw=getpwuid(getuid());
    if(pw==NULL){
        snprintf(path,size,".shell_history");
        return;
    }
    const char* home=pw->pw_dir;
    snprintf(path,size,"%s/%s",home,".shell_history");
}

void load_history(){
    char path[1024];
    history_filepath(path, sizeof(path));
    FILE* fp=fopen(path,"r");
    if(fp==NULL){
        return;
    }
    char* line;
    size_t len=0;
    while(getline(&line,&len,fp)!=-1){
        if(count<15){
            line[strcspn(line,"\n")]=0;
            history[count]=strdup(line);
            count++;
        }
    }
    free(line);
    fclose(fp);

}

void save_history(){
    char path[1024];
    history_filepath(path, sizeof(path));
    FILE* fp=fopen(path,"w");
    if(fp==NULL){
        return;
    }
    for(int i=0;i<count;i++){
        fprintf(fp,"%s\n",history[i]);
    }
    fclose(fp);
}

void add_to_history(const char* command){
    if(strncmp(command,"log",3)==0){
        return;
    }
    if(count>0 && strcmp(history[count-1],command)==0){
        return;
    }
    if(count<15){
        history[count]=strdup(command);
        count++;
    }
    else{
        free(history[0]);
        for(int i=0;i<14;i++){
            history[i]=history[i+1];
        }
        history[14]=strdup(command);
        count = 15;
    }
}

char* get_history_command(int index) {
    int real_index = count - index;
    if (index > 0 && real_index >= 0 && real_index < count) {
        return history[real_index];
    }
    return NULL;
}
//LLM Generated Code Begins
void execute_log(int argc,char* argv[]){
    if(argc>1 && strcmp(argv[1],"purge")==0){
        for(int i=0;i<count;i++){
            free(history[i]);
        }
        count=0;
        return;
    }
    else if (argc > 2 && strcmp(argv[1], "execute") == 0) {
        int idx = atoi(argv[2]);
        char *cmd = get_history_command(idx);
        if(cmd){
            if (isatty(STDOUT_FILENO)) {
                // Standalone: match interactive behavior
                printf("%s\n", cmd);
            } else {
                printf("%s\n", cmd);
            }
            fflush(stdout);
        }
        return;
    }
    else if (argc == 1) {
        for (int i = 0; i < count; i++) {
            printf("%s\n", history[i]);
        }
    }
    else {
        printf("log: invalid arguments\r\n");
    }
}
//LLM Generated Code Ends