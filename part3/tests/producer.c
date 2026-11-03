#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "wrappers.h"

int rnd(int min, int max) { return rand() % (max - min + 1) + min; }

int main(int argc, char **argv) {
    int i, num, type, start, dest;
    srand(time(0));
    if (argc != 2) {
        printf("wrong number of args. producer.x num_of_requests\n");
        return -1;
    }
    sscanf(argv[1], "%d", &num);
    for (i = 0; i < num; i++) {
        type = rnd(0, 3);
        start = rnd(1, 6);
        do { dest = rnd(1, 6); } while (dest == start);
        long ret = issue_request(start, dest, type);
        printf("Issue (%d, %d, %d) returned %ld\n", start, dest, type, ret);
    }
    return 0;
}
