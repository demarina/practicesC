#include <stdio.h>
#include <stdlib.h>

enum{
	MAXARGS = 5,
};

static int
ischardelim(char c)
{
	return ((c == '\n') || (c == '\t') || (c == '\r') || (c == ' '));
}

static int
nextdelim (char *str, int begin)
{
	char c;

	for (;; begin++){
		c = str[begin];
		if (ischardelim(c) || c == '\0')
			break; 
	}

	return begin;
}

static int
nextchar (char *str, int begin)
{
	char c;

	for(;; begin++){
		c = str[begin];
		if (!ischardelim(c))
			break;
	}

	return begin;
}

static int
mytokenize (char *str, char **args, int maxargs)
{
	int newchar;
	int newdelim;
	int tokens;

	newchar = 0;
	tokens = 0;
	newdelim = 0;

	while (tokens < maxargs){
		newchar = nextchar(str, newdelim);
		if(str[newchar] == '\0')
			break;
		args[tokens] = &str[newchar];
		tokens++;
		newdelim = nextdelim(str, newchar);
		if(str[newdelim] == '\0')
				break;
		str[newdelim] = '\0';
		/* I add one to function nextchar don't check this position */
		newdelim++;
	}

	return tokens;
}	

int
main (int argc, char *argv[])
{
	char str[] = "   hola \n que \t    \r que    tal       hala madrid ";
	char *args[MAXARGS];
	int i;
	int j;

	i = 0;

	i = mytokenize(str, args, MAXARGS);
	printf("tokens number: %d \n", i);

	for(j = 0; j < i; j++){
		printf("args[%d]: %s \n", j,args[j]);
	}
	exit(EXIT_SUCCESS);
}















