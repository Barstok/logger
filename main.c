#include "logger.h"

struct test{
	int pzdr;
	int siema;
} global;

Dump dumpfun(void){
	Dump dump = {.dump = &global, .dump_size = sizeof(struct test)};

	return dump;
};

int main(void) {
	printf("PID %d\n", getpid());

	struct test t = {.pzdr=1, .siema=8};


	logger_init(STANDARD, "testt.txt", "build", dumpfun);
	log_msg(STANDARD, "test");
	log_msg(MAX, "test2");
	

	sleep(60);

	logger_close();

	return 0;
}