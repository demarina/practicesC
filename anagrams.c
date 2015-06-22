#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Anag Anag; 
struct Anag{
	char *str;	
	Anag *next;
};	

static Anag *
createanag(char *str)
{
	Anag *new;

	new = malloc(sizeof(Anag));
	new->str = str;
	new->next = NULL;

	return new;
}

static int
areanagrams(char *str1, char *str2)
{
	int len;
	int i;
	int are;
	int j;
	char *cpy;

	cpy = malloc (strlen(str2)*sizeof(char));	
	strcpy (cpy, str2);
	j = 0;
	are = 0;


	if (strlen(str1) == strlen(str2)){
		len = strlen(str1);
		for (i = 0; i < len; i++)
		{
			are = 0;
			for (j = 0; j < len; j++){
				if(cpy[j] !=  ' '){
					if (str1[i] == cpy[j]){
						cpy[j] = ' ';
						are = 1;
						break;
					}
				}	
			}
			if (are == 0)
				break;
		}
	}

	return are;
}

static void
insert(Anag **store, char *argv)
{
	Anag *new;
	new = createanag(argv);
	*store = new;
}

static void
add(Anag **store, char *str)
{
	Anag *new;
	Anag *Paux;

	Paux = *store;
	new = createanag(str);

	for(;;){
		if (Paux->next == NULL){
			Paux->next = new;
			break;
		}
		Paux = Paux->next;
	}
}

static void
fillstore(Anag **store, char *argv)
{
	int i;

	for(i = 0;; i++){
		if(store[i] == NULL){
			insert(&store[i], argv);
			break;
		}else{
			if(areanagrams(store[i]->str, argv)){
				add(&store[i], argv);
				break;
			}
		}
	}
}

static int
sameletters(Anag *store, int i)
{
	Anag *Paux;

	Paux = store;
	while(Paux->next != NULL){
		if(Paux->str[i] != Paux->next->str[i])
			return 0;
		Paux = Paux->next;
	}

	return 1;
}

static void
printletters(Anag *store)
{
	int len;
	int i;

	len = strlen(store->str);

	for(i = 0; i < len; i++){
		if (sameletters(store, i))
			printf("%c", store->str[i]);
	}	
}

static void
printstore(Anag *store)
{
	Anag *Paux;

	Paux = store;
	if (Paux->next != NULL){
		for(;;){
			if(Paux != NULL){
				printf("%s ", Paux->str);
				Paux = Paux->next;
			}
			else{
				printf(" [");
				printletters(store);
				printf("]");
				printf("\n");
				break;
			}
		}
	}
}

int 
main(int argc, char *argv[])
{
	int i;
	Anag **store;

	store = malloc(argc * sizeof(Anag *));
	memset(store, 0, argc * sizeof(Anag *));

	for (i = 1; i < argc; i++)
	{
		fillstore(store, argv[i]);
	}	
	for(i = 0; i < argc; i++){
		if(store[i] != NULL){
			printstore(store[i]);
		}else{
			break;
		}
	}

	exit (EXIT_SUCCESS);
}