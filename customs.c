#include "customs.h"
#include "treasure.h"

void abandonCSTM(){
    char temp[ERROR_BUFFER_SIZE];
    sprintf(temp, "%s", strerror(errno));
    if (write(STDERR_FILENO, temp, sizeof(temp)) == 0){
        ;
    }
    exit(EXIT_FAILURE);
}

DIR* openDirectoryCSTM(char *DirName){
    DIR *dirID = NULL;
    if ((dirID = opendir(DirName)) == NULL){
        abandonCSTM();
    }
    return dirID;
}

void closeDirectoryCSTM(DIR* DirID){
    if (closedir(DirID) == -1){
        abandonCSTM();
    }
}

void createDirectoryCSTM(char *DirName){
    DIR *DirID = NULL;
    if ((DirID = opendir(DirName)) == NULL){
        if (errno == ENOENT){
            int check = 0;
            if ((check = mkdir(DirName, 0775)) == -1)
            {
                abandonCSTM();
            }
        }
    }
    else {
        closeDirectoryCSTM(DirID);
    }
}

bool runThroughCheckDirCSTM(char *DirName)
{
    if (access(DirName, F_OK) != -1){
        return true;
    }
    return false;
}

bool isTreasureAvailable(char *path, char *treasureID){
    int fileID;
    if ((fileID = open(path, O_RDONLY)) == -1){
        abandonCSTM();
    }

    Treasure_t ActiveTreasure;
    int readCheck = 0;
    while ((readCheck = read(fileID,&ActiveTreasure,sizeof(ActiveTreasure))) != 0){
        if (readCheck == -1){
            abandonCSTM();
        }
        if (strcmp(treasureID,ActiveTreasure.treasure_id) == 0){
            if (close(fileID) == -1){
                abandonCSTM();
            }
            return false;
        }
    }
    return true;
}

void trim_string(char *str) {
    if (str == NULL)
        return;
        
    char *start = str;
    
    while (isspace(*start)) start++;
    
    if (*start == 0) {
        *str = 0;
        return;
    }
    
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--;
    
    *(end + 1) = 0;
    
    if (start != str) {
        while (*start) {
            *str = *start;
            str++;
            start++;
        }
        *str = 0;
    }
}

void safe_print(const char *msg) {
    if (write(STDOUT_FILENO, msg, strlen(msg)) == -1){
        ;
    }
}