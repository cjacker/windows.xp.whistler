/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    smss.c

Abstract:


Author:

    Mark Lucovsky (markl) 04-Oct-2000

Revision History:

--*/

#include "smsrvp.h"

EXCEPTION_DISPOSITION
SmpUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );

void
_CRTAPI1 main(
    int argc,
    char *argv[],
    char *envp[],
    ULONG DebugParameter OPTIONAL
    )
{
    NTSTATUS Status;
    KPRIORITY SetBasePriority;
    UNICODE_STRING InitialCommand, DebugInitialCommand, UnicodeParameter;
    HANDLE ProcessHandles[ 2 ];
    ULONG Parameters[ 4 ];
    ULONG Response;
    PROCESS_BASIC_INFORMATION ProcessInfo;
    BOOLEAN WasEnabled;

    SetBasePriority = FOREGROUND_BASE_PRIORITY+2;
    Status = NtSetInformationProcess( NtCurrentProcess(),
                                      ProcessBasePriority,
                                      (PVOID) &SetBasePriority,
                                       sizeof( SetBasePriority )
                                    );
    ASSERT(NT_SUCCESS(Status));

    if (ARGUMENT_PRESENT( DebugParameter )) {
        SmpDebug = DebugParameter;
        }

    try {
        Status = SmpInit( &InitialCommand, &ProcessHandles[ 0 ] );
        if (!NT_SUCCESS( Status )) {
            KdPrint(( "SMSS: SmpInit return failure - Status == %x\n" ));
            RtlInitUnicodeString( &UnicodeParameter, L"Session Manager Initialization" );
            Parameters[ 1 ] = (ULONG)Status;
            }
        else {
#if DEVL
            SYSTEM_FLAGS_INFORMATION FlagInfo;

            NtQuerySystemInformation( SystemFlagsInformation,
                                      &FlagInfo,
                                      sizeof( FlagInfo ),
                                      NULL
                                    );
            if (FlagInfo.Flags & FLG_DEBUG_INITIAL_COMMAND) {
                DebugInitialCommand.MaximumLength = InitialCommand.Length + 64;
                DebugInitialCommand.Length = 0;
                DebugInitialCommand.Buffer = RtlAllocateHeap( RtlProcessHeap(),
                                                              0,
                                                              DebugInitialCommand.MaximumLength
                                                            );
		RtlAppendUnicodeToString( &DebugInitialCommand, L"ntsd -p -1 -d " );
                RtlAppendUnicodeStringToString( &DebugInitialCommand, &InitialCommand );
                InitialCommand = DebugInitialCommand;
                }

#endif
            Status = SmpExecuteInitialCommand( &InitialCommand, &ProcessHandles[ 1 ] );
            if (NT_SUCCESS( Status )) {
                Status = NtWaitForMultipleObjects( 2,
                                                   ProcessHandles,
                                                   WaitAny,
                                                   FALSE,
                                                   NULL
                                                 );
                }

            if (Status == STATUS_WAIT_0) {
                RtlInitUnicodeString( &UnicodeParameter, L"Windows SubSystem" );
                Status = NtQueryInformationProcess( ProcessHandles[ 0 ],
                                                    ProcessBasicInformation,
                                                    &ProcessInfo,
                                                    sizeof( ProcessInfo ),
                                                    NULL
                                                  );

                KdPrint(( "SMSS: Windows subsystem terminated when it wasn't supposed to.\n" ));
                }
            else {
                RtlInitUnicodeString( &UnicodeParameter, L"Windows Logon Process" );
                if (Status == STATUS_WAIT_1) {
                    Status = NtQueryInformationProcess( ProcessHandles[ 1 ],
                                                        ProcessBasicInformation,
                                                        &ProcessInfo,
                                                        sizeof( ProcessInfo ),
                                                        NULL
                                                      );
                    }
                else {
                    ProcessInfo.ExitStatus = Status;
                    Status = STATUS_SUCCESS;
                    }

                KdPrint(( "SMSS: Initial command '%wZ' terminated when it wasn't supposed to.\n", &InitialCommand ));
                }

            if (NT_SUCCESS( Status )) {
                Parameters[ 1 ] = (ULONG)ProcessInfo.ExitStatus;
                }
            else {
                Parameters[ 1 ] = (ULONG)STATUS_UNSUCCESSFUL;
                }
            }
        }
    except( SmpUnhandledExceptionFilter( GetExceptionInformation() ) ) {
        RtlInitUnicodeString( &UnicodeParameter, L"Unhandled Exception in Session Manager" );
        Parameters[ 1 ] = (ULONG)GetExceptionCode();
        }

    //
    // We are hosed, so raise a fata system error to shutdown the system.
    // (Basically a user mode KeBugCheck).
    //

    Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                 (BOOLEAN)TRUE,
                                 TRUE,
                                 &WasEnabled
                               );

    if (Status == STATUS_NO_TOKEN) {

        //
        // No thread token, use the process token
        //

        Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                     (BOOLEAN)TRUE,
                                     FALSE,
                                     &WasEnabled
                                   );
        }

    Parameters[ 0 ] = (ULONG)&UnicodeParameter;

    Status = NtRaiseHardError( STATUS_SYSTEM_PROCESS_TERMINATED,
                               2,
                               1,
                               Parameters,
                               OptionShutdownSystem,
                               &Response
                             );

    //
    // If this returns, giveup
    //

    NtTerminateProcess( NtCurrentProcess(), Status );
}


EXCEPTION_DISPOSITION
SmpUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    NTSTATUS Status;
    ULONG Parameters[ 4 ];
    ULONG Response;

#if DEVL
    DbgPrint( "SMSS: Unhandled exception - Status == %x  IP == %x\n",
              ExceptionInfo->ExceptionRecord->ExceptionCode,
              ExceptionInfo->ExceptionRecord->ExceptionAddress
            );
    DbgPrint( "      Memory Address: %x  Read/Write: %x\n",
              ExceptionInfo->ExceptionRecord->ExceptionInformation[ 0 ],
              ExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ]
            );

    DbgBreakPoint();
#endif

    Parameters[ 0 ] = (ULONG)ExceptionInfo->ExceptionRecord->ExceptionCode;
    Parameters[ 1 ] = (ULONG)ExceptionInfo->ExceptionRecord->ExceptionAddress;
    if ( ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_IN_PAGE_ERROR ) {
        Parameters[ 2 ] = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 2 ];
        }
    else {
        Parameters[ 2 ] = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 0 ];
        }

    Parameters[ 3 ] = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ];


    Status = NtRaiseHardError( STATUS_UNHANDLED_EXCEPTION,
                               4,
                               1,
                               Parameters,
                               OptionShutdownSystem,
                               &Response
                             );

    return EXCEPTION_EXECUTE_HANDLER;
}
