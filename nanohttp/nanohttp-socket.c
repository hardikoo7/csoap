/******************************************************************
 *  $Id: nanohttp-socket.c,v 1.7 2004/01/21 12:28:20 snowdrop Exp $
 *
 * CSOAP Project:  A http client/server library in C
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
#include <nanohttp/nanohttp-socket.h>
#include <nanohttp/nanohttp-common.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif 

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>

/*--------------------------------------------------
  FUNCTION: hsocket_module_init
----------------------------------------------------*/
int hsocket_module_init()
{
  /* nothing to init for unix sockets */
  return 0;
}

/*--------------------------------------------------
  FUNCTION: hsocket_module_destroy
----------------------------------------------------*/
void hsocket_module_destroy()
{
  /* nothing to destroy for unix sockets */
}


/*--------------------------------------------------
  FUNCTION: hsocket_init
----------------------------------------------------*/
int hsocket_init(hsocket_t *sock)
{
  /* nothing to init for unix sockets */
  /* just set the descriptor to -1 */
  *sock = -1;
  return 0;
}


/*--------------------------------------------------
  FUNCTION: hsocket_free
----------------------------------------------------*/
void hsocket_free(hsocket_t sock)
{
  /* nothing to free for unix sockets */
}


/*--------------------------------------------------
  FUNCTION: hsocket_open
----------------------------------------------------*/
int hsocket_open(hsocket_t *dsock, const char* hostname, int port)
{
  int sock;
  struct sockaddr_in address; 
  struct hostent* host;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock <= 0) return HSOCKET_CAN_NOT_CREATE;

  /* Get host data */
  host = gethostbyname(hostname);
  if (host == NULL) return HSOCKET_CAN_NOT_GET_HOSTNAME;

  /* set server addresss */
  address.sin_family = host->h_addrtype;
  address.sin_port = htons(port);
  memcpy((char*)&address.sin_addr.s_addr,
	 host->h_addr_list[0], host->h_length);

  /* connect to the server */
  if (connect(sock, (struct sockaddr*) &address, sizeof(address)) != 0)
    return HSOCKET_CAN_NOT_CONNECT;

  *dsock = sock;
  return HSOCKET_OK;
}


/*--------------------------------------------------
  FUNCTION: hsocket_close
----------------------------------------------------*/
int hsocket_bind(hsocket_t *dsock, int port)
{
  int sock;
  struct sockaddr_in addr;
 
  /* create socket */
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    log_error2("Can not create socket: '%s'\n", strerror(errno));
    return HSOCKET_CAN_NOT_CREATE;
  }

  /* bind socket */
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port); /* short, network byte order */
  addr.sin_addr.s_addr = INADDR_ANY;
  memset(&(addr.sin_zero), '\0', 8); /* zero the rest of the struct */

  if (bind(sock, (struct sockaddr *)&addr, 
	   sizeof(struct sockaddr)) == -1) {
    log_error2("Can not bind: '%s'\n", strerror(errno));
    return HSOCKET_CAN_NOT_BIND;
  }

  *dsock = sock;
  return HSOCKET_OK;
}

/*--------------------------------------------------
  FUNCTION: hsocket_listen
----------------------------------------------------*/
int hsocket_listen(hsocket_t sock, int n)
{
  if (listen(sock, n) == -1) {
    log_error2("Can not listen: '%s'\n", strerror(errno));
    return HSOCKET_CAN_NOT_LISTEN;
  }
  return HSOCKET_OK;
}


/*--------------------------------------------------
  FUNCTION: hsocket_listen
----------------------------------------------------*/
int hsocket_accept(hsocket_t sock, hsocket_t *dest)
{
  socklen_t asize;
  int sockfd;
  struct sockaddr_in addr;

  asize = sizeof(struct sockaddr_in);
  sockfd = accept(sock, (struct sockaddr *)&addr, &asize);
  
  if (sockfd == -1) {
    //httpd_log("httpd_run(): '%s'\n", strerror(errno));
    return HSOCKET_CAN_NOT_ACCEPT;
  }

  log_debug3("accept new socket (%d) from '%s'\n", sockfd,
	     SAVE_STR(((char*)inet_ntoa(addr.sin_addr))) );

  *dest = sockfd;
  return HSOCKET_OK;  
}


/*--------------------------------------------------
  FUNCTION: hsocket_close
----------------------------------------------------*/
void hsocket_close(hsocket_t sock)
{
  close(sock);
}


