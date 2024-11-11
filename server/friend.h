#include <string.h>
#ifndef FRIEND_H
#define FRIEND_H



#define MAX_FRIEND 100

typedef struct {
    int num;
    int list_add[MAX_FRIEND];
}addfr;

typedef struct {
    int num;
    int* fr;
}listfr;

listfr init_list(){
    listfr x;
    x.num = 0;
    memset()
}


int checklistfr(listfr client) {
    if(client.num < MAX_FRIEND){
        return 1;
    }
    return 0;
}





#endif //FRIEND_H