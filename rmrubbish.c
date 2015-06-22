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

typedef enum Condition Condition;
enum Condition{
	yesremov,
	noremov,
	failopen
};

Condition summary = noremov;

int
isDir(char *path)
{
	struct stat buf;
	if(stat(path, &buf) < 0)
		return -1;

	return (buf.st_mode & S_IFMT) == S_IFDIR;
}

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
isValiddir(char *str)
{
	return !(strcmp(str, ".") == 0 || strcmp(str, "..") == 0);
}

int
mustremoved(char *name)
{
	char *str;

	str = strrchr(name,'.');

	if(str != NULL && (strcmp(".rubbish", str)) == 0)
		return 1;

	return 0;
}

void
travedir(char *path)
{
	DIR *dirp;
	struct dirent* dp;
	char *newpath;

	dirp = opendir(path);
	if(dirp == NULL){
		summary = failopen;
		return;
	}
	while((dp = readdir(dirp)) != NULL){
		if(!isValiddir(dp->d_name))
			continue;
		newpath = createpath(path, dp->d_name);
		if(isDir(newpath) == -1)
			continue;
		if (isDir(newpath)){
			travedir(newpath);
		}else{
			if(isFile(newpath)){
				if (mustremoved(dp->d_name)){
					summary = yesremov;
					if (unlink(newpath) < 0){
						printf("error removing %s\n", newpath);
						exit(EXIT_FAILURE);
					}
				}
			}
		}
		free(newpath);
	}
}

void
delegate(char *path)
{
	pid_t pid;

	pid = fork();
	if(pid == 0){
		pid = getpid();
		printf("%d: ", pid);
		travedir(path);
		switch(summary){
			case failopen:
				printf("fail to open in dir %s\n", path);
				break;
			case noremov:
				printf("no files to remove in dir %s\n", path);
				break;
			case yesremov:
				printf("%s ok\n", path);
				exit(EXIT_SUCCESS);
		}
		exit(EXIT_FAILURE);
	}
	if(pid < 0){
		fprintf(stderr, "Fail to do fork in dir %s\n", path);
		exit(EXIT_FAILURE);
	}
}

void
waiter(int argc)
{
	int i;
	int status;
	int fails;

	fails = 0;
	for(i = 1; i < argc; i++){
		wait(&status);
		if(status != 0)
			fails++;
	}
	if(!fails){
		printf("all processes were successful\n");
	}else{
		printf("%d processes failed\n", fails);
	}
}

int
main(int argc, char *argv[])
{
	int i;

	for(i = 1; i < argc; i++){
		delegate(argv[i]);
	}
	waiter(argc);
	exit(EXIT_SUCCESS);
}