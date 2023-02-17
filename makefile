logger: main.c logger.c logger.h
	gcc -o build/logger main.c logger.c -pthread -I.
