#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s.dat", argv[1], argv[1]);
    
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    typedef struct {
        char name[20];
        int score;
    } UserScore;
    
    UserScore scores[100] = {0};
    int count = 0;
    
    Treasure_t t;
    while (fread(&t, sizeof(Treasure_t), 1, fp) == 1) {
        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(scores[i].name, t.user_name) == 0) {
                scores[i].score += t.value;
                found = 1;
                break;
            }
        }
        if (!found && count < 100) {
            strncpy(scores[count].name, t.user_name, sizeof(scores[0].name)-1);
            scores[count].score = t.value;
            count++;
        }
    }
    fclose(fp);

    for (int i = 0; i < count; i++) {
        printf("%s: %d\n", scores[i].name, scores[i].score);
    }
    
    return EXIT_SUCCESS;
}