/******************************************************************
 * $Id: echoattachments-client.c,v 1.3 2004/10/15 15:10:14 snowdrop Exp $
 *
 * CSOAP Project:  CSOAP examples project 
 * Copyright (C) 2003-2004  Ferhat Ayaz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA02111-1307USA
 *
 * Email: ferhatayaz@yahoo.com
 ******************************************************************/

#include <libcsoap/soap-client.h>


/*
static const char *url = "http://csoap.sourceforge.net/cgi-bin/csoapserver";
*/
static const char *url = "http://localhost:10000/echoattachment";
static const char *urn = "";
static const char *method = "echo";


void compareFiles(const char* received, const char *sent)
{
  FILE *f1, *f2;
  char c1, c2;
  long s1,s2;

  printf("Opening received file to compare: '%s'\n", received);
  f1 = fopen(received, "r");
  if (!f1) {
    fprintf(stderr, "Can not open '%s'\n", received);
    return;
  }

  printf("Opening sent file to compare: '%s'\n", sent);
  f2 = fopen(sent, "r");
  if (!f2) {
    fprintf(stderr, "Can not open '%s'\n", sent);
    fclose(f1);
    return;
  }

  fseek(f1, 0, SEEK_END);
  fseek(f2, 0, SEEK_END);

  s1 = ftell(f1);
  s2 = ftell(f2);

  fseek(f1, 0, SEEK_SET);
  fseek(f2, 0, SEEK_SET);

  if (s1 > s2) {

      printf("ERROR: files are not equal! Received file is bigger\n");
      fclose(f1); fclose(f2);
      return;

  } else if (s2 > s1) {

      printf("ERROR: files are not equal! Sent file is bigger\n");
      fclose(f1); fclose(f2);
      return;
  }

  while (feof(f1)) {
    
    c1= fgetc(f1);
    c2= fgetc(f2);
    if (c1 != c2){
      printf("ERROR: files are not equal! Byte compare failed\n");
      fclose(f1); fclose(f2);
      break;
    }
  }
  
  printf("OK: files are equal!\n");
  fclose(f1); fclose(f2);
  
}

int main(int argc, char *argv[])
{
  SoapCtx *ctx, *ctx2;
  char href[MAX_HREF_SIZE];
  xmlNodePtr fault;

  if (argc < 2) {
    fprintf(stderr, "usage: %s <filename> [url]\n", argv[0]);
    exit(1);
  }

  log_set_level(HLOG_VERBOSE);
  if (!soap_client_init_args(argc, argv)) {
	  return 1;
  }

  ctx = soap_client_ctx_new(urn, method);
  if (soap_ctx_add_file(ctx, argv[1], "application/octet-stream", href) != H_OK) {
    fprintf(stderr, "Error while adding '%s'\n", argv[1]);
    soap_ctx_free(ctx);
    exit(1);
  }
  soap_env_add_attachment(ctx->env,"source", href);

  printf("sending request ...\n");  
  if (argc > 2)
    ctx2 = soap_client_invoke(ctx, argv[2], "");
  else
    ctx2 = soap_client_invoke(ctx, url, "");

  fault = soap_env_get_fault(ctx2->env);
  if (fault) {
    soap_xml_doc_print(ctx2->env->root->doc);
  } else if (ctx2->attachments) {
    compareFiles(ctx2->attachments->parts->filename, argv[1]);
  } else {
    printf("No attachments!");
    soap_xml_doc_print(ctx2->env->root->doc);
  }

  soap_ctx_free(ctx2);
  soap_ctx_free(ctx);

  return 0;
}



