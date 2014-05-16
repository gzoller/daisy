#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
//#include "hashmap.h"

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

/*
 * DB : { "_id":"/foobar", "hosts":["ip1","ip2"] }
 */

int main(int argc, char **argv) {

	char *repSet[] = {"localhost:27017"};
	map_t current = NULL;
	map_t prev = NULL;
	int ret;

	init("rs0",repSet,1);
	while(true) {
		loc_t *t = NULL;
		prev = current;
		current = readRouteTable(0);
		if( current == NULL ) {
			printf("Stay w/old...\n");
			current = prev;
		}
		else if( prev != NULL )
			hashmap_free_2(prev, free);
		ret = hashmap_get(current, "greg", (any_t) &t);
		if( ret == MAP_OK ) {
			printf("IPs: %d ",t->fwdCount);
			for( int x=0; x<t->fwdCount; x++ )
				printf("%s ", t->ipFwd[x]);
			printf("\n");
		}
		else {
			printf("Boom! %d\n",ret);
			exit(0);
		}
	}
}
