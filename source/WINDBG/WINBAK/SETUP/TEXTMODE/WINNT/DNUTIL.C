/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    dnutil.c

Abstract:

    Miscellaneous routines for DOS-hosted NT setup program.

Author:

    Ted Miller (tedm) 30-March-2000

Revision History:

--*/

#include "winnt.h"
#include <string.h>
#include <process.h>
#include <malloc.h>
#include <dos.h>
#include <direct.h>
#if DBG
#include <conio.h>
#endif


BOOLEAN WarnedAboutSkip = FALSE;

ULONG
DnGetKey(
    VOID
    )

/*++

Routine Description:

    Waits for any keypress.

Arguments:

    None.

Return Value:

    Actual key pressed.

--*/

{
    USHORT c;

    _asm {
        mov ah,0            // function: read char from keyboard
        int 16h
        mov c,ax
    }

    switch(c) {
    case 0x5000:                    // down arrow
        return(DN_KEY_DOWN);
    case 0x4800:                    // up arrow
        return(DN_KEY_UP);
    case 0x4700:                    // home
        return(DN_KEY_HOME);
    case 0x4f00:                    // end
        return(DN_KEY_END);
    case 0x4900:                    // page up
        return(DN_KEY_PAGEUP);
    case 0x5100:                    // page down
        return(DN_KEY_PAGEDOWN);
    case 0x3b00:                    // F1
        return(DN_KEY_F1);
    case 0x3d00:                    // F3
        return(DN_KEY_F3);
    default:
        return((ULONG)(c & ((c & 0x00ff) ? 0x00ff : 0xffff)));
    }
}


ULONG
DnGetValidKey(
    IN PULONG ValidKeyList
    )

/*++

Routine Description:

    Waits for a keypress matching one of the values given in a list.
    The list must be terminated with a 0 entry.

Arguments:

    ValidKeyList - valid keys.

Return Value:

    Actual key pressed.

--*/

{
    ULONG key;
    int i;

    while(1) {
        key = DnGetKey();
        for(i=0; ValidKeyList[i]; i++) {
            if(key == ValidKeyList[i]) {
                return(key);
            }
        }
    }
}


VOID
vDnDisplayScreen(
    IN PSCREEN Screen,
    IN va_list arglist
    )

/*++

Routine Description:

    Displays a screen.

Arguments:

    Screen - supplies pointer to structure describing screen to display.

    arglist - supplies list of arguments for printf-style formatting.

Return Value:

    None.

--*/

{
    UCHAR y;
    PCHAR p;
    PCHAR CurrentLine;
    int i;
    static CHAR FormatString[1600],FormattedString[1600];

    //
    // Take each line in the screen and put in into a buffer, to form
    // one large string.  Place newlines at the end of each string.
    //
    for(FormatString[0]=0,i=0; Screen->Strings[i]; i++) {
        if(strlen(FormatString)+strlen(Screen->Strings[i])+2 < sizeof(FormatString)) {
            strcat(FormatString,Screen->Strings[i]);
            strcat(FormatString,"\n");
        } else {
            break;
        }
    }

    //
    // Format the string using given arguments.
    //
    vsprintf(FormattedString,FormatString,arglist);

    for(y=Screen->Y,CurrentLine=FormattedString; CurrentLine && *CurrentLine; y++) {

        if(p = strchr(CurrentLine,'\n')) {
            *p = 0;
        }

        DnPositionCursor(Screen->X,y);
        DnWriteString(CurrentLine);

        CurrentLine = p ? p+1 : NULL;
    }
}

VOID
DnDisplayScreen(
    IN PSCREEN Screen,
    ...
    )

/*++

Routine Description:

    Displays a screen.

Arguments:

    Screen - supplies pointer to structure describing screen to display.

Return Value:

    None.

--*/

{
    va_list arglist;

    va_start(arglist,Screen);
    vDnDisplayScreen(Screen,arglist);
    va_end(arglist);
}



VOID
DnFatalError(
    IN PSCREEN Screen,
    ...
    )

