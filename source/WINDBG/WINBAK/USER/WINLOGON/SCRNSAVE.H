/****************************** Module Header ******************************\
* Module Name: scrnsave.h
*
* Copyright (c) 2000, Microsoft Corporation
*
* Secure Attention Key Sequence
*
* History:
* 01-23-91 Davidc       Created.
\***************************************************************************/

//
// Exported function prototypes
//

BOOL
ScreenSaverEnabled(
    PGLOBALS pGlobals
    );

DLG_RETURN_TYPE
RunScreenSaver(
    PGLOBALS pGlobals,
    BOOL WindowStationLocked
    );

