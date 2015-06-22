#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

enum{
	rstand,
	rfile,
	wloop,
	error,
};

typedef struct Coor Coor;
struct Coor{
	int x;
	int y;
	Coor *next;
};

typedef struct BaseCoor BaseCoor;
struct BaseCoor{
	int x;
	int y;
};

Coor *list;

static void
insert(Coor *co)
{
	co->next = list;
	list = co;
}

static Coor*
createCoor(BaseCoor co)
{
	Coor *new;

	new = malloc(sizeof(Coor));
	new->x = co.x;
	new->y = co.y;
	new->next = NULL;

	return new;
}

static void
writecoor(int fd, BaseCoor coor)
{
	if(write(fd, &coor, sizeof(BaseCoor)) != sizeof(BaseCoor))
		err(1, "Error to write");
}

static void
writeloopcoor(int N)
{
	int i;
	BaseCoor aux;

	for(i = 0; i <= N;i++){
		aux.x = i;
		aux.y = i;
		writecoor(1, aux);
	}
}

static int
checkargs(int argc, char *argv[])
{
	if(argc == 1)
		return rstand;
	if(argc == 2)
		return rfile;
	if(argc == 3){
		if(strcmp(argv[1],"-w") == 0)
			return wloop;
	}

	return error;
}

static void
addCoors(int number, BaseCoor *buf)
{
	Coor *new;
	int i;

	for(i = 0; i < number; i++){
		new = createCoor(buf[i]);
		insert(new);
	}
}

static int
calculexc(char *buf, int nr, int other)
{
	int excess;
	excess = (nr - other) % sizeof(BaseCoor);

	return excess;
}

static void
readcoors(int fd)
{
	char buf[1024];
	char buf2[1024];
	BaseCoor *bufCoor;
	char byte[sizeof(BaseCoor)];
	int excess;
	int other;
	int nr;
	int numCoor;

	memset(byte, 0, sizeof(BaseCoor));
	excess = 0;
	other = 0;
	nr = 1;

	while(nr != 0){
		other = 0;
		nr = read(fd, buf, 2);
		if(nr < 0)
			err(1, "Error to read");
		if(excess){
			if((nr + excess) >= sizeof(BaseCoor)){
				other = sizeof(BaseCoor) - excess;
				memcpy(byte+excess, buf, other);
				bufCoor = (BaseCoor *) byte;
				addCoors(1, bufCoor);
			}else{
				memcpy(byte+excess, buf, nr);
				excess = excess + nr;
				continue;
			}
		}
		excess = calculexc(buf, nr, other);
		if(excess)
			memcpy(byte, buf+nr-excess, excess);
		memcpy(buf2, buf+other, nr-excess-other);
		bufCoor = (BaseCoor *) buf2;
		numCoor = (nr - other - excess) / sizeof(BaseCoor);
		addCoors(numCoor, bufCoor);
	}
}

static void
readfile(char *argv)
{
	int fd;

	fd = open(argv, O_RDONLY);
		if(fd < 0)
			err(1, "Error to open: '%s'", argv);
	readcoors(fd);
	close(fd);
}

static void
printlist()
{
	Coor *Paux;
	Coor *Paux2;

	Paux = list;
	while(Paux != NULL){
		printf("(%d,%d)\n",Paux->x, Paux->y);
		Paux2 = Paux;
		Paux = Paux->next;
		free(Paux2);
	}
}

int
main(int argc, char *argv[])
{
	int com;
	com = checkargs(argc, argv);

	switch(com)
	{
		case rstand:
			readcoors(0);
			printlist();
			break;
		case rfile:
			readfile(argv[1]);
			printlist();
			break;
		case wloop:
			writeloopcoor(atoi(argv[2]));
			break;
		default:
			fprintf(stderr, "Arguments fail\n");
			exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}