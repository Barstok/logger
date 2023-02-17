#include "logger.h"

struct test{
	int pzdr;
	int siema;
};

int main(void) {
	printf("PID %d\n", getpid());

	struct test t;

	logger_init(STANDARD, "test", "test");
	log_msg(STANDARD, "test");
	logger_close();

	return 0;
}