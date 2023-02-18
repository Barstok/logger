#include "logger.h"

static struct logger logger;

void dump_signal_handler(int signo, siginfo_t *info, void *context)
{
    sem_post(&logger.sem_dump);
}

void config_signal_handler(int signo, siginfo_t *info, void *context)
{
    sem_post(&logger.sem_config);
    set_config(info->si_value.sival_int);
}

int logger_init(LogLevel level, char *logfile_path, char *dumpfile_path, Dump (*dump_function)(void))
{
    logger.level = level;
    logger.dumpfile_path = dumpfile_path;
    logger.dump_function = dump_function;
    logger.logfile = fopen(logfile_path, "a");
    if (logger.logfile == NULL)
        return -1;

    sem_init(&logger.sem_write_log, 0, 0);
    sem_init(&logger.sem_dump, 0, 0);
    sem_init(&logger.sem_config, 0, 0);

    if(logger.dumpfile_path && logger.dump_function){
        pthread_create(&logger.dump_thread, NULL, dump_log, NULL);
    }

    struct sigaction sig_action_dump;

    sigset_t dump_set;
    sigemptyset(&dump_set);
    sigfillset(&dump_set);

    sig_action_dump.sa_sigaction = dump_signal_handler;
    sig_action_dump.sa_mask = dump_set;
    sig_action_dump.sa_flags = SA_SIGINFO;

    sigaction(SIGRTMIN, &sig_action_dump, NULL);

    struct sigaction sig_action_config;

    sigset_t config_set;
    sigemptyset(&config_set);
    sigfillset(&config_set);

    sig_action_config.sa_sigaction = config_signal_handler;
    sig_action_config.sa_mask = config_set;
    sig_action_config.sa_flags = SA_SIGINFO;

    sigaction(SIGRTMIN + 1, &sig_action_config, NULL);

    logger.is_initialized = 1;
}

int logger_close()
{
    if (!logger.is_initialized)
        return -1;

    if(logger.dumpfile_path && logger.dump_function){
        pthread_cancel(logger.dump_thread);
    }

    fclose(logger.logfile);

    sem_destroy(&logger.sem_dump);
    sem_destroy(&logger.sem_write_log);
    sem_destroy(&logger.sem_config);

    logger.is_initialized = 0;
}

void *dump_log(void *args)
{
    while (1)
    {
        sem_wait(&logger.sem_dump);

        char filename[200];
        sprintf(filename, "%s/dumpfile_%s_.dmp", logger.dumpfile_path, get_current_time());

        FILE *dumpfile = fopen(filename, "wb");

        Dump dump = logger.dump_function();

        fwrite(&dump.dump, dump.dump_size, 1, dumpfile);
        
        fclose(dumpfile);
    }
}

int log_msg(LogLevel level, char *msg)
{
    if (!logger.is_initialized)
        return -1;

    if (level < logger.level || logger.level == OFF)
        return 0;

    sem_wait(&logger.sem_write_log);
    fprintf(logger.logfile, "%s %s %s\n", get_LogLevel_name(level), get_current_time(), msg);
    sem_post(&logger.sem_write_log);

    return 1;
}

void set_config(LogLevel new_level)
{
    sem_wait(&logger.sem_config);
    if (!logger.is_initialized)
        return -1;
    logger.level = new_level;
}

const char *get_LogLevel_name(LogLevel level)
{
    switch (level)
    {
    case MIN:
        return "MIN";
    case STANDARD:
        return "STANDARD";
    case MAX:
        return "MAX";
    }
}

const char *get_current_time()
{
    time_t mytime = time(NULL);
    char *time_str = ctime(&mytime);
    time_str[strlen(time_str) - 1] = '\0';
    return time_str;
}