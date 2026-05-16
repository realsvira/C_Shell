#include "signals.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

volatile sig_atomic_t foregrnd_pid = 0;
volatile sig_atomic_t child_exited = 0;

// Handler for SIGINT (Ctrl-C)
void handle_sigint(int signo) {
    // If there is a foreground process running...
    if (foregrnd_pid != 0) {
        // ...send SIGINT to it.
        kill(-foregrnd_pid, SIGINT);
    }
}

// Handler for SIGTSTP (Ctrl-Z)
void handle_sigtstp(int signo) {
    // If there is a foreground process running...
    if (foregrnd_pid != 0) {
        // ...send SIGTSTP to it to stop it.
        kill(-foregrnd_pid, SIGTSTP);
    }
}

void handle_sigchld(int signo) {
    child_exited = 1;
    
}
//LLM Generated Code Begins
void set_signal_handlers() {
    // Create structs to define the new signal actions.
    struct sigaction sa_int, sa_tstp, sa_chld;
    memset(&sa_int, 0, sizeof(sa_int));
    memset(&sa_tstp, 0, sizeof(sa_tstp));
    memset(&sa_chld, 0, sizeof(sa_chld));

    // Configure SIGINT handler
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    // Configure SIGTSTP handler
    sa_tstp.sa_handler = handle_sigtstp;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    // Configure SIGCHLD handler - REMOVE SA_RESTART!
    sa_chld.sa_handler = handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_NOCLDSTOP; // No SA_RESTART here!
    sigaction(SIGCHLD, &sa_chld, NULL);

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}
//LLM Generated Code Ends