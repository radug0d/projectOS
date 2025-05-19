#include "treasure.h"
#include "operations.h"
#include "customs.h"

void twoArguments(char **argv){
    if (strcmp(argv[1],"--add") == 0){
        add(argv[2]);
    }
    else if (strcmp(argv[1],"--list") == 0){
        list(argv[2]);
    }
    else if (strcmp(argv[1],"--remove_hunt") == 0){
        remove_hunt(argv[2]);
    }
    else {
        printInvalidArguments();
    }
}

void threeArguments(char **argv){
    if (strcmp(argv[1],"--view") == 0){
        view(argv[2],argv[3]);
    }
    else if (strcmp(argv[1],"--remove_treasure") == 0){
        remove_treasure(argv[2],argv[3]);
    }
    else {
        printInvalidArguments();
    }
}

int main(int argc, char **argv){
    switch (argc){
        case 2:{
            if (strcmp(argv[1],"--help") == 0){
                helpUser();
            }
            else {
                printInvalidArguments();
            }
            break;
        }
        case 3:{
            twoArguments(argv);
            break;
        }
        case 4:{
            threeArguments(argv);
            break;
        }
        default:{
            printInvalidArguments();
            break;
        }
    }
    return 0;
}