/*++

Routine Description:

    Displays a fatal error screen and prompts the user to press enter
    to exit.  DOES NOT RETURN.

Arguments:

    Screen - supplies pointer to structure describing screen to display.

Return Value:

    DOES NOT RETURN.

--*/

{
    ULONG ExitOnlyKeyList[2] = { ASCI_CR,0 };
    va_list arglist;
    int i;

    DnClearClientArea();
    DnWriteStatusText(DntEnterEqualsExit);

    va_start(arglist,Screen);
    vDnDisplayScreen(Screen,arglist);
    va_end(arglist);

    for(i=0; Screen->Strings[i]; i++);
    DnPositionCursor(Screen->X,(UCHAR)(Screen->Y + i + 1));
    DnWriteString(DntPressEnterToExit);

    DnGetValidKey(ExitOnlyKeyList);
    DnExit(1);
}


BOOLEAN
DnCopyError(
    IN PCHAR   Filename,
    IN PSCREEN ErrorScreen,
    IN int     FilenameLine
    )

/*++

Routine Description:

    Displays a screen informing the user that there has been an error copying
    a file, and allows the options of continuing or exiting Setup.

Arguments:

    Filename - supplies name of source file which could not be copied.

    ErrorScreen - supplies the text to label the error.

    FilenameLine - supplies line number on the ErrorScreen in which the
        filename should be displayed.

Return Value:

    TRUE if user elects to retry; FALSE if user elects to continue;
    does not return if user chooses to exit.

--*/

{
    ULONG KeyList[4] = { ASCI_CR,DN_KEY_F3,ASCI_ESC,0 };
    ULONG KeyList2[4] = { 0,0,ASCI_CR,0 };

    KeyList2[0] = DniAccelSkip1;
    KeyList2[1] = DniAccelSkip2;

    DnClearClientArea();
    DnWriteStatusText("%s   %s   %s",DntEnterEqualsRetry,DntEscEqualsSkipFile,DntF3EqualsExit);

    ErrorScreen->Strings[FilenameLine] = Filename;
    DnDisplayScreen(ErrorScreen);

    while(1) {
        switch(DnGetValidKey(KeyList)) {

        case DN_KEY_F3:

            DnExitDialog();
            break;

        case ASCI_CR:

            return(TRUE);   // retry

        case ASCI_ESC:

            if(!WarnedAboutSkip) {

                DnClearClientArea();
                DnDisplayScreen(&DnsSureSkipFile);
                DnWriteStatusText("%s   %s",DntEnterEqualsRetry,DntXEqualsSkipFile);

                if(DnGetValidKey(KeyList2) == ASCI_CR) {
                    //
                    // retry
                    //
                    return(TRUE);
                } else {
                    //
                    // User elected to skip: prevent future warnings.
                    //
                    WarnedAboutSkip = TRUE;
                }
            }

            return(FALSE);  // skip file
        }
    }
}


PCHAR
DnDupString(
    IN PCHAR String
    )

/*++

Routine Description:

    Duplicate a string.  Do not return if not enough memory.

Arguments:

    String - string to be duplicated

Return Value:

    Pointer to new string. Does not return if insufficient memory.

--*/

{
    PCHAR p = MALLOC(strlen(String)+1);

    return(strcpy(p,String));
}



VOID
DnGetString(
    IN OUT PCHAR String,
    IN UCHAR X,
    IN UCHAR Y,
    IN UCHAR W
    )

/*++

Routine Description:

    Allow the user to type a string in an edit field.  Interpret F3
    to allow him to exit.

Arguments:

    String - on input, supplies the default string.  On output, contains
        the string entered by the user.

    X,Y - coords of leftmost char of edit field.

    W - width of edit field, and maximum length of the string.

Return Value:

    None.

--*/

