#include "prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <string.h>

char* get_prompt(char* shell_home){
    char system_name[1025];
    struct passwd *pw = getpwuid(getuid());
    char* username;

    char cwd[1025];
    if(pw!=NULL){
        username = pw->pw_name;
    }
    else{
        username = "user";    // Default username
    }
    if(gethostname(system_name,1025)!=0){
        strcpy(system_name,"unknown");    // Default system_name
    }
    if(getcwd(cwd,1025)==NULL){
        strcpy(cwd,"unknown");    // Default cwd
    }

    //LLM Generated Code Begins
    if (shell_home != NULL && strncmp(cwd, shell_home, strlen(shell_home)) == 0) {
    //LLM Generated Code Ends
        char temp[1025];
        strcpy(temp, "~");
        strcat(temp, cwd + strlen(shell_home));
        strcpy(cwd, temp);
    }

    int prompt_len=strlen(username)+strlen(system_name)+strlen(cwd)+6;
    char* temp = (char*)malloc(prompt_len*sizeof(char));
    if(temp==NULL){
        perror("malloc");
        exit(1);
    }
    strcpy(temp,"<");
    strcat(temp,username);
    strcat(temp,"@");
    strcat(temp,system_name);
    strcat(temp,":");
    strcat(temp,cwd);
    strcat(temp,">");
    return temp;
}