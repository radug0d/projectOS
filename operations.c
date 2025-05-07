#include "customs.h"
#include "operations.h"
#include "treasure.h"

#define TIMESTAMP_SIZE 30

void printInvalidArguments(){
    char temp[TEXT_BUFFER];
        snprintf(temp,TEXT_BUFFER,"Invalid arguments...\n");
        if (write(STDERR_FILENO,temp,strlen(temp)) == -1){
            ;
        }

        snprintf(temp,TEXT_BUFFER,"Type './treasure_manager --help' for more information\n");
        if (write(STDOUT_FILENO,temp,strlen(temp)) == -1){
            ;
        }   
        exit(EXIT_FAILURE);
}

void helpUser() {
    const char *help_message = 
        "Usage: treasure_manager <command> [arguments]\n\n"
        "Commands:\n"
        "  --add <hunt_id>                  Add a new treasure to the specified hunt\n"
        "  --list <hunt_id>                 List all treasures in the specified hunt\n"
        "  --view <hunt_id> <treasure_id>   View details of a specific treasure\n"
        "  --remove_treasure <hunt_id> <treasure_id>\n"
        "                                 Remove a specific treasure from a hunt\n"
        "  --remove_hunt <hunt_id>          Remove an entire hunt and all its treasures\n"
        "  --help                           Display this help message\n\n"
        "Examples:\n"
        "  treasure_manager --add hunt001\n"
        "  treasure_manager --list hunt001\n"
        "  treasure_manager --view hunt001 treasure42\n";
    if (write(STDOUT_FILENO, help_message, strlen(help_message)) == -1) {
        abandonCSTM();
    }
}

void log_operation(const char *hunt_id, const char *operation, const char *details) {
    char cale[PATH_MAX];
    char log_entry[LONG_TEXT];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[TIMESTAMP_SIZE];
    
    strftime(timestamp, sizeof(timestamp), "%d-%m-%Y %H:%M:%S", tm_info);
    
    snprintf(log_entry, sizeof(log_entry), "[%s] %s: %s\n", 
             timestamp, operation, details);
    
    snprintf(cale, sizeof(cale), "%s/logged_hunt.txt", hunt_id);
    
    int fileId = open(cale, O_CREAT | O_WRONLY | O_APPEND, 0664);
    if (fileId == -1) {
        abandonCSTM();
    }
    
    if (write(fileId, log_entry, strlen(log_entry)) == -1) {
        close(fileId);
        abandonCSTM();
    }
    
    if (close(fileId) == -1) {
        abandonCSTM();
    }
}

void create_log_symlink(const char *hunt_id) {
    char target_path[PATH_MAX];
    char link_path[PATH_MAX];
    
    snprintf(target_path, sizeof(target_path), "%s/logged_hunt.txt", hunt_id);
    
    snprintf(link_path, sizeof(link_path), "logged_hunt--%s", hunt_id);
    
    unlink(link_path);
    
    if (symlink(target_path, link_path) == -1) {
        abandonCSTM();
    }
}

void add(char *hunt_id){
    if (hunt_id[strlen(hunt_id)-1] == '/'){
        hunt_id[strlen(hunt_id)-1] = '\0';
    }
    if (!(runThroughCheckDirCSTM(hunt_id))){
        createDirectoryCSTM(hunt_id);
    }

    char cale[PATH_MAX];
    char temp[TEXT_BUFFER];
    char input_buffer[LONG_TEXT];
    struct treasure ActiveTreasure;
    
    int stdin_data = 0;
    struct timeval tv;
    fd_set readfds;
    
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
        ssize_t bytes_read = 0;
        ssize_t total_read = 0;
        
        while ((bytes_read = read(STDIN_FILENO, input_buffer + total_read, 
                                 LONG_TEXT - total_read - 1)) > 0) {
            total_read += bytes_read;
            if (total_read >= LONG_TEXT - 1)
                break;
        }
        
        if (total_read > 0) {
            input_buffer[total_read] = '\0';
            stdin_data = 1;
        }
    }
    
    if (stdin_data) {
        parse_and_add_treasure(&ActiveTreasure, input_buffer);
    } else {
        add_treasure(&ActiveTreasure);
    }

    snprintf(cale, sizeof(cale), "%s/%s.dat", hunt_id, hunt_id);

    if (access(cale, F_OK) != 0 && errno == ENOENT) {
        int fileId = 0;
        if ((fileId = open(cale, O_CREAT, 00664)) == -1) {
            abandonCSTM();
        }

        if (close(fileId) == -1) {
            abandonCSTM();
        }
    }

    if (!(isTreasureAvailable(cale, ActiveTreasure.treasure_id))) {
        snprintf(temp, TEXT_BUFFER, "Treasure ID already used\n");
        if (write(STDERR_FILENO, temp, strlen(temp)) == -1) {
            abandonCSTM();
        }
        return;
    }

    int fileId = 0;
    if ((fileId = open(cale, O_CREAT | O_WRONLY | O_APPEND, 00664)) == -1) {
        abandonCSTM();
    }

    if (write(fileId, &ActiveTreasure, sizeof(struct treasure)) == -1) {
        abandonCSTM();
    }

    if (close(fileId) == -1) {
        abandonCSTM();
    }

    snprintf(temp, TEXT_BUFFER, "Added treasure with ID - %s", ActiveTreasure.treasure_id);
    log_operation(hunt_id, "Add hunt", temp);
    create_log_symlink(hunt_id);
}

