#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
// #include "../../libs/libndl/include/NDL.h"
uint32_t NDL_GetTicks();
int NDL_Init(uint32_t flags);


int main(){
    NDL_Init(0);
    int msec = 5;
    while(1){
        while(NDL_GetTicks()/100000 < msec) ;
        printf("dadida...\n");
        msec += 5;
    }

    return 0;
}