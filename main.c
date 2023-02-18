#include "logger.h"

struct test{
	int pzdr;
	int siema;
};

int main(void) {
	printf("PID %d\n", getpid());

	struct test t;

	logger_init(STANDARD, "build", "testt.txt");
	log_msg(STANDARD, "test");
	log_msg(MAX, "test2");
	sleep(60);
	logger_close();

	return 0;
}