void list(char *hunt_id){
    if (hunt_id[strlen(hunt_id)-1] == '/'){
        hunt_id[strlen(hunt_id)-1] = '\0';
    }
    char temp[LONG_TEXT];
    if (!(runThroughCheckDirCSTM(hunt_id))){
        sprintf(temp,"%s\n","No such directory");
        if (write(STDERR_FILENO,temp,strlen(temp)) == -1){
            abandonCSTM();
        }
        return;
    }

    char cale[PATH_MAX];
    snprintf(cale, sizeof(cale), "%s/%s.dat", hunt_id, hunt_id);
    struct stat fileStat;
    if (stat(cale,&fileStat) != 0){
        abandonCSTM();
    }

    sprintf(temp,"\nName: %s\n",hunt_id);
    if (write(STDOUT_FILENO,temp,strlen(temp)) == -1){
        abandonCSTM();
    }
    sprintf(temp,"Size: %ld\n",fileStat.st_size);
    if (write(STDOUT_FILENO,temp,strlen(temp)) == -1){
        abandonCSTM();
    }
    sprintf(temp,"Last modified - %s\n",ctime(&fileStat.st_mtime));
    if (write(STDOUT_FILENO,temp,strlen(temp)) == -1){
        abandonCSTM();
    }

    Treasure_t ActiveTreasure;
    sprintf(temp,"%s/%s.dat",hunt_id,hunt_id);
    int fileId = 0;
    if ((fileId = open(temp, O_RDONLY)) == -1){
        abandonCSTM();
    }

    int check = 0;
    while ((check = read(fileId,&ActiveTreasure,sizeof(ActiveTreasure))) != 0){
        if (check == -1){
            abandonCSTM();
        }
        snprintf(temp,TEXT_BUFFER,"ID: %s\n",ActiveTreasure.treasure_id);
        if (write(STDOUT_FILENO,temp,strlen(temp)) == -1){
            abandonCSTM();
        }
    }

    if (close(fileId) == -1){
        abandonCSTM();
    }

    snprintf(temp,TEXT_BUFFER,"Listed hunt with ID - %s",hunt_id);
    log_operation(hunt_id,"List hunt",temp);
    create_log_symlink(hunt_id);
}

void view(char *hunt_id,char *treasure_id){
    if (hunt_id[strlen(hunt_id)-1] == '/'){
        hunt_id[strlen(hunt_id)-1] = '\0';
    }
    char cale[PATH_MAX];
    char temp[LONG_TEXT];
    if (!(runThroughCheckDirCSTM(hunt_id))){
        snprintf(temp,sizeof(temp),"No such directory (%s)\n",hunt_id);
        if (write(STDERR_FILENO,temp,strlen(temp)) == -1){
            abandonCSTM();
        }
    }

    snprintf(cale,PATH_MAX,"%s/%s.dat",hunt_id,hunt_id);

    int fileReadID = 0;
    if ((fileReadID = open(cale,O_RDONLY)) == -1){
        abandonCSTM();
    }

    Treasure_t ActiveTreasure;

    int readCheck = 0;
    while ((readCheck = read(fileReadID,&ActiveTreasure,sizeof(ActiveTreasure))) != 0){
        if (readCheck == -1){
            abandonCSTM();
        }
        if (strcmp(treasure_id,ActiveTreasure.treasure_id) == 0){
            break;
        }
    }
    if (readCheck == 0){
        snprintf(temp,TEXT_BUFFER,"No matching treasure ID was found");
        log_operation(hunt_id,"View treasure",temp);
    }
    else {
        sprintf(temp,"\nTreasure ID: %s\nUser Name: %s\nCoordinate X: %lf\nCoordinate Y: %lf\nClue: %s\nValue: %d\n",
                ActiveTreasure.treasure_id,ActiveTreasure.user_name,ActiveTreasure.coordinateX,ActiveTreasure.coordinateY,ActiveTreasure.clue,ActiveTreasure.value);
                if (write(STDOUT_FILENO,temp,strlen(temp)) == -1){
                    abandonCSTM();
                }
                snprintf(temp,TEXT_BUFFER,"Showed details on treasure - %s",treasure_id);
                log_operation(hunt_id,"View treasure",temp);
    }
}

