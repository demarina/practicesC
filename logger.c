#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>

void
createfifo(char *dir)
{
	if(access(dir, F_OK) == 0){
		if(unlink(dir) < 0)
			err(1, "Error unlink");
	}


	if(mkfifo(dir, 0664) < 0)
		err(1, "Error to mkfifo");
}

void
remove_n(char *line)
{
	char *remain;

	strtok_r(line, "\n", &remain);
	line = remain;
}

void
printtime()
{
	time_t times;
	char *clok;

	time(&times);
	clok = ctime(&times);
	remove_n(clok);

	fprintf(stderr, "%s: ", clok);
}

void
handler(int i)
{
	printtime();
	printf("time out, no events\n");
	alarm(5);
}

void
executefifo(char *dir)
{
	int fd;
	int br;
	char buffer[1024];

	signal(SIGALRM, handler);
	for(;;){
		printtime();
		fprintf(stderr, "waiting for client\n");
		fd = open(dir, O_RDONLY);
		if(fd < 0)
			err(1, "Error to open FIFO '%s'", dir);
		printtime();
		fprintf(stderr, "ready to read events\n");
		for(;;){
			alarm(5);
			br = read(fd, buffer, sizeof(buffer) - 1);
			if(br < 0)
				err(1, "Error to read");
			if(br == 0)
				break;
			buffer[br] = '\0';
			printtime();
			fprintf(stderr, "%s", buffer);
		}
		alarm(0);
		close(fd);
	}
}

int
main(int argc, char *argv[])
{
	char *dirfifo;

	dirfifo = "/tmp/logger";
	createfifo(dirfifo);
	executefifo(dirfifo);

	exit(EXIT_SUCCESS);
}