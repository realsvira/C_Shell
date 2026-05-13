#ifndef PARSER_H
#define PARSER_H

// Represents a single simple command 
// e.g., "ls -l", "wc", "cat > out.txt"
typedef struct operators {
    int argc;       // Number of arguments
    char** argv;    // Argument list, e.g., {"ls", "-l", NULL}
    char* input_file;
    char* output_file;
    int append_output; // Flag for '>>'
    // Support multiple redirections: store every occurrence to enforce
    // "only the last one takes effect" while validating/creating earlier ones.
    //LLM Generated Code Begins
    char** input_files;      // list of all input redirection filenames in order
    int in_count;
    char** output_files;     // list of all output redirection filenames in order
    int* output_append;      // parallel array: 0 for '>', 1 for '>>'
    int out_count;
    //LLM Generated Code Ends
} operators;

// Represents the entire command line, which is a sequence of simple commands
// connected by operators like '|', '&&', or '&'.
typedef struct parsed {
    operators** commands; // Array of simple commands
    int num_commands;
    char** separators;        // Array of separators between commands
    int background;           // Flag for trailing '&'
} parsed;

// Parses the raw input string and Returns a pointer to a parsed struct on success, or NULL on syntax error.
parsed* parse_input(char* input);
void free_parsed_input(parsed* parsed_input);

#endif