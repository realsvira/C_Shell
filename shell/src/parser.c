#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


static int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c=='\n' || c=='\r';
}

static int is_operator(const char* token) {
    return strcmp(token, "|") == 0 || strcmp(token, "&") == 0 || 
           strcmp(token, "<") == 0 || strcmp(token, ">") == 0 || strcmp(token, ">>") == 0 || strcmp(token, ";") == 0;
}

static int tokenize(char *input, char tokens[][128]) {
    int count = 0;
    while (*input && count < 1024) {
        while (is_whitespace(*input)) {
            input++;
        }
        if (*input == '\0') {
            break;
        }

        if (strncmp(input, ">>", 2) == 0) {
            strcpy(tokens[count++], ">>");
            input += 2;
        } 
        // else if (strncmp(input, "&&", 2) == 0) {
        //     strcpy(tokens[count++], "&&");
        //     input += 2;
        // } 
        else if (strchr("|&<>;", *input)) {
            tokens[count][0] = *input;
            tokens[count][1] = '\0';
            count++;
            input++;
        } 
        else {
            int j = 0;
            while (*input && !is_whitespace(*input) && !strchr("|&<>;", *input)) {
                tokens[count][j++] = *input++;
            }
            tokens[count][j] = '\0';
            count++;
        }
    }
    return count;
}


// This function validates the token sequence before we try to parse it.
static int validate_tokens(int n, char tokens[][128]) {
    if (n == 0){
        return 1;
    }
    if(n>0 && is_operator(tokens[0])){
        return 0;
    }
    //Cannot start with |, &, &&
    if (strcmp(tokens[0], "|") == 0 || strcmp(tokens[0], "&") == 0 ||  strcmp(tokens[0], ";") == 0) {
        return 0;
    }

    //Cannot end with |, &&, <, >, >>
    const char* last_token = tokens[n - 1];
    if (strcmp(last_token, "|") == 0 || 
        strcmp(last_token, "<") == 0 || strcmp(last_token, ">") == 0 || strcmp(last_token, ">>") == 0 || strcmp(last_token, ";") == 0) {
        return 0;
    }

    for(int i=0;i<n-1;i++){
        if(is_operator(tokens[i]) && is_operator(tokens[i+1])){
            if(strcmp(tokens[i], ";")==0 && strcmp(tokens[i+1], "&")==0){
                continue;
            }
            return 0;
        }
    }
    return 1;
}



parsed* parse_input(char* input) {
    char tokens[1024][128];
    int n = tokenize(input, tokens);

    if (!validate_tokens(n, tokens)) {
        return NULL; // Syntax error found
    }
    if (n == 0) {
        parsed* p = calloc(1, sizeof(parsed));
        return p;
    }

    int is_bg=0;
    if(strcmp(tokens[n-1],"&")==0){
        is_bg=1;
        n--;
    }
    if(!validate_tokens(n,tokens)){
        return NULL;
    }
    if(n==0 && is_bg){
        return NULL;
    }

    if(n==0){
        parsed* p=calloc(1, sizeof(parsed));
        return p;
    }
    if ((input[0] == '"' && input[strlen(input)-1] == '"') || 
    (input[0] == '\'' && input[strlen(input)-1] == '\'')) {
    // Return NULL to indicate invalid syntax
    return NULL;
    }

// Also check if string is all whitespace
    int all_whitespace = 1;
    for (int i = 0; input[i]; i++) {
        if (!isspace(input[i])) {
            all_whitespace = 0;
            break;
        }
    }
    if (all_whitespace) {
        return NULL;
    }
    //LLM Generated Code Begins
    parsed* p = calloc(1, sizeof(parsed));
    p->commands = calloc(1024, sizeof(operators*));
    p->separators = calloc(1024, sizeof(char*));
    p->background = is_bg;


    int token_idx = 0;
    while(token_idx<n){
        operators* cmd = calloc(1, sizeof(operators));
        cmd->argv = calloc(1024, sizeof(char*));
    cmd->input_files = calloc(128, sizeof(char*));
    cmd->output_files = calloc(128, sizeof(char*));
    cmd->output_append = calloc(128, sizeof(int));
        
        // Parse one simple command (atomic)
        while (token_idx < n && strcmp(tokens[token_idx], "|") != 0 && strcmp(tokens[token_idx], "&") != 0 && strcmp(tokens[token_idx], ";") != 0) {
            if (strcmp(tokens[token_idx], "<") == 0) {
                token_idx++;
        // Record all input redirections (only last effective later)
        cmd->input_files[cmd->in_count++] = strdup(tokens[token_idx]);
        // Maintain legacy single slot to avoid breaking existing logic; last write wins
        if(cmd->input_file) free(cmd->input_file);
        cmd->input_file = strdup(tokens[token_idx]);
                
            } else if (strcmp(tokens[token_idx], ">") == 0) {
                token_idx++;
        cmd->output_files[cmd->out_count] = strdup(tokens[token_idx]);
        cmd->output_append[cmd->out_count++] = 0;
        if(cmd->output_file) free(cmd->output_file);
        cmd->output_file = strdup(tokens[token_idx]);
        cmd->append_output = 0;
            } else if (strcmp(tokens[token_idx], ">>") == 0) {
                token_idx++;
        cmd->output_files[cmd->out_count] = strdup(tokens[token_idx]);
        cmd->output_append[cmd->out_count++] = 1;
        if(cmd->output_file) free(cmd->output_file);
        cmd->output_file = strdup(tokens[token_idx]);
        cmd->append_output = 1;
            } else {
                cmd->argv[cmd->argc++] = strdup(tokens[token_idx]);
            }
            token_idx++;
        }
        if(cmd->argc>0){
            p->commands[p->num_commands++] = cmd;
        }
        else{
            free(cmd->argv);
            free(cmd);
            p->commands[p->num_commands++] = NULL;
        }
    

        // If there are more tokens, it must be a separator
        if (token_idx < n) {
            p->separators[p->num_commands - 1] = strdup(tokens[token_idx]);
            token_idx++;
        }
    }
    //LLM Generated Code Ends
    return p;
}
//LLM Generated Code Begins
void free_parsed_input(parsed* p) {
    if(!p){
        return;
    }
    for (int i = 0; i < p->num_commands; i++) {
        operators* cmd = p->commands[i];
        if(!cmd) continue;
        for (int j = 0; j < cmd->argc; j++) {
            free(cmd->argv[j]);
        }
        free(cmd->argv);
        if(cmd->input_file) free(cmd->input_file);
        if(cmd->output_file) free(cmd->output_file);
    for(int k=0;k<cmd->in_count;k++) free(cmd->input_files[k]);
    for(int k=0;k<cmd->out_count;k++) free(cmd->output_files[k]);
    free(cmd->input_files);
    free(cmd->output_files);
    free(cmd->output_append);
        free(cmd);
        if (i < p->num_commands - 1) {
            free(p->separators[i]);
        }
    }
    free(p->commands);
    free(p->separators);
    free(p);
}
//LLM Generated Code Ends