{
    ULONG key;
    int Position = strlen(String);

    DnStartEditField(TRUE,X,Y,W);

    DnPositionCursor(X,Y);
    DnWriteString(String);
    DnPositionCursor((UCHAR)(X+Position),Y);

    while(1) {

        key = DnGetKey();

        switch(key) {

        case DN_KEY_F3:
            DnExitDialog();
            break;

        case ASCI_BS:
            if(Position) {
                String[--Position] = 0;
                DnPositionCursor((UCHAR)(X+Position),Y);
                DnWriteChar(' ');
            }
            break;

        case ASCI_ESC:
            Position = 0;
            String[0] = 0;
            DnStartEditField(TRUE,X,Y,W);       // blanks edit field
            DnPositionCursor(X,Y);
            break;

        case ASCI_CR:
            DnStartEditField(FALSE,X,Y,W);
            return;

        default:
            if(((UCHAR)Position < W) && !(key & 0xffffff00)) {
                DnWriteChar((CHAR)key);
                String[Position++] = (CHAR)key;
                String[Position] = 0;
                DnPositionCursor((UCHAR)(X+Position),Y);
            }
        }
    }
}


BOOLEAN
DnIsDriveValid(
    IN unsigned Drive
    )

/*++

Routine Description:

    Determine whether a drive is valid (ie, exists and is accessible).

Arguments:

    Drive - drive (1=A, 2=B, etc).

Return Value:

    TRUE if drive is valid.
    FALSE if not.

--*/

{
    int CurrentDrive = _getdrive();
    int Status;

    //
    // We'll make the determination of whether the drive is valid by
    // attempting to switch to it.  If this succeeds, assume the drive
    // is valid.
    //
    Status = _chdrive(Drive);

    _chdrive(CurrentDrive);

    return((BOOLEAN)(Status == 0));
}


BOOLEAN
DnIsDriveRemote(
    IN unsigned Drive
    )

/*++

Routine Description:

    Determine whether a drive is remote.

Arguments:

    Drive - drive (1=A, 2=B, etc).

Return Value:

    TRUE if drive is remote.
    FALSE if not (or we couldn't determine whether the drive is remote).

--*/

{
    union REGS RegIn,RegOut;

    //
    // Call IOCTL function 09.
    //

    RegIn.x.ax = 0x4409;
    RegIn.h.bl = (unsigned char)Drive;

    intdos(&RegIn,&RegOut);

    //
    // If carry set (error), assume not remote.
    // If no error, bit 12 of dx set if remote.
    //
    return((BOOLEAN)(!RegOut.x.cflag && (RegOut.x.dx & 0x1000)));
}


BOOLEAN
DnIsDriveRemovable(
    IN unsigned Drive
    )

/*++

Routine Description:

    Determine whether a drive is removable.

Arguments:

    Drive - drive (1=A, 2=B, etc).

Return Value:

    TRUE if drive is removable.
    FALSE if not removable.

    If an error occurs making the determination, the drive is assumed
    not removable.

--*/

{
    int ax;
    union REGS RegIn,RegOut;

    //
    // Call IOCTL function 08.
    //

    RegIn.x.ax = 0x4408;
    RegIn.h.bl = (unsigned char)Drive;

    ax = intdos(&RegIn,&RegOut);

    //
    // If an error occured, assume not removable.
    // If no error, ax = 0 if removable, ax = 1 if not removable.
    //
    return((BOOLEAN)(!RegOut.x.cflag && !ax));
}



#if DBG
long allocated;
long allocs;
#define MEMSIG 0xa3f8
#define callerinfo()    printf("      -- Caller: %s, line %u\n",file,line)
#endif

PVOID
Malloc(
    IN unsigned Size
#if DBG
   ,IN char *file,
    IN int line
#endif
    )

/*++

Routine Description:

    Allocates memory and fatal errors if none is available.

Arguments:

    Size - number of bytes to allocate

Return Value:

    Pointer to memory.  DOES NOT RETURN if no memory is available.

--*/

{
    unsigned *p;

#if DBG
    p = malloc(Size+(2*sizeof(unsigned)));

    if(p == NULL) {
        DnFatalError(&DnsOutOfMemory);
    }

    *p++ = Size;

    *(unsigned *)(((PCHAR)p)+Size) = MEMSIG;

    allocated += Size;
    allocs++;
#else
    if((p = malloc(Size)) == NULL) {
        DnFatalError(&DnsOutOfMemory);
    }
#endif
    return(p);
}