void remove_treasure(char *hunt_id, char *treasure_id){
    if (hunt_id[strlen(hunt_id)-1] == '/'){
        hunt_id[strlen(hunt_id)-1] = '\0';
    }
    char cale[PATH_MAX];
    char oldCale[PATH_MAX];
    char temp[TEXT_BUFFER];
    if (!(runThroughCheckDirCSTM(hunt_id))){
        snprintf(temp,sizeof(temp),"No such directory (%s)\n",hunt_id);
        if (write(STDERR_FILENO,temp,strlen(temp)) == -1){
            abandonCSTM();
        }
    }

    snprintf(cale,PATH_MAX,"%s/%s.dat",hunt_id,hunt_id);

    int fileReadID = 0, fileWriteID = 0;
    if ((fileReadID = open(cale,O_RDONLY)) == -1){
        abandonCSTM();
    }

    snprintf(cale,PATH_MAX,"%s/temp.data",hunt_id);

    if ((fileWriteID = open(cale,O_CREAT | O_WRONLY, 0664)) == -1){
        abandonCSTM();
    }

    Treasure_t ActiveTreasure;

    int readCheck = 0, writeCheck = 0;
    while ((readCheck = read(fileReadID,&ActiveTreasure,sizeof(ActiveTreasure))) != 0){
        if (readCheck == -1){
            abandonCSTM();
        }
        if (strcmp(treasure_id,ActiveTreasure.treasure_id) == 0){
            writeCheck = 1;
            continue;
        }
        if (write(fileWriteID,&ActiveTreasure,sizeof(ActiveTreasure)) == -1){
            abandonCSTM();
        }
    }

    if (close(fileReadID) == -1){
        abandonCSTM();
    }

    if (close(fileWriteID) == -1){
        abandonCSTM();
    }

    snprintf(oldCale,PATH_MAX,"%s/%s.dat",hunt_id,hunt_id);
    if (remove(oldCale) == -1){
        abandonCSTM();
    }

    if (rename(cale,oldCale) == -1){
        abandonCSTM();
    }

    if (writeCheck == 1){
        snprintf(temp,TEXT_BUFFER,"Removed treasure with id - %s",treasure_id);
        log_operation(hunt_id,"Remove treasure",temp);
        create_log_symlink(hunt_id);
    }
    else {
        snprintf(temp,TEXT_BUFFER,"No matching treasure ID was found");
        log_operation(hunt_id,"Remove treasure",temp);
        create_log_symlink(hunt_id);
    }
}

void remove_hunt(char *hunt_id){
    if (hunt_id[strlen(hunt_id)-1] == '/'){
        hunt_id[strlen(hunt_id)-1] = '\0';
    }
    char temp[TEXT_BUFFER];
    char cale[PATH_MAX];

    if (!(runThroughCheckDirCSTM(hunt_id))){
        snprintf(temp,sizeof(temp),"No such directory (%s)\n",hunt_id);
        if (write(STDERR_FILENO,temp,strlen(temp)) == -1){
            abandonCSTM();
        }
    }

    DIR *dirID = openDirectoryCSTM(hunt_id);

    struct dirent *contents = NULL;
    errno = 0;
    while ((contents = readdir(dirID)) != NULL){
        if(strcmp(contents->d_name, ".") == 0 || strcmp(contents->d_name, "..")==0){
            continue;
        }
        snprintf(cale,PATH_MAX,"%s/%s",hunt_id,contents->d_name);
        if (remove(cale) == -1){
            abandonCSTM();
        }
    }
    if (contents == NULL && errno != 0){
        abandonCSTM();
    }

    closeDirectoryCSTM(dirID);

    if (rmdir(hunt_id) == -1){
        abandonCSTM();
    }
    
    snprintf(cale, PATH_MAX, "logged_hunt--%s", hunt_id);
    if (unlink(cale) == -1){
        abandonCSTM();
    }
}