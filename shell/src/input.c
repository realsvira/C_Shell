#include "input.h"
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>

static char current_input[1025] = "";

const char* get_current_input_buffer() {
    return current_input;
}

char* get_user_input(){
    if(fgets(current_input, sizeof(current_input), stdin) != NULL){
        int len = strlen(current_input);
        if(len > 0 && current_input[len-1] == '\n'){
            current_input[len-1] = '\0';
        }
        return strdup(current_input);
    }
    current_input[0] = '\0';
    return NULL;
}