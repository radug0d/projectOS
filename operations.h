#ifndef OPERATIONS_H__
#define OPERATIONS_H__

#include "treasure.h"
#include "customs.h"

void printInvalidArguments();
void helpUser();
void add(char *hunt_id);
void list(char *hunt_id);
void view(char *hunt_id,char *treasure_id);
void remove_treasure(char *hunt_id, char *treasure_id);
void remove_hunt(char *hunt_id);


#endif