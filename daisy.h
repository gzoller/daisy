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

void init( char *setName, char **repSet, int numInSet );
void reloadRouteTable();
loc_t *lookup( char *path );
bool isStuck();
int errors();

#endif /* DAISY_H_ */
