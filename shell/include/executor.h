#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"
#include <sys/types.h>
#include <unistd.h>
pid_t execute_pipe(operators** commands,int num_commands,int in_bg,const char* shell_home, const char* prev_cwd);

#endif