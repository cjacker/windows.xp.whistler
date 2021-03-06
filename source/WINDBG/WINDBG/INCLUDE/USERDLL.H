/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    UserDll.h

Abstract:

    Header file for User Dll management

Author:

    Ramon J. San Andres (ramonsa)  26-October-2000

Environment:

    Win32, User Mode

--*/

VOID
ModListSetDefaultShe (
    SHE     SheDefault
    );

BOOL
ModListGetDefaultShe (
    CHAR    *Name,
    SHE     *She
    );

VOID
ModListSetSearchPath(
    CHAR    *Path
    );

VOID
ModListAddSearchPath(
    CHAR    *Path
    );

DWORD
ModListGetSearchPath(
    CHAR    *Buffer,
    DWORD   Size
    );

VOID
ModListInit(
    VOID
    );

VOID
ModListAdd(
    CHAR    *Name,
    SHE      SheDefault
    );

VOID
ModListModLoad(
    CHAR    *Name,
    SHE     SheCurrent
    );

VOID
ModListModUnload(
    CHAR    *Name
    );

PVOID
ModListGetNext(
    PVOID   Previous,
    CHAR   *Name,
    SHE    *SheDefault
    );