/*--------------------------------------------------
  FUNCTION: hsocket_send
----------------------------------------------------*/
int hsocket_nsend(hsocket_t sock, const char* buffer, int n)
{
  int size;

  size = send((int)sock, buffer, n, 0);
  if (size == -1)
    return HSOCKET_CAN_NOT_SEND;

  return HSOCKET_OK;
}

/*--------------------------------------------------
  FUNCTION: hsocket_send
----------------------------------------------------*/
int hsocket_send(hsocket_t sock, const char* buffer)
{
  int size;
  size = send((int)sock, buffer, strlen(buffer), 0);
  if (size == -1)
    return HSOCKET_CAN_NOT_SEND;

  return HSOCKET_OK;
}


int hsocket_read(hsocket_t sock, char* buffer, int total, int force)
{
  int status;
  int totalRead;

  totalRead = 0;

  do {
    status = recv(sock, &buffer[totalRead], total - totalRead, 0);
    if (!force) return status;
    if (status > 0) {
      totalRead += status;
    } else {
      return status;
    }
    if (totalRead >= total)
      return 0;
  } while (1);
}

/*--------------------------------------------------
  FUNCTION: hsocket_recv
----------------------------------------------------*/
int hsocket_recv(hsocket_t sock, char** buffer, int *totalSize)
{
  ssize_t size;
  int chunk=1;
  char tmp[HSOCKET_MAX_BUFSIZE+1];
  int fsize;
  int bufSize;

  if (*totalSize > 0) {
    bufSize = *totalSize;
  } else {
    bufSize = HSOCKET_MAX_BUFSIZE;
  }

  *totalSize = 0;

  /* calculate first size for realloc */
  if (*buffer) {
    fsize = strlen(*buffer);
  } else {
    fsize = 0;
  }

  do {
    
    size = recv(sock, tmp, bufSize, 0);
    bufSize = HSOCKET_MAX_BUFSIZE;

    if (size == -1) {
      log_error1("Error reading from socket\n");
      return HSOCKET_CAN_NOT_RECEIVE;
    }

    if (size == 0) {
      break;
    }

    *totalSize += size;
    if (*buffer) {
      log_debug2("reallocation %d bytes",*totalSize+fsize+1);
      *buffer = (char*)realloc((char*)*buffer, 
			       (*totalSize)+fsize+HSOCKET_MAX_BUFSIZE);
      strcat(*buffer, tmp);
    } else {
      log_debug1("Allocating");
      *buffer = (char*)realloc(NULL, *totalSize+1);
      strcpy(*buffer, tmp);
    }

    log_debug1("Assigning");
    (*buffer)[*totalSize+fsize] = '\0';	
    chunk++;
  } while (size > 0);

  log_debug1("Returning");
  return HSOCKET_OK;
}



/*--------------------------------------------------
  FUNCTION: hsocket_recv
----------------------------------------------------*/
int hsocket_recv_cb(hsocket_t sock, 
		    hsocket_recv_callback cb, void *userdata)
{
  ssize_t size;
  char tmp[HSOCKET_MAX_BUFSIZE+1];

  do {
    
    size = recv(sock, tmp, HSOCKET_MAX_BUFSIZE, 0);

    if (size == -1) {
      log_error1("Error reading from socket\n");
      return HSOCKET_CAN_NOT_RECEIVE;
    }
    
    if (size == 0) {
      break;
    }

    tmp[size]='\0';
    if (!cb(sock, tmp, size, userdata)) {
      break;
    } 

  } while (size > 0);

  return HSOCKET_OK;
}



/*--------------------------------------------------
  FUNCTION: hbufsocket_read
----------------------------------------------------*/
int hbufsocket_read(hbufsocket_t *bufsock, char *buffer, int size)
{
  int status;
  int tmpsize;

  if (bufsock->bufsize - bufsock->cur >= size) {

    log_debug1("no need to read from socket");
    strncpy(buffer, &(bufsock->buffer[bufsock->cur]), size);
    bufsock->cur += size;
    return HSOCKET_OK;

  } else {

    tmpsize = bufsock->bufsize - bufsock->cur;
    log_debug2("tmpsize = %d", tmpsize);

    if (tmpsize > 0)
      strncpy(buffer, &(bufsock->buffer[bufsock->cur]), tmpsize); 

    size -= tmpsize;

    free(bufsock->buffer);
 
    status = hsocket_read(bufsock->sock, &buffer[tmpsize], size, 1);
    if (status == size) {
      bufsock->buffer = (char*)malloc(size+1);
      strncpy(bufsock->buffer, &buffer[tmpsize], size);
      bufsock->cur = size;
    } else {
      return status;
    }
    
    return HSOCKET_OK;
  }
}








