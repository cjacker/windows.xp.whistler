/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    Regqval.c

Abstract:

    This module contains the client side wrappers for the Win32 Registry
    query value APIs.  That is:

        - RegQueryValueA
        - RegQueryValueW
        - RegQueryValueExA
        - RegQueryValueExW

Author:

    David J. Gilman (davegi) 18-Mar-2000

Notes:

    See the notes in server\regqval.c.

--*/

#include <rpc.h>
#include "regrpc.h"
#include "client.h"

LONG
RegQueryValueA (
    HKEY hKey,
    LPCSTR lpSubKey,
    LPSTR lpData,
    PLONG lpcbData
    )

/*++

Routine Description:


    Win 3.1 ANSI RPC wrapper for querying a value.

--*/

{
    HKEY            ChildKey;
    LONG            Error;
    DWORD           ValueType;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    //
    // Limit the capabilities associated with HKEY_PERFORMANCE_DATA.
    //

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    hKey = MapPredefinedHandle( hKey );
    // ASSERT( hKey != NULL );
    if( hKey == NULL ) {
        return ERROR_INVALID_HANDLE;
    }

    //
    // If the sub-key is NULL or points to an empty string then the value is
    // to be queried from this key (i.e.  hKey) otherwise the sub-key needs
    // to be opened.
    //

    if(( lpSubKey == NULL ) || ( *lpSubKey == '\0' )) {

        ChildKey = hKey;

    } else {

        //
        // The sub-key was supplied so impersonate the
        // client and attempt to open it.
        //

        Error = RegOpenKeyExA(
                    hKey,
                    lpSubKey,
                    0,
                    KEY_QUERY_VALUE,
                    &ChildKey
                    );

        if( Error != ERROR_SUCCESS ) {
            return Error;
        }
    }

    //
    // ChildKey contains an HKEY which may be the one supplied (hKey) or
    // returned from RegOpenKeyExA. Query the value using the special value
    // name NULL.
    //

    Error = RegQueryValueExA(
                ChildKey,
                NULL,
                NULL,
                &ValueType,
                lpData,
                lpcbData
                );
    //
    // If the sub key was opened, close it.
    //

    if( ChildKey != hKey ) {

        if( IsLocalHandle( ChildKey )) {

            LocalBaseRegCloseKey( &ChildKey );

        } else {

            ChildKey = DereferenceRemoteHandle( ChildKey );
            BaseRegCloseKey( &ChildKey );
        }
    }

    //
    // If the type of the value is not a null terminate string, then return
    // an error. (Win 3.1 compatibility)
    //

    if (!Error && (ValueType != REG_SZ)) {
        Error = ERROR_INVALID_DATA;
    }

    //
    // If value doesn't exist, return ERROR_SUCCESS and an empty string.
    // (Win 3.1 compatibility)
    //
    if( Error == ERROR_FILE_NOT_FOUND ) {
        if( ARGUMENT_PRESENT( lpcbData ) ) {
            *lpcbData = sizeof( CHAR );
        }
        if( ARGUMENT_PRESENT( lpData ) ) {
            *lpData = '\0';
        }
        Error = ERROR_SUCCESS;
    }

    //
    // Return the results of querying the value.
    //

    return Error;

}

LONG
RegQueryValueW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    LPWSTR lpData,
    PLONG  lpcbData
    )

/*++

Routine Description:

    Win 3.1 Unicode RPC wrapper for querying a value.

--*/

