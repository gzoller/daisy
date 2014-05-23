/*
 * Copyright (C) Evan Miller
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_http_proxy_module.h>
#include <string.h>
#include "daisy.h"

	static char* ngx_http_daisy(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

	extern ngx_module_t  ngx_http_proxy_module;

	//static char* ngx_http_daisy_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

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
//	    ngx_http_daisy_merge_loc_conf   /* merge location configuration */
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

/*
	static ngx_int_t
	ngx_http_daisy_handler(ngx_http_request_t *r)
	{
		ngx_buf_t *b;
		ngx_chain_t out;

		char uriKey[256] = "";
		if( r->uri.len == 0 ) {
		    strcat(uriKey,"/");
		} else {
		    unsigned int len = 1;
		    while( len != 0 && len < r->uri.len && len < 255 && r->uri.data[len] != '/' ) len++;
		    strncat( uriKey, (const char*) r->uri.data, len);
		}

		char c[255] = "OK ";
		strcat( c, uriKey );

		r->headers_out.status = NGX_HTTP_OK;
		r->headers_out.content_length_n = strlen(c);
		r->headers_out.content_type.len = sizeof("text/html") - 1;
		r->headers_out.content_type.data = (u_char*) "text/html";
		ngx_http_send_header(r);

		b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
		if( b == NULL ) {
		    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
        	    return NGX_HTTP_INTERNAL_SERVER_ERROR;
		}
		b->pos = (unsigned char*)c;
		b->last = (unsigned char*)c + strlen(c);
		b->memory = 1;
		b->last_buf = 1;
		out.buf = b;
		out.next = NULL;
		return ngx_http_output_filter(r,&out);
	}
*/

 	void
	getDynamicURL(ngx_str_t uri, int *len, unsigned char *urlBuf){
		strcpy((char*)urlBuf,(const char*)"www.ford.com");
		*len = 12;
	}

	static char *
	ngx_http_daisy(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
	{
	    ngx_str_t *value;
	    value = cf->args->elts;

	    //ngx_http_core_loc_conf_t *clcf;
	    //clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	    //clcf->handler = ngx_http_daisy_handler;

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

ngx_http_proxy_loc_conf_t *plcf;
plcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_proxy_module);
plcf->getDynamicURL = getDynamicURL;

/*
int i=0;
for (i = 0; ngx_modules[i]; i++) {
if( ngx_modules[i]->type == NGX_HTTP_MODULE )
printf("Module_ %ld %ld v%ld type %ld\n",ngx_modules[i]->index, ngx_modules[i]->ctx_index,ngx_modules[i]->version,ngx_modules[i]->type);
}
*/
/*
 * Goal:  Find proxy module and locate the loc_conf object.  Then insert getDynamicURL function pointer into it.
 */

	    return NGX_CONF_OK;
	}

/*
	static char *
	ngx_http_daisy_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
	{
printf("CF: %ld\n",(unsigned long)cf->ctx);
printf("Dad: %ld\n",(unsigned long)parent);
printf("Kid: %ld\n",(unsigned long)child);
		return NGX_CONF_OK;
	}
*/
