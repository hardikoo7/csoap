/******************************************************************
 *  $Id: soap-router.h,v 1.11 2006/11/23 15:27:33 m0gg Exp $
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
#ifndef __csoap_router_h
#define __csoap_router_h

/**
 *
 * Authentication callback function for a cSOAP service.
 *
 */
typedef int (*soap_auth) (struct SoapEnv *request, const char *user, const char *pass);

/**
 *
 * The router object. A router can store a set of 
 * services. A service is a C function. 
 *
 */
struct SoapRouter
{
  SoapServiceNode *service_head;
  SoapServiceNode *service_tail;
  SoapService *default_service;
  soap_auth auth;
  xmlDocPtr description;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * Creates a new router object. Create a router if
 * you are implementing a soap server. Then register
 * the services to this router. 
 * <P>A router points also to http url context.
 *
 * @returns Soap router 
 *
 * @see soap_router_free
 *
 */
extern struct SoapRouter *soap_router_new(void);

/**
 *
 * Registers a SOAP service (in this case a C function)
 * to the router.
 * 
 * @param router The router object
 * @param func Function to register as a soap service
 * @param method Method name to call the function from 
 *  the client side.
 * @param urn The urn for this service
 *
 */
extern herror_t soap_router_register_service(struct SoapRouter *router, SoapServiceFunc func, const char *method, const char *urn);

/**
 *
 * Register a default service for the router.
 *
 */
extern herror_t soap_router_register_default_service(struct SoapRouter * router, SoapServiceFunc func, const char *method, const char *urn);

/**
 *
 * Register a service description for the router.
 *
 */
extern void soap_router_register_description(struct SoapRouter *router, xmlDocPtr doc);

/**
 *
 * Register a security provider for the router.
 *
 */
extern void soap_router_register_security(struct SoapRouter *router, soap_auth auth);

/**
 *
 * Searches for a registered soap service.
 *
 * @param router The router object
 * @param urn URN of the service
 * @param method The name under which the service was registered.
 *
 * @return The service if found, NULL otherwise.
 *
 */
extern SoapService *soap_router_find_service(struct SoapRouter *router, const char *urn, const char *method);

/**
 *
 * Frees the router object.
 *
 * @param router The router object to free
 *
 * @see soap_router_new
 *
 */
extern void soap_router_free(struct SoapRouter * router);

#ifdef __cplusplus
}
#endif

#endif
