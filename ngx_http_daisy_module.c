/*
 * Copyright (C) Evan Miller
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <string.h>
#include "daisy.h"

	static char* ngx_http_daisy(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

/*
	typedef struct {
	    ngx_str_t    db_host;
	    ngx_str_t    db_inst;
	} ngx_http_daisy_loc_conf_t;
*/

	static ngx_command_t  ngx_http_daisy_commands[] = {
	    { ngx_string("daisy"),
	      NGX_HTTP_LOC_CONF|NGX_CONF_1MORE, //NGX_CONF_NOARGS,
	      ngx_http_daisy,
	      NGX_HTTP_LOC_CONF_OFFSET,
	      //offsetof(ngx_http_daisy_loc_conf_t, db_host),
	      0,
	      NULL },

	/*
	    { ngx_string("circle_gif"),
	      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
	      ngx_http_circle_gif,
	      NGX_HTTP_LOC_CONF_OFFSET,
	      0,
	      NULL },

	    { ngx_string("circle_gif_min_radius"),
	      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	      ngx_conf_set_num_slot,
	      NGX_HTTP_LOC_CONF_OFFSET,
	      offsetof(ngx_http_circle_gif_loc_conf_t, min_radius),
	      NULL },

	    { ngx_string("circle_gif_max_radius"),
	      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	      ngx_conf_set_num_slot,
	      NGX_HTTP_LOC_CONF_OFFSET,
	      offsetof(ngx_http_circle_gif_loc_conf_t, max_radius),
	      NULL },

	    { ngx_string("circle_gif_step_radius"),
	      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
	      ngx_conf_set_num_slot,
	      NGX_HTTP_LOC_CONF_OFFSET,
	      offsetof(ngx_http_circle_gif_loc_conf_t, step_radius),
	      NULL },
	*/
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
	    NULL   /* merge location configuration */
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

	#ifdef Z
	static ngx_http_module_t  ngx_http_circle_gif_module_ctx = {
	    ngx_http_circle_gif_preconf,   /* preconfiguration */
	    ngx_http_circle_gif_postconf,  /* postconfiguration */

	    NULL,                          /* create main configuration */
	    NULL,                          /* init main configuration */

	    NULL,                          /* create server configuration */
	    NULL,                          /* merge server configuration */

	    ngx_http_circle_gif_create_loc_conf,  /* create location configuration */
	    ngx_http_circle_gif_merge_loc_conf /* merge location configuration */
	};


	ngx_module_t  ngx_http_circle_gif_module = {
	    NGX_MODULE_V1,
	    &ngx_http_circle_gif_module_ctx, /* module context */
	    ngx_http_circle_gif_commands,   /* module directives */
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


	static ngx_int_t
	ngx_http_circle_gif_handler(ngx_http_request_t *r)
	{
	    ngx_int_t     rc;
	    ngx_buf_t    *b;
	    ngx_chain_t   out;
	    unsigned char *image;
	    // all right, let's figure out what they're asking for
	    u_int bg_color, fg_color, req_radius = 0;
	    size_t i;
	    ngx_int_t power;
	    char *digit;
	    unsigned char bg[3]; 
	    unsigned char fg[3];

	    ngx_http_circle_gif_loc_conf_t  *cglcf;
	    cglcf = ngx_http_get_module_loc_conf(r, ngx_http_circle_gif_module);

	    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
		return NGX_HTTP_NOT_ALLOWED;
	    }

	    rc = ngx_http_discard_request_body(r);

	    if (rc != NGX_OK && rc != NGX_AGAIN) {
		return rc;
	    }

	    if (r->headers_in.if_modified_since) {
		return NGX_HTTP_NOT_MODIFIED;
	    }

	    digit = (char *)r->uri.data + r->uri.len - 1;

	    if (!(*digit-- == 'f' && *digit-- == 'i' && *digit-- == 'g' && *digit-- == '.')) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Invalid extension with %s", digit);
		return NGX_HTTP_NOT_FOUND;
	    }
	    for(i=0, power=1; *digit >= '0' && *digit <= '9' && i < (r->uri.len - 7 - 7 - 4); i++, power *= 10) {
		req_radius += (*digit-- - '0')*power;
	    }
	    fg_color   = strtol(digit -= 6, NULL, 16); // "XXXXXX"
	    bg_color   = strtol(digit -= 7, NULL, 16); // "XXXXXX/"

	    bg[RED] = bg_color >> 16;
	    bg[GREEN] = bg_color >> 8;
	    bg[BLUE] = bg_color;

	    fg[RED] = fg_color >> 16;
	    fg[GREEN] = fg_color >> 8;
	    fg[BLUE] = fg_color;

	    r->headers_out.content_type.len = sizeof("image/gif") - 1;
	    r->headers_out.content_type.data = (u_char *) "image/gif";

	    if (req_radius < cglcf->min_radius || req_radius > cglcf->max_radius) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Invalid radius %ui", req_radius);
		return NGX_HTTP_NOT_FOUND;
	    }

	    int radius_index = radius2index(req_radius, cglcf);

	    r->headers_out.status = NGX_HTTP_OK;
	    r->headers_out.content_length_n = cglcf->circle_sizes[radius_index];

	    if (r->method == NGX_HTTP_HEAD) {
		rc = ngx_http_send_header(r);

		if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
		    return rc;
		}
	    }

	    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
	    if (b == NULL) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	    }

	    out.buf = b;
	    out.next = NULL;

	    image = ngx_palloc(r->pool, cglcf->circle_sizes[radius_index]);
	    if (image == NULL) {
	      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate memory for circle image.");
	      return NGX_HTTP_INTERNAL_SERVER_ERROR;
	    }
	    ngx_memcpy(image, cglcf->circle_templates[radius_index], cglcf->circle_sizes[radius_index]);
	    ngx_http_circle_gif_colorize(image, bg, fg);

	    b->pos = image;
	    b->last = image + cglcf->circle_sizes[radius_index];

	    b->memory = 1;
	    b->last_buf = 1;

	    rc = ngx_http_send_header(r);

	    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
		return rc;
	    }

	    return ngx_http_output_filter(r, &out);
	}
	#endif

	static char *
	ngx_http_daisy(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
	{
	/*
	    ngx_http_core_loc_conf_t  *clcf;
	    ngx_http_circle_gif_loc_conf_t *cglcf = conf;

	    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	    clcf->handler = ngx_http_circle_gif_handler;

	    cglcf->enable = 1;
	*/
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
	    return NGX_CONF_OK;
	}
