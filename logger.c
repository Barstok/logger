#include "logger.h"

static struct logger logger;

int logger_init(LogLevel level, char *dumpfile_path, char *logfile_path)
{
    printf("INITIALIZATION STARTED\n");
    logger.level = level;
    logger.dumpfile_path = dumpfile_path;
    logger.logfile = fopen(logfile_path,"a");
    if(logger.logfile == NULL) return -1;

    sem_init(&logger.sem_write_log, 0, 0);
    sem_init(&logger.sem_dump, 0, 0);

    int ret = pthread_create(&logger.dump_thread, NULL, dump_log, NULL);

    struct sigaction sig_action;

    sigset_t set;
    sigemptyset(&set);
    sigfillset(&set);

    sig_action.sa_sigaction = dump_signal_handler;
    sig_action.sa_mask = set;
    sig_action.sa_flags = SA_SIGINFO;

    ret = sigaction(SIGRTMIN, &sig_action, NULL);

    logger.is_initialized = 1;
    printf("INITIALIZATION FINISHED\n");
}

int logger_close()
{
    if(!logger.is_initialized) return -1;

    pthread_join(logger.dump_thread, NULL);

    fclose(logger.logfile);

    sem_destroy(&logger.sem_dump);
}

void dump_signal_handler(int signo, siginfo_t *info, void *other)
{
    sem_post(&logger.sem_dump);
}

void *dump_log(void *args)
{
    printf("Dump thread opened\n");
    fflush(stdout);
    while (1)
    {
        sem_wait(&logger.sem_dump);
        printf("Rozpoczeto zapis do dumpa\n");
        fflush(stdout);
        
        FILE* dumpfile = fopen(logger.dumpfile_path, "wb");

        fwrite(&logger, sizeof(LogLevel), 1, dumpfile);

        fclose(dumpfile);

        printf("Zakonczono zapis do dumpa\n");
        fflush(stdout);
    }
}