VOID
Free(
    IN PVOID Block
#if DBG
   ,IN char *file,
    IN int line
#endif
    )

/*++

Routine Description:

    Free a block of memory previously allocated with Malloc().

Arguments:

    Block - supplies pointer to block to free.

Return Value:

    None.

--*/

{
#if DBG
    unsigned *p;

    if(!allocs) {
        printf("Free: allocation count going negative!\n");
        callerinfo();
        _asm { int 3 }
    }
    allocs--;

    p = ((unsigned *)Block) - 1;

    allocated -= *p;

    if(allocated < 0) {
        printf("Free: total allocation going negative!\n");
        callerinfo();
        _asm { int 3 }
    }

    if(*(unsigned *)((PCHAR)Block+(*p)) != MEMSIG) {
        printf("Free: memory block lacks MEMSIG!\n");
        callerinfo();
        _asm { int 3 }
    }

    free(p);
#else
    free(Block);
#endif
}


PVOID
Realloc(
    IN PVOID Block,
    IN unsigned Size
#if DBG
   ,IN char *file,
    IN int line
#endif
    )

/*++

Routine Description:

    Reallocates a block of memory previously allocated with Malloc();
    fatal errors if none is available.

Arguments:

    Block - supplies pointer to block to resize

    Size - number of bytes to allocate

Return Value:

    Pointer to memory.  DOES NOT RETURN if no memory is available.

--*/

{
    PVOID p;
#if DBG
    unsigned BlockSize;

    BlockSize = ((unsigned *)Block)[-1];
    allocated -= BlockSize;
    allocated += Size;

    if(*(unsigned *)((PCHAR)Block + BlockSize) != MEMSIG) {
        printf("Realloc: memory block lacks MEMSIG!\n");
        callerinfo();
        _asm { int 3 }
    }

    p = realloc((unsigned *)Block - 1,Size + (2*sizeof(unsigned)));

    if(p == NULL) {
        DnFatalError(&DnsOutOfMemory);
    }

    *(unsigned *)p = Size;
    (unsigned *)p += 1;

    *(unsigned *)((PCHAR)p + Size) = MEMSIG;
#else
    if((p = realloc(Block,Size)) == NULL) {
        DnFatalError(&DnsOutOfMemory);
    }
#endif
    return(p);
}


VOID
DnExit(
    IN int ExitStatus
    )

/*++

Routine Description:

    Exits back to DOS in an orderly fashion.

Arguments:

    ExitStatus - supplies value to be passed to exit()

Return Value:

    None.  Does not return.

--*/

{
    unsigned DriveCount;

    //
    // Do a video mode switch to clear the screen.
    //

    _asm {
        mov ax,3
        int 10h
    }

    // restore current drive
    _dos_setdrive(DngOriginalCurrentDrive,&DriveCount);

    exit(ExitStatus);
}


BOOLEAN
DnWriteSmallIniFile(
    IN  PCHAR  Filename,
    IN  PCHAR *Lines,
    OUT FILE  **FileHandle OPTIONAL
    )
{
    FILE *fileHandle;
    unsigned i,len;
    BOOLEAN rc;

    //
    // If the file is already there, change attributes to normal
    // so we can overwrite it.
    //
    _dos_setfileattr(Filename,_A_NORMAL);

    //
    // Open/truncate the file.
    //
    fileHandle = fopen(Filename,"wt");
    if(fileHandle == NULL) {
        return(FALSE);
    }

    //
    // Assume success.
    //
    rc = TRUE;

    //
    // Write lines into the file indicating that this is
    // a winnt setup. On a doublespaced floppy, there should
    // be room for a single sector outside the CVF.
    //
    for(i=0; Lines[i]; i++) {

        len = strlen(Lines[i]);

        if(fwrite(Lines[i],1,len,fileHandle) != len) {
            rc = FALSE;
            break;
        }
    }

    //
    // Leave the file open if the caller wants the handle.
    //
    if(rc && FileHandle) {
        *FileHandle = fileHandle;
    } else {
        fclose(fileHandle);
    }

    return(rc);
}
