#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>

extern volatile sig_atomic_t foregrnd_pid;
extern volatile sig_atomic_t child_exited;
void set_signal_handlers();
void handle_sigchld(int signo); 
#endif