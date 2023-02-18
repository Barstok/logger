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

int logger_init(LogLevel level, char *dumpfile_path, char *logfile_path)
{
    logger.level = level;
    logger.dumpfile_path = dumpfile_path;
    logger.logfile = fopen(logfile_path, "a");
    if (logger.logfile == NULL)
        return -1;

    sem_init(&logger.sem_write_log, 0, 0);
    sem_init(&logger.sem_dump, 0, 0);
    sem_init(&logger.sem_config, 0, 0);

    pthread_create(&logger.dump_thread, NULL, dump_log, NULL);

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

    sigaction(SIGRTMIN+1, &sig_action_config, NULL);

    logger.is_initialized = 1;
}

int logger_close()
{
    if (!logger.is_initialized)
        return -1;

    pthread_cancel(logger.dump_thread);

    fclose(logger.logfile);

    sem_destroy(&logger.sem_dump);
    sem_destroy(&logger.sem_write_log);
    sem_destroy(&logger.sem_config);
}

void *dump_log(void *args)
{
    while (1)
    {
        fflush(stdout);
        sem_wait(&logger.sem_dump);
        FILE *dumpfile = fopen(logger.dumpfile_path, "wb");

        fwrite(&logger, sizeof(LogLevel), 1, dumpfile);

        fclose(dumpfile);
    }
}

int log_msg(LogLevel level, char *msg)
{
    if (!logger.is_initialized)
        return -1;

    if (level < logger.level)
        return 0;

    sem_wait(&logger.sem_write_log);
    fprintf(logger.logfile, "%s %s\n", getLogLevelName(level), msg);
    sem_post(&logger.sem_write_log);

    return 1;
}

void set_config(LogLevel new_level){
    sem_wait(&logger.sem_config);

    logger.level = new_level;
}

const char *getLogLevelName(LogLevel level)
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