/**
 * @file   ngx_http_hello_world_module.c
 * @author António P. P. Almeida <appa@perusio.net>
 * @date   Wed Aug 17 12:06:52 2011
 *
 * @brief  A hello world module for Nginx.
 *
 * @section LICENSE
 *
 * Copyright (C) 2011 by Dominic Fallows, António P. P. Almeida <appa@perusio.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "optimizely-sdk.h"


#define HELLO_WORLD "hello world\r\n"
#define OPTIMIZELY_SDK_ENABLED 1

static char *ngx_http_hello_world(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t *r);

#if OPTIMIZELY_SDK_ENABLED
static int optly_sdk_initialized = 0;
static int optly_sdk_handle = 0;
static int (*optly_sdk_init)();
static int (*optly_sdk_client)(char *);
static int (*optly_sdk_is_feature_enabled)(int handle, char*, optimizely_user_attributes*, char**);
static char* (*optly_sdk_get_feature_variable_string)(int, char*, char*, optimizely_user_attributes*, char**);
static char *sdk_key = NULL; // THIS MUST BE A VALID KEY
//static char *user_id = "0x123SomeUserId00";
#endif


/**
 * This module provided directive: hello world.
 *
 */
static ngx_command_t ngx_http_hello_world_commands[] = {

    { ngx_string("hello_world"), /* directive */
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS, /* location context and takes
                                            no arguments*/
      ngx_http_hello_world, /* configuration setup function */
      0, /* No offset. Only one context is supported. */
      0, /* No offset when storing the module configuration on struct. */
      NULL},

    ngx_null_command /* command termination */
};

/* The hello world string. */
static u_char ngx_hello_world[] = HELLO_WORLD;

/* The module context. */
static ngx_http_module_t ngx_http_hello_world_module_ctx = {
    NULL, /* preconfiguration */
    NULL, /* postconfiguration */

    NULL, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */

    NULL, /* create location configuration */
    NULL /* merge location configuration */
};

/* Module definition. */
ngx_module_t ngx_http_hello_world_module = {
    NGX_MODULE_V1,
    &ngx_http_hello_world_module_ctx, /* module context */
    ngx_http_hello_world_commands, /* module directives */
    NGX_HTTP_MODULE, /* module type */
    NULL, /* init master */
    NULL, /* init module */
    NULL, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};


#if OPTIMIZELY_SDK_ENABLED
/**
 * This loads the optimizely sdk and calls OptimizelySDKInit()
 * if successful set optly_sdk_initialized = 1
 */
static void initialize_optimizely_sdk(char *sdk_path, ngx_http_request_t *r)
{
    if (optly_sdk_initialized == 1) {
        return;
    }

    if (sdk_key == NULL) {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "no sdk_key defined");
        exit(1);
    }

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "initalizing the optimizely sdk");

    void *handle;
    char *error;

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "calling dlopen()");
    handle = dlopen (sdk_path, RTLD_LAZY);
    if (!handle) {
        fputs (dlerror(), stderr);
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, dlerror());
        exit(1);
    }
    
    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "dlopen succeded");

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "looking up optimizely_sdk_init");
    optly_sdk_init = dlsym(handle, "optimizely_sdk_init");
    if ((error = dlerror()) != NULL)  {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, error);
        exit(1);
    }

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "looking up optimizely_sdk_client");
    optly_sdk_client = dlsym(handle, "optimizely_sdk_client");
    if ((error = dlerror()) != NULL)  {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, error);
        exit(1);
    }

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "looking up optimizely_sdk_is_feature_enabled");
    optly_sdk_is_feature_enabled = dlsym(handle, "optimizely_sdk_is_feature_enabled");
    if ((error = dlerror()) != NULL)  {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, error);
        exit(1);
    }

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "looking up optimizely_sdk_get_feature_variable_string");
    optly_sdk_get_feature_variable_string = dlsym(handle, "optimizely_sdk_get_feature_variable_string");
    if ((error = dlerror()) != NULL)  {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, error);
        exit(1);
    }

    //create the sdk client handle
    optly_sdk_init();
    optly_sdk_handle = optly_sdk_client(sdk_key);
    optly_sdk_initialized = 1;
}
#endif

/**
 * Content handler.
 *
 * @param r
 *   Pointer to the request structure. See http_request.h.
 * @return
 *   The status of the response generation.
 */
static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t *r)
{
    ngx_buf_t *b;
    ngx_chain_t out;
    u_char *message;

    /* Set the Content-Type header. */
    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *) "text/plain";

    /* Allocate a new buffer for sending out the reply. */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    /* Insertion in the buffer chain. */
    out.buf = b;
    out.next = NULL; /* just one buffer */

    // No Error Handling is done below when using the optimzely SDK

#if OPTIMIZELY_SDK_ENABLED
    char *optly_error = NULL;
    optimizely_user_attributes a = {0};
    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "checking on feature");
    initialize_optimizely_sdk("/usr/local/nginx/sbin/optimizely-sdk.so", r);

    if (optly_sdk_is_feature_enabled(optly_sdk_handle, "current_greeting", &a, &optly_error)) {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "getting current greeting");
        message = (u_char *) optly_sdk_get_feature_variable_string(optly_sdk_handle, "current_greeting",
                                                                   "greeting", &a, &optly_error);
    } else {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "using default message");
        message = ngx_hello_world; // "hello world\r\n"
    }
#else
    message = ngx_hello_world; // "hello world\r\n"
#endif

    b->pos = message; /* first position in memory of the data */
    b->last = message + ((long int)ngx_strlen((const char *)message)); /* last position in memory of the data */

    b->memory = 1; /* content is in read-only memory */
    b->last_buf = 1; /* there will be no more buffers in the request */

    /* Sending the headers for the reply. */
    r->headers_out.status = NGX_HTTP_OK; /* 200 status code */
    /* Get the content length of the body. */
    r->headers_out.content_length_n = ((long int)ngx_strlen((const char *)message));
    ngx_http_send_header(r); /* Send the headers */

    // TODO free the message returned by the SDK

    /* Send the body, and return the status code of the output filter chain. */
    return ngx_http_output_filter(r, &out);
} /* ngx_http_hello_world_handler */

/**
 * Configuration setup function that installs the content handler.
 *
 * @param cf
 *   Module configuration structure pointer.
 * @param cmd
 *   Module directives structure pointer.
 * @param conf
 *   Module configuration structure pointer.
 * @return string
 *   Status of the configuration setup.
 */
static char *ngx_http_hello_world(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf; /* pointer to core location configuration */

    /* Install the hello world handler. */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_hello_world_handler;

    return NGX_CONF_OK;
} /* ngx_http_hello_world */
