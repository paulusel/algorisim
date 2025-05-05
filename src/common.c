#include "common.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int random_number() {
    int random_num;
    int fd = open("/dev/urandom", O_RDONLY);
    read(fd, &random_num, sizeof(random_num));
    close(fd);
    return abs(random_num);
}
