#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "philosophers/philosophers.h"
#include "best_fit/best_fit.h"
#include "worst_fit/worst_fit.h"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        printf("simulator expects at least one argument: either -d, -b, -w\n");
        return 1;
    }

    if(strlen(argv[1]) != 2) {
        printf("bad argument. argument must be either -d, -b, -w\n");
        return 1;
    }

    switch (argv[1][1]) {
        case 'd':
            int nthreads = 5;
            if(argc > 3 && strcmp(argv[2], "-t") == 0) {
                int val = atoi(argv[3]);
                if(val) nthreads = val;
            }
            philosophers_start_dinning(nthreads);
            break;
        case 'b':
            break;
        case 'w':
            break;
        default:
            printf("unrecognized argument. expected either -d, -b, -w\n");
            return 1;
    }

    return 0;
}
