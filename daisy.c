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

#include "daisy.h"

char *_setName;
char **_repSet;
int  _numInSet;
mongoc_collection_t *collection;
mongoc_client_t *client = NULL;
int errorCount = 0;
bool stuck = false;

loc_t *populateEntry( bson_iter_t iter ) {
	uint32_t len = 0;
	bson_iter_t child;
	int total = 0;
	char buf[256];
	loc_t *entry;

	// Get id (path)
	bson_iter_next( &iter );
	strcpy( buf, bson_iter_utf8(&iter, &len) );

	// Now the list of host IPs.
	bson_iter_next( &iter );

	// First time to get count
	bson_iter_recurse( &iter, &child );
	while(bson_iter_next(&child)) total++;

	// Create the entry object and populate
	entry = malloc(sizeof(loc_t) + (total)*128);
	strcpy( entry->key, buf );
	entry->fwdCount = total;

	bson_iter_recurse( &iter, &child );
	int i = 0;
	while(bson_iter_next(&child)) {
		strcpy( entry->ipFwd[i++], bson_iter_utf8(&child,&len));
	}
	entry->idx = 0;
	return entry;
}

// mongodb://127.0.0.1:9090,1.2.3.4:80?replicaSet=rs0
mongoc_client_t *_getClient() {
	bson_string_t *str;

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

	client = mongoc_client_new(str->str);

	bson_string_free(str,true);
	return client;
}

void _refreshConn() {
	if( client != NULL ) {
		mongoc_collection_destroy(collection);
		mongoc_client_destroy(client);
	}
	client = _getClient();
	collection = mongoc_client_get_collection(client, "daisy", "table");
}

map_t readRouteTable(int attemptNo ) {
	mongoc_cursor_t *cursor;
	mongoc_read_prefs_t *read_prefs;
	bson_t q;
	bson_iter_t iter;
	const bson_t *doc;
	loc_t *entry;
	map_t routeTable;
	bson_error_t error;
	bool err = false;

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
	mongoc_cursor_next(cursor, &doc);
	err = mongoc_cursor_error(cursor, &error);
	if( !err ) {
		routeTable = hashmap_new();
		do {
			bson_iter_init( &iter, doc );
			entry = populateEntry( iter );
			hashmap_put( routeTable, entry->key, entry);
		} while( mongoc_cursor_next(cursor, &doc) );
	}
	else {
		errorCount++;
		if( attemptNo == 7 ) {  // repeatedly try up to 7 times
			stuck = true;
			return NULL;  // Abandon ship!  We're stuck in a loop of fail.
		}
		client = NULL;
		_refreshConn();  // try to re-establish db connectivity
		return readRouteTable(attemptNo+1);
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(&q);
	mongoc_read_prefs_destroy(read_prefs);

	return routeTable;
}

void init(char *setName,char **repSet, int numInSet) {
	mongoc_init();
	_setName = setName;
	_repSet = repSet;
	_numInSet = numInSet;
	_refreshConn();
}
