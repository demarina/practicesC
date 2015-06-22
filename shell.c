#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef struct inforedir inforedir;
struct inforedir{
	int havein;
	char *in;
	int haveout;
	char *out;
	int havebackground;
	pid_t pidwait;
};

char *commands[32][32];
int **pipes;
int numcoms;
inforedir redir;

void
reset()
{
	redir.havein = 0;
	redir.haveout = 0;
	redir.havebackground = 0;
	redir.pidwait = 0;
}

int
tokenizer(char *string, char *delim, char *array[])
{
	int i;
	char *remain;

	for(i = 0;;i++){
		array[i] = strtok_r(string, delim, &remain);
		if(array[i] == NULL)
			break;
		string = remain;
	}

	return i;
}

void
remove_n(char *line)
{
	char *remain;

	strtok_r(line, "\n", &remain);
	line = remain;
}

char
searchpos(int search, char *line)
{
	int i;

	for(i = 0;;i++){
		if(line[i] == '\0')
			break;
		if(line[i] == '<'){
			redir.havein = 1;
			if(--search == 0)
				break;
		}else if(line[i] == '>'){
			redir.haveout = 1;
			if(--search == 0)
				break;
		}
	}

	return line[i];
}

void
doinredir()
{
	int fd;

	fd = open(redir.in, O_RDONLY);
	if (fd < 0)
		fprintf(stderr, "Error to do redirection IN\n");
	dup2(fd, 0);
	close(fd);
}

void
dooutredir()
{
	int fd;

	fd = creat(redir.out, 0744);
	if (fd < 0)
		fprintf(stderr, "Error to do redirection OUT\n");
	dup2(fd, 1);
	close(fd);
}

void
doinredirback()
{
	int fd;

	fd = open("/dev/null", O_RDONLY);
	if (fd < 0)
		fprintf(stderr, "Error to do redirection in background\n");
	dup2(fd, 0);
	close(fd);
}

void
getfileredir(char *line, char sign)
{
	char *file[52];

	tokenizer(line, &sign, file);
	tokenizer(file[1], " ", file);
	if(sign == '<'){
		if(file[0][0] == '$'){
			file[0] = getenv(&file[0][1]);
		}
		redir.in = file[0];
	}else if(sign == '>'){
		if(file[0][0] == '$'){
			file[0] = getenv(&file[0][1]);
		}
		redir.out = file[0];
	}
}

void
redirects(char *line)
{
	char first;
	char second;

	first = searchpos(1, line);
	second = searchpos(2, line);

	if(second != '\0'){
		getfileredir(line, second);
	}
	if(first != '\0'){
		getfileredir(line, first);
	}
}

void
preparedirs(int i)
{
	if(numcoms == 1){
		if(redir.havein)
			doinredir();
		else if(redir.havebackground)
				doinredirback();
		if(redir.haveout)
			dooutredir();
	}else{
		if(i == 0){
			if(redir.havein)
				doinredir();
			else if(redir.havebackground)
				doinredirback();
		}else if(i == (numcoms - 1)){
			if(redir.haveout)
				dooutredir();
		}
	}
}

void
checkenvs()
{
	int i;
	int j;

	j = 0;
	for(i = 0; i < numcoms; i++){
		while(commands[i][j] != NULL){
			if(commands[i][j][0] == '$'){
				commands[i][j] = getenv(&commands[i][j][1]);
				if(commands[i][j] == NULL){
					fprintf(stderr, "Don't exist the enviroment variable\n");
					numcoms = 0;
				}
			}
			j++;
		}		
	}
}

int
checkcreatenvs(char *line)
{
	char *remain[32];
	char *remain2[4];
	char *remain3[4];
	char *line2;

	line2 = malloc(sizeof(strlen(line)));
	strcpy(line2, line);
	if(tokenizer(line2, "=", remain) > 1){
		tokenizer(remain[0], " ", remain2);
		tokenizer(remain[1], " ", remain3);
		setenv(remain2[0], remain3[0], 1);
		printf("1\n");
		free(line2);
		printf("1\n");
		return 1;
	}
	printf("2\n");
	free(line2);
	printf("2\n");
	return 0;
}

