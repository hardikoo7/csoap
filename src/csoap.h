/******************************************************************
 *  $Id: csoap.h,v 1.2 2003/05/20 21:00:18 snowdrop Exp $
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
#ifndef CSOAP_H 
#define CSOAP_H 


/* Include csoap headers */
#include "csoapcall.h"  
#include "csoapfault.h" 
#include "csoapurl.h"
#include "csoaptypes.h"

/**
 * Initialize routine. Must be called before any
 * other csoap API function.
 *
 * @param argc command line parameter
 * @param argv command line parameter
 *
 * @return CSOAP_OK if successfull.
 */
int SoapInit(int argc, char *argv[]);


/**
 * Free a string allocated by a csoap 
 * function.
 *
 *  @param str the string to free
 */
void SoapFreeStr(char* str); 


#endif 
