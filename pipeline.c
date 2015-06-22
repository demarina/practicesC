#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

typedef struct info info;
struct info{
	char *coms[32];
	char *in;
	char *out;
};

char *arpath[] = {"/bin", "/usr/bin", "usr/local/bin"};
int **pipes;
int numcoms;
info data;

void
createpipes()
{
	int i;

	pipes = malloc(sizeof(int*) * (numcoms - 1));
	for(i = 0; i < (numcoms - 1); i++){
		pipes[i] = malloc(sizeof(int) * 2);
		pipe(pipes[i]);
	}
}

void
closepipes()
{
	int i;

	for(i = 0; i < numcoms - 1; i++){
		close(pipes[i][0]);
		close(pipes[i][1]);
	}
}

void
preparepipes(int i)
{
	if(i == 0){
		dup2(pipes[i][1], 1);
	}
	else if(i > 0 && (i < numcoms - 1)){
		dup2(pipes[i-1][0], 0);
		dup2(pipes[i][1], 1);
	}else if(i == (numcoms - 1)){	
		dup2(pipes[i-1][0], 0);
	}

	closepipes();			
}

void
insertcom(char *com)
{
	int i;

	i = 0;
	while(data.coms[i] != NULL)
		i++;
	data.coms[i] = com;
}

void
filldata(int argc, char *argv[])
{
	int i;

	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "-i") == 0)
			data.in = argv[++i];		
		else if(strcmp(argv[i], "-o") == 0)
			data.out = argv[++i];
		else{
			insertcom(argv[i]);
			numcoms++;
		}
	}
}

void
dodups()
{
	int fd;

	if(data.out != NULL){
		fd = creat(data.out, 0744);
		if (fd < 0)
			err(1, "Error to create file");
		dup2(fd, 1);
		close(fd);
	}
	if(data.in != NULL){
		fd = open(data.in, O_RDONLY);
		if (fd < 0)
			err(1, "Error to open file");
		dup2(fd, 0);
		close(fd);
	}
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

char *
path2exec(int com)
{
	int i;
	char *pathcom;

	for(i = 0;;i++){
		if(arpath[i] == NULL)
			return NULL;
		pathcom = createpath(arpath[i], data.coms[com]);
		if(access(pathcom, X_OK) == 0)
			return pathcom;
		free(pathcom);
	}
}

void
executecom(int i)
{
	pid_t pid;
	char *path;

	pid = fork();
	if(pid == 0){
		if(numcoms > 1)
			preparepipes(i);
		path = path2exec(i);
		if(path != NULL){
			execl(path, data.coms[i], NULL);
		}
		err(1, "Can't execute command '%s'", data.coms[i]);
	}
	if(pid < 0)
		err(1, "Error to do fork");
}

void
execute()
{
	int i;

	if(numcoms > 1)
		createpipes();
	for(i = 0; i < numcoms; i++)
		executecom(i);
	closepipes();
	for(i = 0; i < numcoms; i++)
		wait(NULL);
}

int
main(int argc, char *argv[])
{
	filldata(argc, argv);
	dodups();
	execute();

	exit(EXIT_SUCCESS);
}



















