/*******************************************************************/
/*	      Copyright(c)  2000 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	ldebug.h
//
// Description: This module debug definitions for
//		the supervisor module.
//
// Author:	Stefan Solomon (stefans)    May 22, 2000.
//
// Revision History:
//
//***

#ifndef _LDEBUG_
#define _LDEBUG_


#if DBG

//
// Debug levels
//
#define DEBUG_STACK_TRACE     0x00000001
#define DEBUG_PIPE_OPERATIONS 0x00000002
#define DEBUG_NETBIOS         0x00000004

extern DWORD g_level;

#define GET_CONSOLE    1

#endif


#endif // _LDEBUG_

