#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <string.h>
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

char *arpath[] = {"/bin", "/usr/bin", "usr/local/bin"};

int
isFile(char *path)
{
	struct stat buf;
	if(stat(path, &buf) < 0)
		return -1;

	return (buf.st_mode & S_IFMT) == S_IFREG;
}

char *
createpath(char *str, char *str2)
{
	char *buffer;
	int lenbuf;

	lenbuf = (strlen(str) + strlen(str2) + 2);
	buffer = malloc(lenbuf);
	snprintf(buffer, lenbuf, "%s/%s", str, str2);
	
	return buffer;
}

int
isTxt(char *file)
{
	char *str;

	str = strrchr(file,'.');

	if(str != NULL && (strcmp(".txt", str)) == 0)
		return 1;

	return 0;
}

int
prepare(char *file)
{
	int fd;

	fd = open(file, O_RDONLY);
	if(fd < 0){
		fprintf(stderr, "Error to open %s\n", file);
		return -1;
	}
	dup2(fd, 0);
	close(fd);

	return 1;
}

int
canexecute(char* com)
{
	int i;
	char *pathcom;

	for(i = 0;;i++){
		if(arpath[i] == NULL)
			return -1;
		pathcom = createpath(arpath[i], com);
		if(access(pathcom, X_OK) == 0)
			return i;
	}
}

void
execute(char *file, char *argv[], int fd)
{
	pid_t pid;
	int status;
	int numpath;
	char *pathexec;

	pid = fork();
	if(pid == 0){
		if(prepare(file) == -1)
			return;
		numpath = canexecute(argv[1]);
		if(numpath != -1){
			dup2(fd, 1);
			close(fd);
			argv++;
			pathexec = createpath(arpath[numpath], argv[0]);
			execv(pathexec, argv);
		}
		exit(EXIT_FAILURE);
	}
	if(pid < 0){
		fprintf(stderr, "Fail to do FORK");
		exit(EXIT_FAILURE);
	}
	wait(&status);
}

void
findfiles(char *path, char* argv[], int fd)
{
	DIR *dirp;
	struct dirent* dp;

	dirp = opendir(path);
	if(dirp == NULL)
		return;
	while((dp = readdir(dirp)) != NULL){
		if (isFile(dp->d_name)){
			if(isTxt(dp->d_name)){
				execute(dp->d_name, argv, fd);
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	int fd;

	fd = creat("apply.output", 0774);
	findfiles(".", argv, fd);
	close(fd);
	exit(EXIT_SUCCESS);
}