void
checkbackground(char *line)
{
	char *line2;
	char *remain[512];
	int tokens;

	line2 = malloc(sizeof(strlen(line)));

	strcpy(line2, line);
	tokens = tokenizer(line2, " ", remain);
	if(strcmp(remain[tokens - 1], "&") == 0){
		tokenizer(line, "&", remain);
		redir.havebackground = 1;
	}
	printf("3\n");
	free(line2);
	printf("3\n");
}

int
docd(char *line)
{
	char *remain[32];
	char *line2;
	int tokens;

	line2 = malloc(sizeof(strlen(line)));
	strcpy(line2, line);
	tokens = tokenizer(line2, " ", remain);

	if( tokens > 0 && strcmp(remain[0], "cd") == 0){
		printf("tio\n");
		if(remain[1] == NULL){
			chdir(getenv("HOME"));
		}else{
			if(remain[1][0] == '$'){
				remain[1] = getenv(&remain[1][1]);
			}
			if(chdir(remain[1]) < 0)
				fprintf(stderr, "%s: No exist directory\n", remain[1]);
		}
		printf("4\n");
		free(line2);
		printf("4\n");
		return 1;
	}
	printf("5\n");
	free(line2);
	printf("5\n");
	return 0;
}

int
tokenlinep(char *line)
{	
	char *array[32];
	int i;
	int args;

	numcoms = tokenizer(line, "|", array);
	for(i = 0; i < numcoms; i++){
		args = tokenizer(array[i], " \t", commands[i]);
		commands[i][args] = NULL;
	}

	return numcoms;
}

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
		printf("6\n");
		free(pipes[i]);
		printf("6\n");
	}
	if(numcoms > 1){
		printf("7\n");
		free(pipes);
		printf("7\n");
	}
}

void
preparepipe(int i)
{
	if(i == 0){
		dup2(pipes[i][1], 1);
	}
	else if((i > 0) && (i < (numcoms - 1))){
		dup2(pipes[i-1][0], 0); 
		dup2(pipes[i][1], 1);
	}else{
		dup2(pipes[i-1][0], 0);
	}
	closepipes();
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
path2exec(int i)
{
	char *path;
	char *envpath;
	char *arraypath[512];
	int j;
	char *aux;

	path = createpath(".", commands[i][0]);
	if(access(path, X_OK) == 0)
		return path;
	printf("8\n");
	free(path);
	printf("8\n");
	envpath = getenv("PATH");

 	aux = strdup(envpath);
	tokenizer(aux, ":", arraypath);
 
	for(j = 0;;j++){
		if(arraypath[j] == NULL)
			break;
		path = createpath(arraypath[j], commands[i][0]);
		if(access(path, X_OK) == 0){
			printf("9\n");
			free(aux);
			printf("9\n");
			return path;
		}
		printf("10\n");
		free(path);
		printf("10\n");
	}
	printf("11\n");
	free(aux);
	printf("11\n");
	return NULL;
}

void
execute(int i)
{
	pid_t pid;
	char *path;

	pid = fork();
	if(pid == 0){
		if(numcoms > 1)
			preparepipe(i);
		preparedirs(i);
		path = path2exec(i);
		if(path != NULL)
			execv(path, commands[i]);
		printf("12\n");
		free(path);
		printf("12\n");
		fprintf(stderr, "%s: can't execute command\n", commands[i][0]);	
		exit(EXIT_FAILURE);
	}
	if(i == numcoms -1)
		redir.pidwait = pid;
	if(pid < 0){
		fprintf(stderr, "Error to execute command 'FORK'");
	}
}

void
executecommands()
{
	int i;
	pid_t pid;

	for (i = 0; i < numcoms; i++)
		execute(i);
	closepipes();
	if(!redir.havebackground){
		for(;;){
			pid = wait(NULL);
			if(pid == redir.pidwait)
				break;
		}
	}
}

int
main(int argc, char *argv[])
{
	char line[512];

	for(;;){
		reset();
		if(ttyname(0))
			printf("AlbertoGarcia$ ");
		if(fgets(line, 512, stdin) == NULL)
			break;
		if (strlen(line) == 1)
			continue;
		remove_n(line);
		checkbackground(line);
		if(docd(line))
			continue;
		redirects(line);
		if(checkcreatenvs(line) == 1)
			continue;
		tokenlinep(line);
		checkenvs();
		if(numcoms > 1)
			createpipes();
		executecommands();
	}
	
	exit(EXIT_SUCCESS);
}