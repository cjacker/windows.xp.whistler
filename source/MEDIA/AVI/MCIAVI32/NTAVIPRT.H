/******************************************************************************

   Copyright (C) Microsoft Corporation 1996-2000. All rights reserved.

   Title:   ntaviprt.h - Definitions for the portable win16/32 version of AVI

*****************************************************************************/
#ifndef _NTAVIPRT_H
#define _NTAVIPRT_H


#ifndef WIN32


#else

#define IsGDIObject(obj) (GetObjectType((HGDIOBJ)(obj)) != 0)

#endif

#endif

