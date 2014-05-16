#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "hashmap.h"

#include <sys/resource.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>

#include <mongoc.h>

#include "daisy.h"

//struct rusage {
//    struct timeval ru_utime; /* user CPU time used */
//    struct timeval ru_stime; /* system CPU time used */
//    long   ru_maxrss;        /* maximum resident set size */
//    long   ru_ixrss;         /* integral shared memory size */
//    long   ru_idrss;         /* integral unshared data size */
//    long   ru_isrss;         /* integral unshared stack size */
//    long   ru_minflt;        /* page reclaims (soft page faults) */
//    long   ru_majflt;        /* page faults (hard page faults) */
//    long   ru_nswap;         /* swaps */
//    long   ru_inblock;       /* block input operations */
//    long   ru_oublock;       /* block output operations */
//    long   ru_msgsnd;        /* IPC messages sent */
//    long   ru_msgrcv;        /* IPC messages received */
//    long   ru_nsignals;      /* signals received */
//    long   ru_nvcsw;         /* voluntary context switches */
//    long   ru_nivcsw;        /* involuntary context switches */
//};
void showMem(char *s) {
	errno = 0;
	struct rusage* memory = malloc(sizeof(struct rusage));
	getrusage(RUSAGE_SELF, memory);
	if(errno == EFAULT)
	    printf("Error: EFAULT\n");
	else if(errno == EINVAL)
	    printf("Error: EINVAL\n");
	printf("%s Max: %ld\n", s, memory->ru_maxrss);
}

struct loc {
	char key[256];
//	char **ipFwd; // List of string //payload[128];
	int idx;
	int fwdCount;
//	char (*ipFwd)[128]; // List of string //payload[128];
	char ipFwd[][128]; // List of string //payload[128];
};

int numTokens( char *s, char sep ) {
	int count = 0;
	char *ptr = s;
	while((ptr = strchr(ptr, sep)) != NULL) {
	    count++;
	    ptr++;
	}
	return count;
}

bool readEntry( FILE *input, map_t map ) {
	char buf[256];

	if(fgets(buf, sizeof buf, input) == NULL)
		return false;
	int count = numTokens(buf,',');

//	struct loc *entry = malloc(sizeof(struct loc));
	struct loc *entry = malloc(sizeof(struct loc) + (count)*128);

	entry->fwdCount = 1; //count;
	strcpy(entry->key, strtok(buf,","));

//	entry->ipFwd = malloc( entry->fwdCount * 128 );
//	entry->ipFwd = malloc( sizeof(char*) * entry->fwdCount );

	for( int i=0; i<entry->fwdCount; i++) {
//		entry->ipFwd[i] = malloc(256);
		strcpy(entry->ipFwd[i], strtok(NULL,","));
	}
	size_t ln = strlen(entry->ipFwd[entry->fwdCount-1]) - 1;
	if (entry->ipFwd[entry->fwdCount-1][ln] == '\n')
		entry->ipFwd[entry->fwdCount-1][ln] = '\0';
	entry->idx = 0;
	hashmap_put( map, entry->key, entry);
	return true;
}


map_t buildMap( char *fname ) {
	FILE *input = fopen(fname, "r");
	map_t map = hashmap_new();
	while(readEntry(input, map));
	fclose(input);
//	showMem("Build");
	return map;
}

/*
 * DB : { "_id":"/foobar", "eps":["ip1","ip2"] }
 */

int main(int argc, char **argv) {

//	mongoc_client_t *client;
//
//	client = mongoc_client_new("mongodb://localhost:27017/daisy");
//	if (!client) {
//      fprintf (stderr, "Failed to parse URI.\n");
//      return EXIT_FAILURE;
//	}
//	mongoc_client_destroy (client);
//	printf("Worked.\n");
//	exit(0);
	char *repSet[] = {"localhost:27017"};

	init("rs0",repSet,1);
//	while(true)
		readRouteTable();
	printf("OK!\n");
	exit(0);

//	showMem("Start");
	printf("My PID: %d\n",getpid());
	map_t m = NULL;
	while( true ) {
		m = buildMap( argv[1] );
//	printf("%d done.\n",hashmap_length(m));

		struct loc *t = NULL;
		if( hashmap_get(m, "Greg", (any_t) &t) == MAP_OK )
			printf("IPs: %s:%s\n", t->ipFwd[0], t->ipFwd[1]);

		hashmap_free_2(m, free);
	}
//	showMem("End");
}
