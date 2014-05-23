#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "daisy.h"

/*
 * DB : { "_id":"/foobar", "hosts":["ip1","ip2"] }
 */

int main(int argc, char **argv) {

	char *repSet[] = {"localhost:27017"};

	init("rs0",repSet,1);

	int i = 0;

	while(true) {
		reloadRouteTable();

		loc_t *t = lookup("http://excitinglives.com/sapien/urna/pretium.html");
		if( t != NULL ) {
			for( int z=0; z<i; z++ )
				printf(" ");
			i++;
			if( i > 5 ) i = 0;
			printf("IPs: %d ",t->fwdCount);
			for( int x=0; x<t->fwdCount; x++ )
				printf("%s ", t->ipFwd[x]);
			printf("\n");
		}
		else {
			printf("Boom!\n");
			exit(0);
		}
	}
}
