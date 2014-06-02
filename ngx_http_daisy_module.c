/*
 * Copyright (C) Evan Miller
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_http_proxy_module.h>
#include <string.h>
#include "daisy.h"

#define RELOAD_PATH "/daisy/test"

	static char* ngx_http_daisy(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

	extern ngx_module_t  ngx_http_proxy_module;

	extern ngx_module_t  *ngx_modules[];

	static ngx_command_t  ngx_http_daisy_commands[] = {
	    { ngx_string("daisy"),
	      NGX_HTTP_LOC_CONF|NGX_CONF_1MORE, 
	      ngx_http_daisy,
	      NGX_HTTP_LOC_CONF_OFFSET,
	      0,
	      NULL },

	      ngx_null_command
	};

	//-----------------------------------------
	static ngx_http_module_t  ngx_http_daisy_module_ctx = {
	    NULL,  /* preconfiguration */
	    NULL,  /* postconfiguration */

	    NULL,  /* create main configuration */
	    NULL,  /* init main configuration */

	    NULL,  /* create server configuration */
	    NULL,  /* merge server configuration */

	    NULL,  /* create location configuration */
	    NULL
	};

	ngx_module_t  ngx_http_daisy_module = {
	    NGX_MODULE_V1,
	    &ngx_http_daisy_module_ctx,    /* module context */
	    ngx_http_daisy_commands,       /* module directives */
	    NGX_HTTP_MODULE,               /* module type */
	    NULL,                          /* init master */
	    NULL,                          /* init module */
	    NULL,                          /* init process */
	    NULL,                          /* init thread */
	    NULL,                          /* exit thread */
	    NULL,                          /* exit process */
	    NULL,                          /* exit master */
	    NGX_MODULE_V1_PADDING
	};

 	int
	getDynamicURL(ngx_str_t uri, int *len, unsigned char *urlBuf, int incPtr){
		loc_t *loc = NULL;
		int port = 80;

		int scan = 0;
		int count = 0;
		while( count < 3 && scan < (int) uri.len ) {
			if(uri.data[scan] == '/') count++;
			scan++;
		}
		if( count == 3 ) {
			scan--;
			uri.data[scan] = '\0';
			loc = lookup( (char*) uri.data );
			
			// Reload table?
			if( !strcmp( (char*)uri.data, RELOAD_PATH ) )
				reloadRouteTable();

			uri.data[scan] = '/';
			if( loc != NULL ) {
				*len = 0;
				while( loc->ipFwd[loc->idx][*len] != '\0' && loc->ipFwd[loc->idx][*len] != ':' ) {
					urlBuf[*len] = loc->ipFwd[loc->idx][*len];
					(*len)++;
				}
				if( loc->ipFwd[loc->idx][*len] == ':' )
					port = atoi( &loc->ipFwd[loc->idx][*len+1] );
	
				// rotate the pointer
				if( incPtr ) {
					loc->idx += 1;
					if( loc->idx == loc->fwdCount ) loc->idx = 0;
				}
			}
		}
		return port;
	}

	static char *
	ngx_http_daisy(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
	{
	    ngx_str_t *value;
	    value = cf->args->elts;

	    if( !ngx_strcmp(value[1].data, CMD_DB) ) {
		// daisy db <db_name> <db_collection>
		setDB( value[2].data, value[3].data );
	    }
	    else if( !ngx_strcmp(value[1].data, CMD_CONN) ) {
		// daisy conn <replica_set_name> <db_host_ip_and_port> ... <db_host_ip_and_port>
		// or
		// daisy conn <db_host_ip_and_port>
		if( cf->args->nelts == 3 ) { // non-replicated mode
			init( "", (char**)&value[2].data, 1 );
		} else {
			int numHosts = cf->args->nelts - 3;
			char **hosts;
			int i = 0;
			hosts = malloc( 256 * numHosts );  // never freed for life of program
			for( i=0; i<numHosts; i++ )
				strcpy( hosts[i], (char*)value[i+3].data );
			init( (char*)value[2].data, hosts, numHosts );
		}
	    }

		// Find proxy module and locate the loc_conf object.  Then insert getDynamicURL function pointer into it.
		ngx_http_proxy_loc_conf_t *plcf;
		plcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_proxy_module);
		plcf->getDynamicURL = getDynamicURL;

	    return NGX_CONF_OK;
	}
