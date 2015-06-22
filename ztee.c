#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>

void
checkargs(int argc)
{
	if (argc != 2){
		err(1, "Arguments fail, must be: Program File");
	}
}

void
unzip(int pi[])
{
	pid_t pid;

	pid = fork();
	if (pid == 0){
		dup2(pi[1], 1);
		close(pi[1]);
		close(pi[0]);
		execl("/usr/bin/gunzip", "gunzip", NULL);
		err(1,"Fail to execute command");
	}
	if (pid < 0)
		err(1, "FORK error");
}

void
writecon(char *buffer, int nr, int fd)
{
	if(write(1, buffer, nr) != nr)
		err(1, "Error to write");
	if(write(fd, buffer, nr) != nr)
		err(1, "Error to write");
}


void
readpipe(int pi[], int fd)
{
	char buffer[1024];
	int nr;

	nr = 1;
	close(pi[1]);
	while(nr != 0){
		nr = read(pi[0], buffer, sizeof(buffer));
		if(nr < 0)
			err(1, "Error to read");
		writecon(buffer, nr, fd);
	}
}

void
execute(char *file, int pi[])
{
	int fd;

	unzip(pi);
	fd = creat(file, 0744);
	readpipe(pi, fd);
	close(fd);
}

int
main (int argc, char *argv[])
{
	int pi[2];

	checkargs(argc);
	pipe(pi);
	execute(argv[1], pi);
	close(pi[0]);
	wait(NULL);

	exit(EXIT_SUCCESS);
}