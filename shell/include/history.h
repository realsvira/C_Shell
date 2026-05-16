#ifndef HISTORY_H
#define HISTORY_H

void load_history();
void save_history();

void add_to_history(const char* command);
void execute_log(int argc,char* argv[]);

char* get_history_command(int index);

#endif