{
    HKEY        ChildKey;
    LONG        Error;
    DWORD       ValueType;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    //
    // Limit the capabilities associated with HKEY_PERFORMANCE_DATA.
    //

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    hKey = MapPredefinedHandle( hKey );
    // ASSERT( hKey != NULL );
    if( hKey == NULL ) {
        return ERROR_INVALID_HANDLE;
    }


    //
    // If the sub-key is NULL or points to an empty string then the value is
    // to be queried from this key (i.e.  hKey) otherwise the sub-key needs
    // to be opened.
    //

    if(( lpSubKey == NULL ) || ( *lpSubKey == '\0' )) {

        ChildKey = hKey;

    } else {

        //
        // The sub-key was supplied so attempt to open it.
        //

        Error = RegOpenKeyExW(
                    hKey,
                    lpSubKey,
                    0,
                    KEY_QUERY_VALUE,
                    &ChildKey
                    );

        if( Error != ERROR_SUCCESS ) {
            return Error;
        }
    }

    //
    // ChildKey contains an HKEY which may be the one supplied (hKey) or
    // returned from RegOpenKeyExA. Query the value using the special value
    // name NULL.
    //

    Error = RegQueryValueExW(
                ChildKey,
                NULL,
                NULL,
                &ValueType,
                ( LPBYTE )lpData,
                lpcbData
                );

    //
    // If the sub key was opened, close it.
    //

    if( ChildKey != hKey ) {

        if( IsLocalHandle( ChildKey )) {

            LocalBaseRegCloseKey( &ChildKey );

        } else {

            ChildKey = DereferenceRemoteHandle( ChildKey );
            BaseRegCloseKey( &ChildKey );
        }
    }

    //
    // If the type of the value is not a null terminate string, then return
    // an error. (Win 3.1 compatibility)
    //

    if (ValueType != REG_SZ) {
        Error = ERROR_INVALID_DATA;
    }

    //
    // If value doesn't exist, return ERROR_SUCCESS and an empty string.
    // (Win 3.1 compatibility)
    //
    if( Error == ERROR_FILE_NOT_FOUND ) {
        if( ARGUMENT_PRESENT( lpcbData ) ) {
            *lpcbData = sizeof( WCHAR );
        }
        if( ARGUMENT_PRESENT( lpData ) ) {
            *lpData = ( WCHAR )'\0';
        }
        Error = ERROR_SUCCESS;
    }

    //
    // Return the results of querying the value.
    //

    return Error;
}


LONG
APIENTRY
RegQueryValueExA (
    HKEY hKey,
    LPSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpdwType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )

/*++

Routine Description:

    Win32 ANSI RPC wrapper for querying a value.

    RegQueryValueExA converts the lpValueName argument to a counted Unicode
    string and then calls BaseRegQueryValue.

--*/

