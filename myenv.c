#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void 
writeid()
{
	pid_t pid;
	uid_t uid;

	pid = getpid();
	uid = getuid();
	printf("UID: %d\n", uid);
	printf("PID: %d\n", pid);
}

void
checkenv(int argc, char *argv[])
{
	int i;
	char *env;

	for (i = 1; i < argc; i++){
		env = getenv(argv[i]);
		if (env == NULL)
			fprintf(stderr, "error: %s does not exist\n", argv[i]);
		else
			printf("%s: %s\n", argv[i], env);
	}
}

int
main (int argc, char *argv[])
{
	writeid();
	checkenv(argc, argv);

	exit (EXIT_SUCCESS);
}