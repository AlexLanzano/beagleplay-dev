#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int read_iio(int fd)
{
    char buffer[6] = {0};
    size_t bytes_read = read(fd, buffer, 5);
    lseek(fd, 0, SEEK_SET);
    return atoi(buffer);
}

int main(int argc, char **argv)
{
    int accel_x_file = open("/sys/bus/iio/devices/iio:device0/in_accel_x_raw", O_RDWR);
    int accel_y_file = open("/sys/bus/iio/devices/iio:device0/in_accel_y_raw", O_RDWR);
    int accel_z_file = open("/sys/bus/iio/devices/iio:device0/in_accel_z_raw", O_RDWR);
    if (accel_x_file < 0) {
        printf("x no good\n");
        return 0;
    }
    if (accel_y_file < 0) {
        printf("y no good\n");
        return 0;
    }
    if (accel_z_file < 0) {
        printf("z no good\n");
        return 0;
    }

    while (1) {
        int accel_x = read_iio(accel_x_file);
        int accel_y = read_iio(accel_y_file);
        int accel_z = read_iio(accel_z_file);

        printf("x: %i y: %i z: %i\n", accel_x, accel_y, accel_z);
        usleep(500000);
    }

}
