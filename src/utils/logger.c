#include <stdio.h>
#include <time.h>
#include "logger.h"

#define LOG_FILE "log/personality.log"

void log_action(const char *message) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) {
        perror("[ERROR] Failed to open log file");
        return;
    }

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buffer[20];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log, "[%s] %s\n", time_buffer, message);
    fclose(log);
}
