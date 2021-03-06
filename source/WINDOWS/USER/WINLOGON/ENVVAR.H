/****************************** Module Header ******************************\
* Module Name: envvar.h
*
* Copyright (c) 2000, Microsoft Corporation
*
* Define apis in envvar.c
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

//
// Prototypes
//

BOOL
AppendNTPathWithAutoexecPath(
    PVOID *pEnv,
    LPTSTR lpPathVariable,
    LPTSTR lpAutoexecPath
    );

BOOL
CreateUserEnvironment(
    PVOID *pEnv
    );

BOOL
SetUserEnvironmentVariable(
    PVOID *pEnv,
    LPTSTR lpVariable,
    LPTSTR lpValue,
    BOOL bOverwrite
    );

DWORD
ExpandUserEnvironmentStrings(
    PVOID pEnv,
    LPTSTR lpSrc,
    LPTSTR lpDst,
    DWORD nSize
    );

BOOL
SetEnvironmentVariables(
    PGLOBALS pGlobals,
    PVOID *pEnv
    );

BOOL
SetHomeDirectoryEnvVars(
    PVOID *pEnv,
    LPTSTR lpHomeDirectory,
    LPTSTR lpHomeDrive,
    LPTSTR lpHomeShare,
    LPTSTR lpHomePath
    );

BOOL
ProcessAutoexec(
    PVOID *pEnv,
    LPTSTR lpPathVariable
    );

VOID
ChangeToHomeDirectory(
    PGLOBALS pGlobals,
    PVOID  *pEnv,
    LPTSTR lpHomeDir,
    LPTSTR lpHomeDrive,
    LPTSTR lpHomeShare,
    LPTSTR lpHomePath
    );
