/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 2000  Microsoft Corporation

Module Name:

    msystem.h

Abstract:

    Prototypes functions encapsulating OS function. This essentially covers
    everything that is not in NET***.

Author:

    Dan Hinsley (danhi) 10-Mar-2000

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments.

Notes:


Revision History:

    26-Aug-2000 beng
	Separated from port1632.h

--*/

//
// various memory allocation routines
//

PTCHAR
MGetBuffer(
    WORD usSize
    );

WORD
MAllocMem(
    DWORD Size,
    PVOID * ppBuffer
    );

WORD
MReallocMem(
    DWORD Size,
    PVOID * ppBuffer
    );

WORD
MFreeMem(
    PVOID pBuffer
    );

VOID
MSleep(
    DWORD Time
    );

WORD
MGetDateTime(
    PDATETIME pDateTime
    );

WORD
MSetDateTime(
    PDATETIME pDateTime
    );

WORD
DosQHandType(
    HFILE hf,
    PWORD pus1,
    PWORD pus2);

WORD 
MTimeToSecsSince1970(
    PLARGE_INTEGER time,
    PULONG seconds) ;

