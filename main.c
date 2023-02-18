#include "logger.h"

struct test{
	int pzdr;
	int siema;
};

int main(void) {
	printf("PID %d\n", getpid());

	struct test t;

	logger_init(STANDARD, "test", "test.txt");
	log_msg(STANDARD, "test");
	log_msg(MAX, "test2");
	sleep(5);
	printf("logger close");
	fflush(stdout);
	logger_close();

	return 0;
}