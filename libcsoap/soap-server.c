/******************************************************************
*  $Id: soap-server.c,v 1.7 2004/10/28 10:30:46 snowdrop Exp $
*
* CSOAP Project:  A SOAP client/server library in C
* Copyright (C) 2003  Ferhat Ayaz
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA  02111-1307, USA.
* 
* Email: ayaz@jprogrammer.net
******************************************************************/
#include <libcsoap/soap-server.h>
#include <nanohttp/nanohttp-server.h>
#include <string.h>


typedef struct _SoapRouterNode
{
	char *context;
	SoapRouter *router;
	struct _SoapRouterNode *next;

}SoapRouterNode;

SoapRouterNode *head = NULL;
SoapRouterNode *tail = NULL;

static
SoapRouterNode *router_node_new(SoapRouter *router, 
								const char *context, 
								SoapRouterNode *next);

static
SoapRouter *router_find(const char *context);

static
void _soap_server_send_ctx(httpd_conn_t* conn, SoapCtx *ctxres);

/*---------------------------------*/
void soap_server_entry(httpd_conn_t *conn, hrequest_t *req);
static
void _soap_server_send_env(http_output_stream_t *out, SoapEnv* env);
static
void _soap_server_send_fault(httpd_conn_t *conn, hpair_t *header, 
							 const char* errmsg);
/*---------------------------------*/


herror_t soap_server_init_args(int argc, char *argv[])
{
	return httpd_init(argc, argv);
}


int soap_server_register_router(SoapRouter *router, const char* context)
{

	if (!httpd_register(context, soap_server_entry)) {
		return 0;
	}

	if (tail == NULL) {
		head = tail = router_node_new(router, context, NULL);
	} else {
		tail->next =  router_node_new(router, context, NULL);
		tail = tail->next;
	}

	return 1;
}


herror_t soap_server_run()
{
	return httpd_run();
}


void soap_server_destroy()
{
	SoapRouterNode *node = head;
	SoapRouterNode *tmp;

	while (node != NULL) {
		tmp = node->next;
		log_verbose2("soap_router_free(%p)", node->router);
		soap_router_free(node->router);
		free(node->context);
		free(node);
		node = tmp;
	}
}


void soap_server_entry(httpd_conn_t *conn, hrequest_t *req)
{
	hpair_t *header = NULL;
	char buffer[1054];
	char urn[150];
	char method[150];
	SoapCtx *ctx, *ctxres;
	SoapRouter *router;
	SoapService *service;
	SoapEnv *env;
	herror_t err;

	if (req->method != HTTP_REQUEST_POST) {

		httpd_send_header(conn, 200, "OK");
		http_output_stream_write_string(conn->out, "<html><head></head><body>");
		http_output_stream_write_string(conn->out, "<h1>Sorry! </h1><hr>");
		http_output_stream_write_string(conn->out, "I only speak with 'POST' method");
		http_output_stream_write_string(conn->out, "</body></html>");
		return;
	}


	header = hpairnode_new(HEADER_CONTENT_TYPE, "text/xml", NULL);

	err = soap_env_new_from_stream(req->in, &env);
	if (err != H_OK) 
	{
  		_soap_server_send_fault(conn, header, herror_message(err));
		herror_release(err);
		return;
	}


	if (env == NULL) {

		_soap_server_send_fault(conn, header,"Can not receive POST data!");

	} else {

	  ctx = soap_ctx_new(env);
	  soap_ctx_add_files(ctx, req->attachments);

		if (ctx->env == NULL) {

			_soap_server_send_fault(conn, header,"Can not parse POST data!");

		} else {

			/*soap_xml_doc_print(env->root->doc);*/

			router = router_find(req->path);

			if ( router == NULL) {

				_soap_server_send_fault(conn, header, "Can not find router!");

			} else {

				if (!soap_env_find_urn(ctx->env, urn)) {

					_soap_server_send_fault(conn, header, "No URN found!");
					soap_ctx_free(ctx);
					return;
				} else {
					log_verbose2("urn: '%s'", urn);
				}

				if (!soap_env_find_methodname(ctx->env, method)) {

					_soap_server_send_fault(conn, header, "No method found!");
					soap_ctx_free(ctx);
					return;
				}else {
					log_verbose2("method: '%s'", method);
				}

				service = soap_router_find_service(router, urn, method);

				if (service == NULL) {

					sprintf(buffer, "URN '%s' not found", urn);
					_soap_server_send_fault(conn, header, buffer);
					soap_ctx_free(ctx);
					return;
				} else {

					log_verbose2("func: %p", service->func);
					ctxres = soap_ctx_new(NULL);
					/* ===================================== */
          /*       CALL SERVICE FUNCTION           */
					/* ===================================== */
					err = service->func(ctx, ctxres);
					if (err != H_OK) {
						sprintf(buffer, "Service returned following error message: '%s'",
							herror_message(err));
						herror_release(err);
						_soap_server_send_fault(conn, header, buffer);
						soap_ctx_free(ctx);
						return;
					}

					if (ctxres->env == NULL) {

						sprintf(buffer, "Service '%s' returned no envelope", urn);
						_soap_server_send_fault(conn, header, buffer);
						soap_ctx_free(ctx);
						return;

					} else {

/*						httpd_send_header(conn, 200, "OK");
						_soap_server_send_env(conn->out, ctxres->env);
*/
  				 _soap_server_send_ctx(conn, ctxres);
						/* free envctx */
						soap_ctx_free(ctxres);
					}

				}

			}   
		}
		soap_ctx_free(ctx);
	}
}


