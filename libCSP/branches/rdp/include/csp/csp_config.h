/*
Cubesat Space Protocol - A small network-layer protocol designed for Cubesats
Copyright (C) 2010 Gomspace ApS (gomspace.com)
Copyright (C) 2010 AAUSAT3 Project (aausat3.space.aau.dk) 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * This is the default SVN version of the CSP configuration file.
 * It contains all the required values to compile CSP.
 * If you make any changes to the values in this file, please avoid
 * commiting them back to the repository unless they are required.
 *
 * This can be done by copying the file to another directory
 * and using include-path prioritisation, to prefer your local
 * copy over the default. Or perhaps even simpler by defining the
 * symbol CSP_USER_CONFIG in your makefile and naming your copy
 * csp_config_user.h
 *
 * This will also ensure that your copy of the configuraiton will never
 * be overwritten by a SVN checkout. However, please notice that
 * sometimes new configuration directives will be added to the configuration
 * at which point you should copy these to your local configuration too.
 *
 */

#ifndef _CSP_CONFIG_H_
#define _CSP_CONFIG_H_

#ifdef CSP_USER_CONFIG
#include <csp_config_user.h>
#else

/* General config */
#define CSP_DEBUG           1       // Enable/disable debugging output
#define CONN_MAX			10      // Number of statically allocated connection structs
#define CONN_QUEUE_LENGTH	100		// Number of packets potentially in queue for a connection

/* Transport layer config */
#define CSP_USE_RDP			1

/* Buffer config */
#define CSP_BUFFER_CALLOC	0		// Set to 1 to clear buffer at allocation
#define CSP_BUFFER_STATIC   0
#define CSP_BUFFER_SIZE     320
#define CSP_BUFFER_COUNT    12
#define CSP_BUFFER_FREE	    0  
#define CSP_BUFFER_USED	    1

#endif

#endif // _CSP_CONFIG_H_
