/******************************************************************
 *  $Id: nanohttp-request.h,v 1.1 2004/10/15 13:30:42 snowdrop Exp $
 *
 * CSOAP Project:  A http client/server library in C
 * Copyright (C) 2003-2004  Ferhat Ayaz
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
 * Email: ferhatayaz@yahoo.com
 ******************************************************************/
#ifndef NANO_HTTP_REQUEST_H 
#define NANO_HTTP_REQUEST_H  

#include <nanohttp/nanohttp-stream.h>
#include <nanohttp/nanohttp-mime.h>

/*
  request object
 */
typedef struct hrequest
{
  hreq_method_t method;
  http_version_t version;
  char path[REQUEST_MAX_PATH_SIZE];

  hpair_t *query;
  hpair_t *header;

  http_input_stream_t *in;
  content_type_t *content_type;
  attachments_t *attachments;
  char root_part_id[150];
}hrequest_t;

hrequest_t *hrequest_new_from_socket(hsocket_t sock);
void hrequest_free(hrequest_t *req);

#endif





