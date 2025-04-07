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

void view_treasure(const char* hunt_id, int id) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, RECORD_FILE);
    int fd = open(filepath, O_RDONLY | O_BINARY);
    if (fd < 0) { perror("open"); return; }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.id == id) {
            printf("User: %s\nCoords: (%f, %f)\nClue: %s\nValue: %d\n",
                   t.username, t.latitude, t.longitude, t.clue, t.value);
            close(fd);
            return;
        }
    }
    printf("Treasure not found.\n");
    close(fd);
}

void remove_treasure(const char* hunt_id, int id) {
    char filepath[256], temp[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, RECORD_FILE);
    snprintf(temp, sizeof(temp), "%s/temp.dat", hunt_id);

    int in = open(filepath, O_RDONLY | O_BINARY);
    int out = open(temp, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);
    if (in < 0 || out < 0) { perror("open"); return; }

    Treasure t;
    int found = 0;
    while (read(in, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.id != id) write(out, &t, sizeof(Treasure));
        else found = 1;
    }
    close(in); close(out);
    remove(filepath);
    rename(temp, filepath);
    if (found) log_action(hunt_id, "Removed treasure");
    else printf("Treasure not found.\n");
}

void remove_hunt(const char* hunt_id) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", hunt_id);
    system(cmd); // simplu și rapid
    printf("Hunt %s removed.\n", hunt_id);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s --add|--list|--view|--remove|--remove_hunt <hunt_id> [id]\n", argv[0]);
        return 1;
    }

    const char* cmd = argv[1];
    const char* hunt_id = argv[2];

    if (strcmp(cmd, "--add") == 0) add_treasure(hunt_id);
    else if (strcmp(cmd, "--list") == 0)    list_treasures(hunt_id);
    else if (strcmp(cmd, "--view") == 0 && argc >= 4) view_treasure(hunt_id, atoi(argv[3]));
    else if (strcmp(cmd, "--remove") == 0 && argc >= 4) remove_treasure(hunt_id, atoi(argv[3]));
    else if (strcmp(cmd, "--remove_hunt") == 0) remove_hunt(hunt_id);
    else printf("Invalid command.\n");

    return 0;
}