{
    PUNICODE_STRING     ValueName;
    DWORD               ValueType;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    LONG                Error;
    DWORD               ValueLength;
    DWORD               InputLength;
    PWSTR               UnicodeValueBuffer;
    ULONG               UnicodeValueLength;

    PSTR                AnsiValueBuffer;
    ULONG               AnsiValueLength;
    ULONG               Index;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    //
    // Validate dependency between lpData and lpcbData parameters.
    //

    if( ARGUMENT_PRESENT( lpReserved ) ||
        (ARGUMENT_PRESENT( lpData ) && ( ! ARGUMENT_PRESENT( lpcbData )))) {
        return ERROR_INVALID_PARAMETER;
    }


    hKey = MapPredefinedHandle( hKey );
    // ASSERT( hKey != NULL );
    if( hKey == NULL ) {
        return ERROR_INVALID_HANDLE;
    }

    //
    // Convert the value name to a counted Unicode string using the static
    // Unicode string in the TEB.
    //

    ValueName = &NtCurrentTeb( )->StaticUnicodeString;
    ASSERT( ValueName != NULL );
    RtlInitAnsiString( &AnsiString, lpValueName );
    Status = RtlAnsiStringToUnicodeString(
                ValueName,
                &AnsiString,
                FALSE
                );

    if( ! NT_SUCCESS( Status )) {
        return RtlNtStatusToDosError( Status );
    }

    //
    //  Add the terminating NULL to the Length so that RPC transmits
    //  it.
    //

    ValueName->Length += sizeof( UNICODE_NULL );

    //
    // Call the Base API, passing it the supplied parameters and the
    // counted Unicode strings. Note that zero bytes are transmitted (i.e.
    // InputLength = 0) for the data.
    //

    ValueLength = ARGUMENT_PRESENT( lpcbData )? *lpcbData : 0;
    InputLength = 0;

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegQueryValue (
                             hKey,
                             ValueName,
                             &ValueType,
                             lpData,
                             &ValueLength,
                             &InputLength
                             );
        //
        //  Make sure that the local side didn't destroy the Buffer in
        //  the StaticUnicodeString
        //
        ASSERT( (&NtCurrentTeb( )->StaticUnicodeString)->Buffer );


    } else {

        Error = (LONG)BaseRegQueryValue (
                             DereferenceRemoteHandle( hKey ),
                             ValueName,
                             &ValueType,
                             lpData,
                             &ValueLength,
                             &InputLength
                             );
    }

    //
    // If no error or callers buffer too small, and type is one of the null
    // terminated string types, then do the UNICODE to ANSI translation.
    // We handle the buffer too small case, because the callers buffer may
    // be big enough for the ANSI representation, but not the UNICODE one.
    // In this case, we need to allocate a buffer big enough, do the query
    // again and then the translation into the callers buffer.  We only do
    // this if the caller actually wants the value data (lpData != NULL)
    //

    if ((Error == ERROR_SUCCESS || Error == ERROR_MORE_DATA) &&
        ARGUMENT_PRESENT( lpcbData ) &&
        (ValueType == REG_SZ ||
         ValueType == REG_EXPAND_SZ ||
         ValueType == REG_MULTI_SZ)
       ) {

        if( ARGUMENT_PRESENT( lpData ) ) {
            UnicodeValueBuffer         = (PWSTR)lpData;
            UnicodeValueLength         = ValueLength;

            AnsiValueBuffer            = lpData;
            AnsiValueLength            = ARGUMENT_PRESENT( lpcbData )?
                                                         *lpcbData : 0;

            if ( ( Error == ERROR_MORE_DATA ) &&
                 (ValueLength / sizeof( WCHAR )) <= *lpcbData ) {

                //
                // Here if the callers buffer is big enough for the ANSI
                // representation, but not the UNICODE one.  Allocate a
                // buffer for the UNICODE value and reissue the query.
                //

                UnicodeValueBuffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                                      UnicodeValueLength
                                                     );
                if (UnicodeValueBuffer == NULL) {
                    Error = ERROR_NOT_ENOUGH_MEMORY;
                } else {
                    InputLength = 0;

                    if( IsLocalHandle( hKey )) {

                        //
                        //  Add the terminating NULL to the Length
                        //  (remember that in the local case, ValueName->Length
                        //  was decremented by sizeof( UNICODE_NULL ) in the first
                        //  call to LocalBaseRegQueryValue).
                        //  This won't happen in the remote case, since the
                        //  server side will decrement ValueName->Length on
                        //  the transmitted structure (a copy of ValueName), and
                        //  the new Valuename->Length won't be transmitted back to
                        //  the client.
                        //

                        ValueName->Length += sizeof( UNICODE_NULL );


                        Error = (LONG)LocalBaseRegQueryValue (
                                             hKey,
                                             ValueName,
                                             &ValueType,
                                             (LPBYTE)UnicodeValueBuffer,
                                             &ValueLength,
                                             &InputLength
                                             );
                        //
                        //  Make sure that the local side didn't destroy the
                        //  Buffer in the StaticUnicodeString
                        //

                        ASSERT((&NtCurrentTeb()->StaticUnicodeString)->Buffer);


                    } else {

                        Error = (LONG)BaseRegQueryValue (
                                             DereferenceRemoteHandle( hKey ),
                                             ValueName,
                                             &ValueType,
                                             (LPBYTE)UnicodeValueBuffer,
                                             &ValueLength,
                                             &InputLength
                                             );
                    }
                }
            }

            if( ( Error == ERROR_SUCCESS ) &&
                ( UnicodeValueBuffer == ( LPWSTR )lpData ) &&
                ( ( ( DWORD )lpData & 0x1 ) != 0 )
              ) {

                //
                //  Here if the caller's buffer was big enough for the UNICODE
                //  representation of the data in the registry.
                //  In this case we have to allocate another buffer, and copy the
                //  contents of lpData to this new buffer.
                //  This is necessary since the caller's buffer is a PBYTE
                //  (not necessarily aligned), and RtlUnicodeToMultiByteN()
                //  expects a PWSTR as source buffer (an aligned buffer).
                //  If we don't allocate this new buffer, we may have alignment
                //  problems on MIPS or ALPHA.
                //

                UnicodeValueBuffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                                      UnicodeValueLength
                                                    );
                if (UnicodeValueBuffer == NULL) {
                    Error = ERROR_NOT_ENOUGH_MEMORY;
                } else {
                    RtlMoveMemory( UnicodeValueBuffer,
                                   lpData,
                                   UnicodeValueLength );
                }
            }

            if (Error == ERROR_SUCCESS) {
                //
                // We have a UNICODE value, so translate it to ANSI in the callers
                // buffer.  In the case where the caller's buffer was big enough
                // for the UNICODE version, we do the conversion in place, which
                // works since the ANSI version is smaller than the UNICODE version.
                //


                Index = 0;
                Status = RtlUnicodeToMultiByteN( AnsiValueBuffer,
                                                 AnsiValueLength,
                                                 &Index,
                                                 UnicodeValueBuffer,
                                                 UnicodeValueLength
                                                );
                if (!NT_SUCCESS( Status )) {
                    Error = RtlNtStatusToDosError( Status );
                }
            }

            if (UnicodeValueBuffer != (PWSTR)lpData) {
                //
                // Here if we had to allocate a buffer for the UNICODE Value,
                // so free it.
                //

                RtlFreeHeap( RtlProcessHeap(), 0, UnicodeValueBuffer );
            }
        }
        //
        // Return the length of the ANSI version to the caller.
        //
        ValueLength = ValueLength /= sizeof( WCHAR );
    }

    //
    // Stored the returned length in the caller specified location and
    // return the error code.
    //

    if (lpdwType != NULL) {
        *lpdwType = ValueType;
    }

    if( ARGUMENT_PRESENT( lpcbData ) ) {
        *lpcbData = ValueLength;
    }
    return Error;
}


