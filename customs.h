#ifndef CUSTOMS_H__
#define CUSTOMS_H__

#include "treasure.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#define DIRNAMESIZE 20
#define ERROR_BUFFER_SIZE 20
#define TEXT_BUFFER 60
#define LONG_TEXT 1024

void abandonCSTM();

DIR* openDirectoryCSTM(char *DirName);
void closeDirectoryCSTM(DIR *DirID);
void createDirectoryCSTM(char *DirName);
bool runThroughCheckDirCSTM(char *DirName);
bool isTreasureAvailable(char *path, char *treasureID);
void trim_string(char *str);

#endif