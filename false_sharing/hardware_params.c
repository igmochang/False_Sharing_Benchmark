#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX_CPU_INFO_LEN 1024

// Function to read CPU information from /proc/cpuinfo
void read_cpu_info() {
    char line[MAX_CPU_INFO_LEN];
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (file == NULL) {
        perror("Error opening /proc/cpuinfo");
        return;
    }

    printf("CPU Information:\n");

    while (fgets(line, MAX_CPU_INFO_LEN, file) != NULL) {
        printf("%s", line);
    }

    fclose(file);
}

// Function to read CPU topology information
void read_topology_info() {
    int cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of Cores: %d\n", cpu_count);

    // Determine cache information
    char cache_path[256];
    int i;
    for (i = 0; i < cpu_count; i++) {
        snprintf(cache_path, sizeof(cache_path), "/sys/devices/system/cpu/cpu%d/cache", i);
        int fd = open(cache_path, O_RDONLY | __O_DIRECTORY);

        if (fd >= 0) {
            printf("Cache information for CPU %d:\n", i);

            char cache_info[MAX_CPU_INFO_LEN];
            ssize_t read_len = read(fd, cache_info, sizeof(cache_info));
            if (read_len >= 0) {
                printf("%.*s\n", (int)read_len, cache_info);
            } else {
                perror("Error reading cache information");
            }

            close(fd);
        } else {
            perror("Error opening cache directory");
        }
    }
}

int main() {
    read_cpu_info();
    read_topology_info();

    return 0;
}