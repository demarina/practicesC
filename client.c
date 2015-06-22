#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <sys/uio.h>
#include <strings.h>

int
openfifo(char *dir)
{
	int fd;

	fd = open(dir, O_WRONLY);
	if (fd < 0)
		err(1, "Error to open FIFO '%s'", dir);

	return fd;
}

void
writefifo(int fd)
{
	char line[512];

	for(;;){
		fgets(line, 512, stdin);
		if(write(fd, line, strlen(line)) != strlen(line))
			err(1, "Error to write FIFO");
		if(line == NULL)
			break;
	}
}

int
main(int argc, char *argv[])
{
	int fd;
	char *dirfifo;

	dirfifo = "/tmp/logger";
	fd = openfifo(dirfifo);
	writefifo(fd);

	exit(EXIT_SUCCESS);
}