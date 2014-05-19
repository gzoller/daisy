/*
 * daisy.h
 *
 *  Created on: May 15, 2014
 *      Author: greg.zoller
 */

#ifndef DAISY_H_
#define DAISY_H_

#include "hashmap.h"

struct loc {
	char key[256];
	int idx;
	int fwdCount;
	char ipFwd[][128]; // List of string[128];
};
typedef struct loc loc_t;

#define CMD_DB   "db"
#define CMD_CONN "conn"

void setDB( u_char *db, u_char *coll );
void init( char *setName, char **repSet, int numInSet );

void reloadRouteTable();
loc_t *lookup( char *path );
int isStuck();
int errors();

#endif /* DAISY_H_ */