static
void _soap_server_send_ctx(httpd_conn_t* conn, SoapCtx *ctx)
{
	xmlBufferPtr buffer;
	static int counter = 1;
	char strbuffer[150];
	part_t *part;

	if (ctx->env == NULL || ctx->env->root == NULL) return;

	buffer = xmlBufferCreate();
/*	xmlIndentTreeOutput = 1;*/
	xmlThrDefIndentTreeOutput(1);
/*  xmlKeepBlanksDefault(0);*/
	xmlNodeDump(buffer, ctx->env->root->doc, ctx->env->root, 1 ,1);

	if (ctx->attachments)
	{
	  sprintf(strbuffer, "000128590350940924234%d", counter++);
	  httpd_mime_send_header(conn, strbuffer, "", "text/xml", 200, "OK");
	  httpd_mime_next(conn, strbuffer, "text/xml", "binary");
		http_output_stream_write_string(conn->out, (const char*)xmlBufferContent(buffer));
		part = ctx->attachments->parts;
		while (part)
		{
  		httpd_mime_send_file(conn, part->id, part->content_type, part->transfer_encoding, part->filename);
		  part = part->next;
		}
		httpd_mime_end(conn);
	}
	else
	{
	  httpd_send_header(conn, 200, "OK");
		http_output_stream_write_string(conn->out, (const char*)xmlBufferContent(buffer));
	}
	xmlBufferFree(buffer);  

}

static
void _soap_server_send_env(http_output_stream_t *out, SoapEnv* env)
{
	xmlBufferPtr buffer;
	if (env == NULL || env->root == NULL) return;

	buffer = xmlBufferCreate();
	xmlNodeDump(buffer, env->root->doc, env->root, 1 ,1);
	http_output_stream_write_string(out,  (const char*)xmlBufferContent(buffer));
	xmlBufferFree(buffer);  

}

static
void _soap_server_send_fault(httpd_conn_t *conn, hpair_t *header, 
							 const char* errmsg)
{
	SoapEnv *envres;
	herror_t err;
	char buffer[45];
	httpd_set_headers(conn, header);
	err = httpd_send_header(conn, 500, "FAILED");
	if (err != H_OK) {
		 /* WARNING: unhandled exception !*/
		log_error4("%s():%s [%d]", herror_func(err), herror_message(err), herror_code(err));
		return;
	}

	err = soap_env_new_with_fault(Fault_Server,
		errmsg?errmsg:"General error",
		"cSOAP_Server", NULL, &envres);
	 if (err != H_OK) {
		 log_error1(herror_message(err));
		http_output_stream_write_string(conn->out, "<html><head></head><body>");
		http_output_stream_write_string(conn->out, "<h1>Error</h1><hr>");
		http_output_stream_write_string(conn->out, "Error while sending fault object:<br>Message: ");
		http_output_stream_write_string(conn->out, herror_message(err));
		http_output_stream_write_string(conn->out, "<br>Function: ");
		http_output_stream_write_string(conn->out, herror_func(err));
		http_output_stream_write_string(conn->out, "<br>Error code: ");
		sprintf(buffer, "%d", herror_code(err));
		http_output_stream_write_string(conn->out, buffer);
		http_output_stream_write_string(conn->out, "</body></html>");
		return;

		 herror_release(err);
	 } else {
	_soap_server_send_env(conn->out, envres);
	 }

}



static 
SoapRouterNode *router_node_new(SoapRouter *router, 
								const char *context,
								SoapRouterNode *next)
{
	SoapRouterNode *node;
	const char *noname = "/lost_found";

	node = (SoapRouterNode*)malloc(sizeof(SoapRouterNode));
	if (context) {
		node->context = (char*)malloc(strlen(context)+1);
		strcpy(node->context, context);
	} else {
		log_warn2("context is null. Using '%s'", noname);
		node->context = (char*)malloc(strlen(noname)+1);
		strcpy(node->context, noname);
	}

	node->router = router;
	node->next = next;

	return node;
}


static
SoapRouter *router_find(const char* context)
{
	SoapRouterNode *node = head;

	while (node != NULL) {
		if (!strcmp(node->context, context))
			return node->router;
		node = node->next;
	}

	return NULL;
}





