#ifndef LOGGER_H
#define LOGGER_H

#include <semaphore.h>
#include <pthread.h>
#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum{
    OFF,
    MIN,
    STANDARD,
    MAX
} LogLevel;

struct logger{
    int is_initialized;
    LogLevel level;
    char* dumpfile_path;
    FILE* logfile;
    sem_t sem_dump;
    sem_t sem_write_log;
    sem_t sem_config;
    pthread_t dump_thread;
    pthread_t config_thread;
};

int logger_init(LogLevel level, char* dumpfile_path, char* logfile_path);
int logger_close();

void* dump_log(void* args);

void set_config(LogLevel new_level);
int log_msg(LogLevel level, char *msg);

const char* getLogLevelName(LogLevel level);

#endif