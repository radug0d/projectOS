#include "customs.h"

#ifndef TREASURE_H__
#define TREASURE_H__

typedef struct treasure{
    char treasure_id[20];
    char user_name[20];
    double coordinateX;
    double coordinateY;
    char clue[200];
    int value;
}Treasure_t;

void add_treasure(Treasure_t* buff_struct);
void parse_and_add_treasure(Treasure_t* buff_struct, const char* input_string);


#endif