#ifndef COMMANDS_H
#define COMMANDS_H

void execute_hop(int argc, char* argv[], const char* shell_home, char* prev_cwd);
void execute_reveal(int argc, char* argv[], const char* shell_home, const char* prev_cwd);
void execute_log(int argc, char* argv[]);

void execute_activities();
void execute_ping(int argc, char* argv[]);
void execute_fg(int argc, char* argv[]);
void execute_bg(int argc, char* argv[]);
#endif

