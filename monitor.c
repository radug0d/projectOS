#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "customs.h"
#include "monitor_state.h"

#define CMD_FILE ".monitor_cmd"
#define BUFFER_SIZE 1024
#define PATH_BUFFER 512

void handle_term(){
    sleep(5);
    exit(EXIT_SUCCESS);
}

void list_hunts() {
    DIR *d = opendir(".");
    if (!d){
        abandonCSTM();
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_type != DT_DIR) continue;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".git") == 0) continue;

        char data_file[BUFFER_SIZE];
        snprintf(data_file, sizeof(data_file), "%s/%s.dat", entry->d_name, entry->d_name);
        printf("%s\n", data_file);

        int fd = open(data_file, O_RDONLY);
        if (fd == -1) {
            safe_print(strerror(errno));
            continue;
        }

        int count = 0;
        struct treasure t;
        ssize_t n;
        while ((n = read(fd, &t, sizeof(t))) > 0) {
            if (n != sizeof(t)) break;
            count++;
        }
        close(fd);

        char msg[BUFFER_SIZE];
        snprintf(msg, sizeof(msg), "Hunt: %s | Treasures: %d\n", entry->d_name, count);
        write(STDOUT_FILENO, msg, strlen(msg));
    }

    closedir(d);
}

void list_treasures(const char *hunt_id) {
    char path[PATH_BUFFER];
    snprintf(path, sizeof(path), "%s/%s.dat", hunt_id, hunt_id);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        char msg[BUFFER_SIZE];
        snprintf(msg, sizeof(msg), "Could not open file: %s\n", path);
        write(STDOUT_FILENO, msg, strlen(msg));
        return;
    }

    struct treasure t;
    ssize_t n;
    while ((n = read(fd, &t, sizeof(t))) > 0) {
        if (n != sizeof(t)) break;

        pid_t pid = fork();
        if (pid < 0) {
            safe_print("Fork failed\n");
            close(fd);
            return;
        }
        else if (pid == 0) {
            execlp("./treasure_manager", "treasure_manager", "--view", hunt_id, t.treasure_id, NULL);
            abandonCSTM();
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }

    close(fd);
}

void view_treasure(const char *hunt_id, const char *treasure_id) {
    pid_t pid = fork();
    if (pid < 0) {
        safe_print("Fork failed\n");
        return;
    }
    else if (pid == 0) {
        execlp("./treasure_manager", "treasure_manager", "--view", hunt_id, treasure_id, (char *)NULL);
        abandonCSTM();
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}


void handle_command(){
    int fd = open(CMD_FILE, O_RDONLY);
    if (fd == -1) {
        abandonCSTM();
    }
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        close(fd);
        abandonCSTM();
    }

    buffer[bytes_read] = '\0';
    buffer[strcspn(buffer, "\n")] = '\0';
    trim_string(buffer);

    char command[256];
        char *iterator = command;
        strncpy(command, buffer, sizeof(command) - 1);
        while (!isspace(*iterator) && *iterator != '\0') {
            iterator++;
        }
        *iterator = '\0';

    if (strcmp(command, "list_hunts") == 0) {
        list_hunts();
    } 
    else if (strncmp(command, "list_treasures", 14) == 0) {
        const char *hunt_id = buffer + 15;
        trim_string((char *)hunt_id);
        list_treasures(hunt_id);
    }
    else if (strncmp(buffer, "view_treasure", 13) == 0) {
        char *args = buffer + 14;
        char *hunt_id = strtok(args, " ");
        char *treasure_id = strtok(NULL, " ");
    
        if (hunt_id && treasure_id) {
            view_treasure(hunt_id, treasure_id);
        } else {
            safe_print("Invalid view_treasure usage. Format: view_treasure <hunt_id> <treasure_id>\n");
        }
    }

    if (close(fd) == -1) {
        safe_print("failed to close the command file\n");
    }
}

int main(){
    struct sigaction sa;
    memset(&sa,0,sizeof(struct sigaction));

    sa.sa_handler = handle_term;
    if (sigaction(SIGTERM,&sa,NULL) == -1){
        abandonCSTM();
    }

    sa.sa_handler = handle_command;
    if (sigaction(SIGUSR1,&sa,NULL) == -1){
        abandonCSTM();
    }

    while (1){

    }
    return 0;
}