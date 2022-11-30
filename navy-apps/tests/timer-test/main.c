#include <stdio.h>
#include <sys/time.h>
#include <NDL.h>

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