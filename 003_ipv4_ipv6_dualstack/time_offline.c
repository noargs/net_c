#include <stdio.h>
#include <time.h>

int main() {

    time_t timer;
    time(&timer); // unix time type time_t -> long

    //printf("timer before ctime: %ld\n", timer);
    printf("Local time is: %s\n", ctime(&timer)); // Tue May 16 10:05:42 2022

    return 0;
}