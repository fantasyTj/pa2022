#include <stdio.h>
#include <sys/time.h>

int main(){
    int msec = 5;
    struct timeval tv;
    while(1){
        while(tv.tv_usec/100000 < msec) gettimeofday(&tv, NULL);
        printf("dadida...\n");
        msec += 5;
    }

    return 0;
}