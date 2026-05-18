# C Shell

A fully-featured Unix-like shell implemented from scratch in C, built on top of low-level POSIX system calls.
Supports real-world shell workflows including multi-stage pipelines, background job management,
signal-safe process control, and persistent command history — all without relying on any shell library.

---

## Features

### Command Execution
- Executes any binary available on the system `PATH` via `fork()` + `execvp()`
- Passes arguments exactly as typed, including flags and options
- Returns and displays exit status of failed commands
- Handles executable not found and permission errors gracefully

### Pipelines
- Full support for multi-stage pipelines: `cmd1 | cmd2 | cmd3 | ...`
- Each stage runs as a separate child process with stdin/stdout wired via Unix pipes
- File descriptors are carefully managed and closed to prevent resource leaks

### Background Execution & Job Control
- Run any command in the background with `&`
- List all active background jobs with `jobs`, showing job ID, PID, and status
- Bring a background job to the foreground with `fg <job_id>`
- Send a foreground job to the background with `bg <job_id>`
- Automatically reaps finished background processes via `SIGCHLD`

### Signal Handling
- `Ctrl+C` (`SIGINT`) — interrupts the current foreground process without killing the shell
- `Ctrl+Z` (`SIGTSTP`) — suspends the foreground process and moves it to the background
- `SIGCHLD` — caught asynchronously to clean up zombie processes and update job status
- Shell itself is protected from being accidentally killed by user signals

### Built-in Commands
- `cd <dir>` — change the working directory (with `~` expansion)
- `exit` — cleanly exits the shell
- `history` — displays previously entered commands
- `jobs` — lists active background jobs
- `fg <n>` / `bg <n>` — foreground/background job control

### Command History
- Stores all commands entered during the session
- Retrieve and display full history with the `history` command
- History is maintained in-memory across the session lifecycle

### Prompt
- Displays a custom prompt with the current working directory
- Updates dynamically as you navigate the filesystem

---

## How It Works

```
User Input
    │
    ▼
 [input.c]  ──── reads raw line from stdin
    │
    ▼
 [parser.c] ──── tokenizes into command structs, detects pipes and & 
    │
    ▼
 [executor.c] ── forks child processes, sets up pipe chain, calls execvp()
    │         └── [signals.c] sets up signal masks per process
    │         └── [jobs.c]    registers background jobs in job table
    ▼
 [prompt.c] ──── prints next prompt, reaps finished jobs
```

---

## Project Structure

```
shell/
├── Makefile
├── include/
│   ├── commands.h
│   ├── executor.h
│   ├── history.h
│   ├── input.h
│   ├── jobs.h
│   ├── parser.h
│   ├── prompt.h
│   └── signals.h
└── src/
    ├── main.c
    ├── commands.c
    ├── executor.c
    ├── history.c
    ├── input.c
    ├── jobs.c
    ├── parser.c
    ├── prompt.c
    └── signals.c
```

| Module | Responsibility |
|--------|----------------|
| `main.c` | Entry point; drives the main REPL loop |
| `parser.c` | Tokenizes raw input into structured command representations |
| `executor.c` | Creates child processes, wires pipes, calls `execvp()` |
| `jobs.c` | Maintains the job table for background process tracking |
| `signals.c` | Installs and manages signal handlers for the shell and children |
| `commands.c` | Implements all built-in commands |
| `history.c` | Stores and retrieves command history |
| `input.c` | Reads raw user input from stdin |
| `prompt.c` | Renders the shell prompt |

---

## Build & Run

Requires `gcc` and a POSIX-compliant system (Linux / macOS).

```bash
make          # compiles all modules and links shell.out
./shell.out   # launch the shell
make clean    # remove build artifacts
```

Compiler flags used: `-std=c99 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L`

---

## Example Usage

```bash
# Basic command execution
echo "Hello, World!"
ls -lah /usr/bin

# Multi-stage pipeline
cat access.log | grep "404" | sort | uniq -c | sort -rn

# Background execution
sleep 30 &          # runs in background
jobs                # list background jobs: [1] 12345 Running  sleep 30
fg 1                # bring job 1 to foreground
# Ctrl+Z            # suspend it again
bg 1                # resume it in background

# Built-ins
cd ~/projects
history             # show all commands from this session
exit
```

---

## Technical Highlights

- **Zero external shell libraries** — built entirely on POSIX syscalls (`fork`, `execvp`, `pipe`, `dup2`, `waitpid`, `kill`, `sigaction`)
- **Modular architecture** — parsing, execution, job control, and signal handling are fully decoupled into separate modules
- **Safe signal handling** — signals are blocked/unblocked precisely around critical sections; children restore default handlers before `exec`
- **Pipe chain management** — file descriptors are tracked and closed at every stage to prevent descriptor leaks across a pipeline
- **Zombie-free** — `SIGCHLD` is caught and `waitpid` is called in a loop to reap all terminated children immediately
- **Robust error handling** — every syscall checks return values; errors are reported to stderr without crashing the shell

---

> Commit history was cleaned and structured for clarity after development.
