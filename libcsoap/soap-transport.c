/******************************************************************
*  $Id: soap-transport.c,v 1.1 2006/11/21 20:59:02 m0gg Exp $
*
* CSOAP Project:  A SOAP client/server library in C
* Copyright (C) 2007 Heiko Ronsdorf
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
* Email: hero@persua.de
******************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <libxml/tree.h>
#include <libxml/uri.h>

#include <nanohttp/nanohttp-common.h>
#include <nanohttp/nanohttp-socket.h>
#include <nanohttp/nanohttp-stream.h>
#include <nanohttp/nanohttp-request.h>
#include <nanohttp/nanohttp-server.h>
#include <nanohttp/nanohttp-logging.h>

#include "soap-fault.h"
#include "soap-env.h"
#include "soap-ctx.h"
#include "soap-service.h"
#include "soap-router.h"
#include "soap-addressing.h"
#include "soap-server.h"

#include "soap-nhttp.h"
#include "soap-nudp.h"

#include "soap-transport.h"

struct soap_transport
{
  char *scheme;
  void *data;
  msg_exchange invoke;
  struct soap_transport *next;
};

static struct soap_transport *head = NULL;
static char soap_transport_name[512] = "not set";

herror_t
soap_transport_receive(SoapCtx *request, SoapCtx **response)
{
  return soap_server_process(request, response);
}

static struct soap_transport *
_soap_transport_new(const char *scheme, void *data, msg_exchange invoke)
{
  struct soap_transport *ret;

  if (!(ret = (struct soap_transport *)malloc(sizeof(struct soap_transport))))
  {
    log_error2("malloc failed (%s)", strerror(errno));
    return NULL;
  }

  memset(ret, 0, sizeof(struct soap_transport));

  ret->scheme = strdup(scheme);
  ret->data = data;
  ret->invoke = invoke;

  log_verbose4("scheme=%s, data=%p, invoke=%p", ret->scheme, ret->data, ret->invoke);

  return ret;
}

static void *
_soap_transport_destroy(struct soap_transport *transport)
{
  void *ret;

  if (transport->scheme)
    free(transport->scheme);

  ret = transport->data;

  free(transport);

  return ret;
}

herror_t
soap_transport_server_init_args(int argc, char **argv)
{
  herror_t status;
  char hostname[256];

  if ((status = soap_nhttp_server_init_args(argc, argv)) != H_OK)
    return status;

  if ((status = soap_nudp_server_init_args(argc, argv)) != H_OK)
    return status;

  gethostname(hostname, 256);
  sprintf(soap_transport_name, "%s://%s:%i/csoap", soap_nhttp_get_protocol(), hostname, soap_nhttp_get_port());

  return H_OK;
}

herror_t
soap_transport_register_router(SoapRouter *router, const char *context)
{
  herror_t status;

  if ((status = soap_nhttp_register_router(router, context)) != H_OK)
    return status;

  if ((status = soap_nudp_register_router(router, context)) != H_OK)
    return status;
  
  return H_OK;
}

herror_t
soap_transport_add(const char *scheme, void *data, msg_exchange invoke)
{
  struct soap_transport *transport;
  struct soap_transport *walker;

  if (!(transport = _soap_transport_new(scheme, data, invoke)))
  {
    log_error1("_soap_transport_new failed");
    return H_OK;
  }

  if (head == NULL)
  {
    head = transport;
  }
  else
  {
    for (walker=head; walker->next; walker=head->next);
      /* nothing */
    walker->next = transport;
  }
  return H_OK;
}

herror_t
soap_transport_server_run(void)
{
  herror_t status;

  if ((status = soap_nhttp_server_run()) != H_OK)
    return status;

  if ((status = soap_nudp_server_run()) != H_OK)
    return status;

  return H_OK;
}
 
const char *
soap_transport_get_name(void)
{
  return soap_transport_name;
}

void
soap_transport_server_destroy(void)
{
  soap_nhttp_server_destroy();

  soap_nudp_server_destroy();
  
  return;
}

herror_t
soap_transport_client_init_args(int argc, char **argv)
{
  herror_t status;

  if ((status = soap_nhttp_client_init_args(argc, argv)) != H_OK)
    return status;

  if ((status = soap_nudp_client_init_args(argc, argv)) != H_OK)
    return status;

  return H_OK;
}

herror_t
soap_transport_client_invoke(SoapCtx *request, SoapCtx **response)
{
  struct soap_transport *walker;
  herror_t ret;
  xmlURI *dest;
  
  log_verbose1(__FUNCTION__);
  soap_xml_doc_print(request->env->root->doc);

  dest = soap_addressing_get_to_address(request->env);

  log_verbose2("trying to contact \"%s\"", soap_addressing_get_to_address_string(request->env));

  for (walker = head; walker; walker = walker->next)
  {
    if (!strcmp(walker->scheme, dest->scheme))
    {
      log_verbose3("found transport layer for \"%s\" (%p)", dest->scheme, walker->invoke);
      ret = walker->invoke(walker->data, request, response);
      xmlFreeURI(dest);
      return ret;
    }
  }
  ret = herror_new("soap_transport_client_invoke", 0, "no transport service found for \"%s\"", dest->scheme);
  xmlFreeURI(dest);
  return ret;
}

void
soap_transport_client_destroy(void)
{
  soap_nhttp_client_destroy();

  soap_nudp_client_destroy();

  return;
}
