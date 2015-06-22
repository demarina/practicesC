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

int
isDir(char *path)
{
	struct stat buf;
	if(stat(path, &buf) < 0)
		return -1;

	return buf.st_mode & S_IFDIR;
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
contains(char *path, char *str)
{
	int fd;
	int br;
	int bt;
	char buffer[512];
	char buffer2[512];
	char *str2;

	bt = 0;
	str2 = malloc(strlen(str)+1);
	memset(str2, 0, strlen(str)+1);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	while(bt < strlen(str)){
		br = read(fd, buffer, strlen(str));
		if(br <= 0)
			break;
		memcpy(buffer2+bt, buffer, br);
		bt = bt + br;
		if(bt < strlen(str))
			continue;
		memcpy(str2, buffer2, strlen(str));
		if (strcmp(str2, str) == 0){
			free(str2);
			close(fd);
			return 1;
		}
	}
	free(str2);
	close(fd);

	return 0;
}

void
travedir(char *path, char *str)
{
	DIR *dirp;
	struct dirent* dp;
	char *newpath;

	dirp = opendir(path);
	if(dirp == NULL)
		return;
	while((dp = readdir(dirp)) != NULL){
		if(!isValiddir(dp->d_name))
			continue;
		newpath = createpath(path, dp->d_name);
		if(isDir(newpath) == -1)
			continue;
		if (isDir(newpath)){
			travedir(newpath, str);
		}else{
			if(contains(newpath, str))
				printf("%s \n", newpath);
		}
		free(newpath);
	}
}

int
main(int argc, char *argv[])
{
	int i;

	if(argc == 2){
		travedir(".", argv[1]);
	}else{
		for(i = 2; i < argc; i++)
			travedir(argv[i], argv[1]);
	}

	exit(EXIT_SUCCESS);
}