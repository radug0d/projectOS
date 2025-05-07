#define _POSIX_C_SOURCE 200809L

#include "customs.h"
#include "monitor_state.h"

#define CMD_FILE ".monitor_cmd"

void print_prompt() {
    const char *prompt = "> ";
    safe_print(prompt);
}

void write_command(const char *cmd) {
    int fd = open(CMD_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        abandonCSTM();
    }
    if (write(fd, cmd, strlen(cmd)) == -1 || write(fd, "\n", 1) == -1) {
        close(fd);
        abandonCSTM();
    }
    if (close(fd) == -1) {
        safe_print("Failed to close command file\n");
    }
}

typedef struct {
    const char *name;
    void (*handler)(const char *cmd);
    bool requires_monitor;
} command_t;

void handle_help(const char *cmd){
    const char *help_message = 
        "Available commands:\n"
        "  start_monitor: Start the monitor process\n"
        "  list_hunts: List all hunts and their treasures\n"
        "  list_treasures <hunt_id>: List all treasures in a specific hunt\n"
        "  view_treasure <hunt_id> <treasure_id>: View details of a specific treasure\n"
        "  stop_monitor: Stop the monitor process\n"
        "  exit: Exit the program\n";
    safe_print(help_message);
}

void handle_start_monitor(const char *cmd){
    if (monitor_ex->state == RUNNING){
        safe_print("Monitor is already running\n");
        return;
    }

    monitor_ex->pid = fork();
    if (monitor_ex->pid == -1){
        abandonCSTM();
    }
    else if (monitor_ex->pid == 0){
        char *args[] = {"./monitor", NULL};
        execv("./monitor", args);
        abandonCSTM(); 
    }
    else {
        monitor_ex->state = RUNNING;
        safe_print("Monitor started successfully.\n");
    }
}

void handle_monitor_command(const char *cmd){
    write_command(cmd);
    if (kill(monitor_ex->pid, SIGUSR1) == -1){
        abandonCSTM();
    }
}

void handle_stop_monitor(const char *cmd){
    monitor_ex->state = SHUTTING_DOWN;
    if (kill(monitor_ex->pid, SIGTERM) == -1){
        safe_print("Failed to send SIGTERM to monitor.\n");
        if (kill(monitor_ex->pid, SIGKILL) == -1){
            abandonCSTM();
        }
    }
}

void handle_exit(const char *cmd){
    exit(EXIT_SUCCESS);
}

void handle_sigchld(int sig){
    int status;
    pid_t pid = wait(&status);
    if (pid == -1){
        safe_print("Error waiting for child process.\n");
    }
    else {
        safe_print("Monitor process has exited.\n");
        monitor_ex->state = OFFLINE;
        monitor_ex->pid = 0;
    }
}

command_t commands[] = {
    {"help", handle_help, false},
    {"start_monitor", handle_start_monitor, false},
    {"list_hunts", handle_monitor_command, true},
    {"list_treasures", handle_monitor_command, true},
    {"view_treasure", handle_monitor_command, true},
    {"stop_monitor", handle_stop_monitor, true},
    {"exit", handle_exit, false},
    {NULL, NULL, false}
};

int main(){
    struct sigaction sa;
    memset(&sa,0,sizeof(struct sigaction));
    
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGUSR1,&sa,NULL) == -1){
        safe_print(strerror(errno));
    }

    sa.sa_handler = handle_sigchld;
    if (sigaction(SIGCHLD,&sa,NULL) == -1){
        safe_print(strerror(errno));
    }

    char input[256];
    while (1) {
        print_prompt();
        ssize_t bytes_read = read(STDIN_FILENO, input, sizeof(input) - 1);
        if (bytes_read == -1){
            if (errno == EINTR) {
                continue; 
            }
            else{
                abandonCSTM();
            }
        }
        if (bytes_read == 0) break;

        input[bytes_read] = '\0';
        input[strcspn(input, "\n")] = 0;
        trim_string(input);
        if (strlen(input) == 0) continue;

        char command[256];
        char *iterator = command;
        strncpy(command, input, sizeof(command) - 1);
        while (!isspace(*iterator) && *iterator != '\0') {
            iterator++;
        }
        *iterator = '\0'; 

        if (monitor_ex->state == SHUTTING_DOWN){
            safe_print("Cannot process commands while shutting down.\n");
            continue;
        }

        if (strcmp(input,"help") == 0){
            handle_help(input);
            continue;
        }

        command_t *cmd = commands;
        bool command_found = false;
        
        while (cmd->name != NULL) {
            if (strncmp(command, cmd->name, strlen(cmd->name)) == 0) {
                command_found = true;

                if (cmd->requires_monitor && monitor_ex->state == OFFLINE) {
                    const char *msg = "Monitor is not running...\n";
                    safe_print(msg);
                } 
                else if (strcmp(cmd->name, "exit") == 0 && monitor_ex->state == RUNNING) {
                    const char *msg = "Monitor still running. Stop it before exiting\n";
                    safe_print(msg);
                }
                else {
                    cmd->handler(input);
                }
            }
            cmd++;
        }

        if (!command_found) {
            const char *msg = "Unknown command. Type 'help' for more information\n";
            safe_print(msg);
        }
    }

    return 0;
}