LONG
APIENTRY
RegQueryValueExW (
    HKEY hKey,
    LPWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpdwType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )

/*++

Routine Description:

    Win32 Unicode RPC wrapper for querying a value.

    RegQueryValueExW converts the lpValueName argument to a counted Unicode
    string and then calls BaseRegQueryValue.

--*/

{
    UNICODE_STRING      ValueName;
    DWORD               InputLength;
    DWORD               ValueLength;
    LONG                Status;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    //
    // Validate dependency between lpData and lpcbData parameters.
    //

    if( ARGUMENT_PRESENT( lpReserved ) ||
        (ARGUMENT_PRESENT( lpData ) && ( ! ARGUMENT_PRESENT( lpcbData )))) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey );
    // ASSERT( hKey != NULL );
    if( hKey == NULL ) {
        return ERROR_INVALID_HANDLE;
    }

    //
    // Convert the value name to a counted Unicode string.
    //

    RtlInitUnicodeString( &ValueName, lpValueName );

    //
    //  Add the terminating NULL to the Length so that RPC transmits
    //  it.
    //

    ValueName.Length += sizeof( UNICODE_NULL );

    //
    // Call the Base API, passing it the supplied parameters and the
    // counted Unicode strings. Note that zero bytes are transmitted (i.e.
    // InputLength = 0) for the data.
    //
    InputLength = 0;
    ValueLength = ( ARGUMENT_PRESENT( lpcbData ) )? *lpcbData : 0;

    if( IsLocalHandle( hKey )) {

        Status = (LONG)LocalBaseRegQueryValue (
                            hKey,
                            &ValueName,
                            lpdwType,
                            lpData,
                            &ValueLength,
                            &InputLength
                            );
    } else {

        Status =  (LONG)BaseRegQueryValue (
                            DereferenceRemoteHandle( hKey ),
                            &ValueName,
                            lpdwType,
                            lpData,
                            &ValueLength,
                            &InputLength
                            );
    }
    if( ARGUMENT_PRESENT( lpcbData ) ) {
        *lpcbData = ValueLength;
    }
    return( Status );
}
