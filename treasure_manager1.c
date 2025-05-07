#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
    #define unlink _unlink
#else
    #include <unistd.h>
    #include <dirent.h>
#endif

#include "treasure.h"
#include "operations.h"
#include "customs.h"

#define MAX_USER 32
#define MAX_CLUE 128
#define RECORD_FILE "treasures.dat"
#define LOG_FILE "logged_hunt"

typedef struct {
    int id;
    char username[MAX_USER];
    float latitude, longitude;
    char clue[MAX_CLUE];
    int value;
} Treasure;

void log_action(const char* hunt_id, const char* action) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, LOG_FILE);
    FILE* log = fopen(path, "a");
    if (!log) return;
    time_t now = time(NULL);
    fprintf(log, "[%s] %s\n", strtok(ctime(&now), "\n"), action);
    fclose(log);
}

void ensure_dir(const char* hunt_id) {
    if (mkdir(hunt_id, 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        exit(1);
    }
}

void add_treasure(const char* hunt_id) {
    ensure_dir(hunt_id);
    Treasure t;
    printf("ID: "); scanf("%d", &t.id);
    printf("User: "); scanf("%s", t.username);
    printf("Lat: "); scanf("%f", &t.latitude);
    printf("Long: "); scanf("%f", &t.longitude);
    printf("Clue: "); getchar(); fgets(t.clue, MAX_CLUE, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;
    printf("Value: "); scanf("%d", &t.value);

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, RECORD_FILE);

    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND | O_BINARY, 0644);
    if (fd < 0) { perror("open"); exit(1); }

    write(fd, &t, sizeof(Treasure));
    close(fd);
    log_action(hunt_id, "Added treasure");
}

void list_treasures(const char* hunt_id) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, RECORD_FILE);
    struct stat st;
    if (stat(filepath, &st) != 0) { perror("stat"); return; }

    printf("Hunt: %s\nSize: %lld bytes\n", hunt_id, (long long)st.st_size);
    printf("Last modified: %s", ctime(&st.st_mtime));

    int fd = open(filepath, O_RDONLY | O_BINARY);
    if (fd < 0) { perror("open"); return; }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("ID: %d | User: %s | Value: %d\n", t.id, t.username, t.value);
    }
    close(fd);
}

void remove_hunt(const char* hunt_id) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", hunt_id);
    system(cmd);
    printf("Hunt %s removed.\n", hunt_id);
}

void processTwoArguments(char **argv) {
    if (strcmp(argv[1], "--add") == 0) {
        add_treasure(argv[2]);
    } else if (strcmp(argv[1], "--list") == 0) {
        list_treasures(argv[2]);
    } else if (strcmp(argv[1], "--remove_hunt") == 0) {
        remove_hunt(argv[2]);
    } else {
        printInvalidArguments();
    }
}

void processThreeArguments(char **argv) {
    if (strcmp(argv[1], "--view") == 0) {
        view_treasure(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "--remove_treasure") == 0) {
        remove_treasure(argv[2], atoi(argv[3]));
    } else {
        printInvalidArguments();
    }
}

int main(int argc, char **argv) {
    switch (argc) {
        case 2: {
            if (strcmp(argv[1], "--help") == 0) {
                helpUser();
            } else {
                printInvalidArguments();
            }
            break;
        }
        case 3: {
            processTwoArguments(argv);
            break;
        }
        case 4: {
            processThreeArguments(argv);
            break;
        }
        default: {
            printInvalidArguments();
            break;
        }
    }
    return 0;
}