/*
 * mongo.c
 *
 *  Created on: May 14, 2014
 *      Author: greg.zoller
 */

#include <mongoc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <bson.h>

char *_setName;
char **_repSet;
int  _numInSet;

// mongodb://127.0.0.1:9090,1.2.3.4:80?replicaSet=rs0
mongoc_client_t *_getClient() {
	bson_string_t *str;
	mongoc_client_t *client;

	str = bson_string_new("mongodb://");
	for( int i=0; i<_numInSet; i++ ) {
		if( i > 0 )
			bson_string_append(str,",");
		bson_string_append(str,_repSet[i]);
	}
	if( _numInSet > 1 ) {
		bson_string_append(str,"?replicaSet=");
		bson_string_append(str,_setName);
	}

	// Works
//	client = mongoc_client_new("mongodb://localhost:27017");

	// No-worky
	printf("URL: %s\n",str->str);
	client = mongoc_client_new(str->str);

	bson_string_free(str,true);
	return client;
}

void readRouteTable() {
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;
	mongoc_client_t *client = _getClient();
	mongoc_read_prefs_t *read_prefs;
	bson_t q;
	bson_iter_t iter;
	const bson_t *doc;

	collection = mongoc_client_get_collection(client, "daisy", "table");
	read_prefs = mongoc_read_prefs_new(MONGOC_READ_PRIMARY_PREFERRED);
	bson_init(&q);
	cursor = mongoc_collection_find(collection,
	                                   MONGOC_QUERY_NONE,
	                                   0,
	                                   100000,
	                                   0,
	                                   &q,
	                                   NULL,
	                                   read_prefs);
	while( mongoc_cursor_next(cursor, &doc) ) {
		uint32_t len = 0;
		bson_iter_t child;
		int total = 0;

		bson_iter_init( &iter, doc );

		// Get id (path)
		bson_iter_next( &iter );
		printf("%s  ->  ", bson_iter_utf8(&iter, &len));

		// Now the list of host IPs.
		bson_iter_next( &iter );

		// First time to get count
		bson_iter_recurse( &iter, &child );
		while(bson_iter_next(&child)) total++;
		printf("Total found: %d\n",total);

		bson_iter_recurse( &iter, &child );
		while(bson_iter_next(&child)) {
			printf("\t%s\n", bson_iter_utf8(&child,&len));
		}
		printf("-------------------\n");
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(&q);
	mongoc_read_prefs_destroy(read_prefs);
	mongoc_collection_destroy(collection);
	mongoc_client_destroy(client);
}

void init(char *setName,char **repSet, int numInSet) {
	mongoc_init();
	_setName = setName;
	_repSet = repSet;
	_numInSet = numInSet;
}
