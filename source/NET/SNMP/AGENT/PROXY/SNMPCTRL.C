//-------------------------- MODULE DESCRIPTION ----------------------------
//
//  snmpctrl.c
//
//  Copyright 2000 Technology Dynamics, Inc.
//
//  All Rights Reserved!!!
//
//	This source code is CONFIDENTIAL and PROPRIETARY to Technology
//	Dynamics. Unauthorized distribution, adaptation or use may be
//	subject to civil and criminal penalties.
//
//  All Rights Reserved!!!
//
//---------------------------------------------------------------------------
//
//  Provides service control functionality for Proxy Agent.
//
//  Project:  Implementation of an SNMP Agent for Microsoft's NT Kernel
//
//  $Revision:   1.1  $
//  $Date:   03 Jul 2000 17:27:32  $
//  $Author:   mlk  $
//
//  $Log:   N:/agent/proxy/vcs/snmpctrl.c_v  $
//
//     Rev 1.1   03 Jul 2000 17:27:32   mlk
//  Integrated w/297 (not as service).
//
//     Rev 1.0   20 May 2000 20:13:56   mlk
//  Initial revision.
//
//     Rev 1.4   01 May 2000  1:00:20   unknown
//  mlk - changes due to nt v1.262.
//
//     Rev 1.3   29 Apr 2000 19:14:54   mlk
//  Cleanup.
//
//     Rev 1.2   27 Apr 2000 23:13:08   mlk
//  Enhanced dbgprintf functionality.
//
//     Rev 1.1   23 Apr 2000 17:48:12   mlk
//  Registry, traps, and cleanup.
//
//     Rev 1.0   10 Apr 2000 20:33:54   mlk
//  Initial revision.
//
//---------------------------------------------------------------------------

//--------------------------- VERSION INFO ----------------------------------

static char *vcsid = "@(#) $Logfile:   N:/agent/proxy/vcs/snmpctrl.c_v  $ $Revision:   1.1  $";

//--------------------------- WINDOWS DEPENDENCIES --------------------------

//--------------------------- STANDARD DEPENDENCIES -- #include<xxxxx.h> ----

#include <windows.h>

#include <process.h>
#include <stdio.h>
#include <string.h>

#include <winsvc.h>


//--------------------------- MODULE DEPENDENCIES -- #include"xxxxx.h" ------

//--------------------------- SELF-DEPENDENCY -- ONE #include"module.h" -----

#include "snmpctrl.h"


//--------------------------- PUBLIC VARIABLES --(same as in module.h file)--

//--------------------------- PRIVATE CONSTANTS -----------------------------

//--------------------------- PRIVATE STRUCTS -------------------------------

//--------------------------- PRIVATE VARIABLES -----------------------------

//--------------------------- PRIVATE PROTOTYPES ----------------------------

//--------------------------- PRIVATE PROCEDURES ----------------------------

//--------------------------- PUBLIC PROCEDURES -----------------------------


// OPENISSUE - danhi says that it was not known that os/2 allows user
// OPENISSUE - defined service controls, they are now not functional in
// OPENISSUE - NT (they were functional in build 258 and earlier).


INT _CRTAPI1 main(
    IN int  argc,     //argument count
    IN char *argv[])  //argument vector
    {
    SC_HANDLE hSCManager;
    SC_HANDLE hService;
    DWORD     dwControl;
    SERVICE_STATUS ServiceStatus;
    INT       loglevel = -1;
    INT       logtype  = -1;
    INT       stdctrl  = -1;

    if      (argc == 1)
        {
        printf("Error:  No arguments specified.\n", *argv);
        printf("\nusage:  snmpctrl [/loglevel:<level>] [/logtype:<type>]\n");

        return 1;
        }

    while(--argc)
        {
        INT temp;

        argv++;

        if      (1 == sscanf(*argv, "/loglevel:%d", &temp))
            {
            loglevel = temp;

            if (loglevel < SNMP_SERVICE_LOGLEVEL_MIN ||
                SNMP_SERVICE_LOGLEVEL_MAX < loglevel)
                {
                printf("Error:  LogLevel must be %d thru %d.\n",
                    SNMP_SERVICE_LOGLEVEL_MIN, SNMP_SERVICE_LOGLEVEL_MAX);
                exit(1);
                }
            }
        else if (1 == sscanf(*argv, "/logtype:%d", &temp))
            {
            logtype = temp;

            if (logtype < SNMP_SERVICE_LOGTYPE_MIN ||
                SNMP_SERVICE_LOGTYPE_MAX < logtype)
                {
                printf("Error:  LogType must be %d thru %d.\n",
                    SNMP_SERVICE_LOGTYPE_MIN, SNMP_SERVICE_LOGTYPE_MAX);
                exit(1);
                }
            }
        else if (1 == sscanf(*argv, "/service_control:%d", &temp))
            {
            stdctrl = temp;

            if (stdctrl != SERVICE_CONTROL_STOP &&
                stdctrl != SERVICE_CONTROL_PAUSE &&
                stdctrl != SERVICE_CONTROL_CONTINUE &&
                stdctrl != SERVICE_CONTROL_INTERROGATE)
                {
                printf("Error:  Service_Control invalid.\n");
                exit(1);
                }
            }
        else
            {
            printf("Error:  Argument %s is invalid.\n", *argv);
            printf("\nusage:  snmpctrl [/loglevel:<level>] [/logtype:<type>]\n");

            return 1;
            }
        } // end while()

    if      ((hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT))
             == NULL)
        {
        printf("error on ConnectSCManager %d\n", GetLastError());

        return 1;
        }
    else if ((hService = OpenService(hSCManager, "snmp",
                                     SERVICE_ALL_ACCESS))
        == NULL)
        {
        printf("error on OpenService %d\n", GetLastError());

        return 1;
        }

    if (loglevel >= 0)
        {
        dwControl = loglevel + SNMP_SERVICE_LOGLEVEL_BASE;

        if (!ControlService(hService, dwControl, &ServiceStatus))
            {
            printf("error on ControlService %d\n", GetLastError());

            return 1;
            }
        else
            printf("Service LogLevel changed to %d.\n", loglevel);
        }

    if (logtype >= 0)
        {
        dwControl = logtype  + SNMP_SERVICE_LOGTYPE_BASE;

        if (!ControlService(hService, dwControl, &ServiceStatus))
            {
            printf("error on ControlService %d\n", GetLastError());

            return 1;
            }
        else
            printf("Service LogType changed to %d.\n", logtype);
        }

    if (stdctrl >= 0)
        {
        dwControl = stdctrl;

        if (!ControlService(hService, dwControl, &ServiceStatus))
            {
            printf("error on ControlService %d\n", GetLastError());

            return 1;
            }
        else
            printf("Service control %d issued.\n", stdctrl);
        }

    return 0;

    } // end main()


//-------------------------------- END --------------------------------------

