/******************************************************************
 *  $Id: soap-ctx.c,v 1.1 2004/10/15 13:33:48 snowdrop Exp $
 *
 * CSOAP Project:  A SOAP client/server library in C
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
 * Email: ferhatayaz@jprogrammer.net
 ******************************************************************/
#include <libcsoap/soap-ctx.h>


SoapCtx* soap_ctx_new(SoapEnv *env) /* should only be used internally */
{
  SoapCtx* ctx = (SoapCtx*)malloc(sizeof(SoapCtx));
  ctx->env = env;
  ctx->attachments = NULL;
  
  return ctx;
}


void soap_ctx_add_files(SoapCtx* ctx, attachments_t *attachments)
{
  part_t *part;
  char href[MAX_HREF_SIZE];
  
  if (attachments==NULL) return;

  part = attachments->parts;
  while (part) {
    soap_ctx_add_file(ctx, part->filename, part->content_type, href);
    part = part->next;
  }
}


hstatus_t soap_ctx_add_file(SoapCtx* ctx, const char* filename, const char* content_type, char *dest_href)
{
  char cid[250];
  char id[250];
  part_t *part;
  static int counter = 1;
  FILE *test = fopen(filename, "r");
  if (!test) return FILE_ERROR_OPEN;
  
  fclose(test);
  
  /* generate an id */
  sprintf(id, "005512345894583%d", counter++);
  sprintf(dest_href, "cid:%s", id);
  sprintf(cid, "<%s>", id);

  /* add part to context */
  part = part_new(cid, filename, content_type, NULL, NULL);
  if (!ctx->attachments) ctx->attachments = attachments_new();
  attachments_add_part(ctx->attachments, part);

  return H_OK;
}


void soap_ctx_free(SoapCtx* ctx)
{
  if (!ctx) return;
  
  if (ctx->attachments)
    attachments_free(ctx->attachments);
  if (ctx->env)
    soap_env_free(ctx->env);
  
  free(ctx);
}


