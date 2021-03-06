/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    dnflop.c

Abstract:

    Floppy-disk creation routines for 32-bit winnt setup.

Author:

    Ted Miller (tedm) 19-December-1993

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop

#ifdef _X86_

#include "msg.h"
#include <stdlib.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

//
// This header file contains an array of 512 bytes
// representing the NT FAT boot code, in a variable
// of type unsigned char[] called FatBootCode.
//
#include <bootfat.h>

#pragma pack(1)

//
// Define bpb structure.
//
typedef struct _MY_BPB {
    USHORT BytesPerSector;
    UCHAR  SectorsPerCluster;
    USHORT ReservedSectors;
    UCHAR  FatCount;
    USHORT RootDirectoryEntries;
    USHORT SectorCountSmall;
    UCHAR  MediaDescriptor;
    USHORT SectorsPerFat;
    USHORT SectorsPerTrack;
    USHORT HeadCount;
} MY_BPB, *PMY_BPB;

#pragma pack()

//
// Marco for accessing unaligned ushorts in the above structure.
// Not strictly needed on x86 but if this code is ever portable
// then this is necessary.
//
#define U_USHORT(x) (*(USHORT UNALIGNED *)&(x))

// Macro to align a buffer.
//
#define ALIGN(p,val)                                        \
                                                            \
    (PVOID)((((ULONG)(p) + (val) - 1)) & (~((val) - 1)))


BOOL
MyGetBpb(
    OUT PMY_BPB Bpb,
    OUT PBOOL   DriveBusy
    )

/*++

Routine Description:

    Return the BPB for the disk in drive A:.  The BPB is read directly
    from the disk.

Arguments:

    Bpb - receives the BPB.

Return Value:

    Boolean value indicating whether the bpb was successfully retreived
    and copied into the caller-supplied buffer.

--*/

{
    PVOID UnalignedBuffer;
    PUCHAR Buffer;
    HANDLE Handle;
    BOOL b;
    DWORD BytesRead;

    //
    // This value is only used to allocate a buffer
    // to read sector 0 of the floppy in a:.
    //
    DWORD BytesPerSector = 512;

    //
    // Open A:
    //
    Handle = CreateFile(
                TEXT("\\\\.\\A:"),
                GENERIC_READ,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if(Handle == INVALID_HANDLE_VALUE) {

        DWORD err = GetLastError();

        *DriveBusy = ((err == ERROR_SHARING_VIOLATION) || (err == ERROR_ACCESS_DENIED));
        return(FALSE);
    }

    //
    // Allocate an aligned buffer.
    //
    UnalignedBuffer = MALLOC(2*BytesPerSector);
    Buffer = ALIGN(UnalignedBuffer,BytesPerSector);

    if(b = ReadFile(Handle,Buffer,BytesPerSector,&BytesRead,NULL)) {

        if((Buffer[0]   == 0xeb)
        && (Buffer[2]   == 0x90)
        && (Buffer[510] == 0x55)
        && (Buffer[511] == 0xaa))
        {
            //
            // Transfer the bpb into the caller's buffer.
            //
            CopyMemory(Bpb,Buffer+11,sizeof(MY_BPB));

        } else {
            //
            // Bogus signature, probably not DOS disk.
            //
            b = FALSE;
        }
    }

    //
    // Clean up.
    //
    FREE(UnalignedBuffer);
    CloseHandle(Handle);

    return(b);
}


BOOL
MyWriteBootSector(
    IN PUCHAR Buffer
    )

/*++

Routine Description:

    Write a bufferful of data to the bootsector of the disk in drive a.

Arguments:

    Buffer - supplies the boot sector to be written.  This buffer should
        be 512 bytes long.

Return Value:

    Boolean value indicating whether the boot sector was successfully
    written.

--*/

{
    PVOID UBuffer,ABuffer;
    HANDLE Handle;
    BOOL rc;
    DWORD BytesWritten;
    //
    // This value is only used to allocate a buffer
    // to read sector 0 of the floppy in a:.
    //
    DWORD BytesPerSector = 512;

    //
    // Open A:
    //
    Handle = CreateFile(
                TEXT("\\\\.\\A:"),
                GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_WRITE_THROUGH,
                NULL
                );

    if(Handle == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    UBuffer = MALLOC(2*BytesPerSector);
    ABuffer = ALIGN(UBuffer,BytesPerSector);

    CopyMemory(ABuffer,Buffer,BytesPerSector);

    rc = WriteFile(Handle,ABuffer,BytesPerSector,&BytesWritten,NULL);

    FREE(UBuffer);
    CloseHandle(Handle);
    return(rc);
}


BOOL
DnPromptAndInspectFloppy(
    IN  DWORD   FirstPromptId,
    IN  DWORD   SubsequentPromptId,
    OUT PMY_BPB Bpb,
    IN  HWND    hdlg,
    IN  PTSTR   FloppyName,
    IN  PTSTR   Floppy0Name,
    IN  PTSTR   Floppy1Name,
    IN  PTSTR   Floppy2Name
    )

/*++

Routine Description:

    Prompt the user to insert a floppy disk in drive a:, and inspect
    the disk to make sure it appears to be a formatted MS-DOS compatible
    floppy disk.

Arguments:

    FirstPromptId - supplies the message to be used in prompting the
        user to insert the disk.

    SubsequentPromptId - supplies the message to be used to prompt the
        user for a disk if the first disk he inserts is not acceptable.

Return Value:

    FALSE if the user elects to exit setup.  TRUE if setup should continue.

--*/

{
    DWORD PromptId,ErrorId;
    DWORD FreeSpace;
    DWORD spc,bps,freeclus,totclus;
    DWORD ec;
    BOOL DriveBusy;
    int rc;

    PromptId = FirstPromptId;

    do {

        do {
            rc = UiMessageBox(
                    hdlg,
                    PromptId,
                    AppTitleStringId,
                    MB_OKCANCEL | MB_ICONASTERISK,
                    FloppyName,
                    Floppy0Name,
                    Floppy1Name,
                    Floppy2Name
                    );

            if(rc == IDCANCEL) {

                ec = UiMessageBox(
                        hdlg,
                        MSG_SURE_EXIT,
                        AppTitleStringId,
                        MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION
                        );

                if(ec == IDYES) {
                    return(FALSE);
                }
            }
        } while(rc == IDCANCEL);

        PromptId = SubsequentPromptId;
        ErrorId = 0;

        //
        // Get the BPB for the disk in the drive.
        //
        if(!MyGetBpb(Bpb,&DriveBusy)) {

            // Unable to get the current BPB for the disk.  Assume
            // Assume not formatted or not formatted correctly.
            //
            ErrorId = DriveBusy ? MSG_FLOPPY_BUSY : MSG_FLOPPY_NOT_FORMATTED;

        } else {
            //
            // Sanity check on the BPB
            //
            if((U_USHORT(Bpb->BytesPerSector) != 512)
            || (   (Bpb->SectorsPerCluster != 1)
                && (Bpb->SectorsPerCluster != 2))       // for 2.88M disks
            || (U_USHORT(Bpb->ReservedSectors) != 1)
            || (Bpb->FatCount != 2)
            || !U_USHORT(Bpb->SectorCountSmall)         // should be < 32M
            || (Bpb->MediaDescriptor == 0xf8)           // hard disk
            || (U_USHORT(Bpb->HeadCount) != 2)
            || !U_USHORT(Bpb->RootDirectoryEntries)) {

                ErrorId = MSG_FLOPPY_BAD_FORMAT;
            } else {

                if(GetDiskFreeSpace(TEXT("A:\\"),&spc,&bps,&freeclus,&totclus)) {

                    FreeSpace = freeclus * spc * bps;

                    //
                    // The constant is what format reports for a 1.2 meg disk.
                    //
                    if(FreeSpace < FLOPPY_CAPACITY_525) {
                        ErrorId = MSG_FLOPPY_NOT_BLANK;
                    }
                } else {
                    ErrorId = MSG_FLOPPY_CANT_GET_SPACE;
                }
            }
        }

        //
        // If there is a problem with the disk, inform the user.
        //
        if(ErrorId) {

            UiMessageBox(
                hdlg,
                ErrorId,
                IDS_ERROR,
                MB_OK | MB_ICONEXCLAMATION
                );
        }
    } while(ErrorId);

    return(TRUE);
}



BOOL
DoCreateBootFloppies(
    IN HWND  hdlg,
    IN PTSTR Floppy0Name,
    IN PTSTR Floppy1Name,
    IN PTSTR Floppy2Name
    )

/*++

Routine Description:

    Create the winnt boot floppies by copying files listed in three
    sections of dosnet.inf to 3 separate floppies, and preparing one
    of the floppies to boot the setup boot loader.

    If we are in floppyless operation, we will instead copy the files
    into a special directory on the system partition (usually C:).

    This routine is expecting to be the entry point of a separate
    thread to perform this action.

Arguments:

    hdlg - supplies handle to dialog currently displayed.

Return Value:

    FALSE if the program should end (critical error).
    TRUE otherwise.

--*/


{
    int i;
    DWORD ErrorId;
    UCHAR SectorBuffer[512];
    TCHAR TextBuffer[1024];
    MY_BPB Bpb;
    BOOL b;
    BOOL rc = TRUE;
    DWORD ec;
    TCHAR BootRoot[64];
    PTSTR System32Dir;

    if(FloppylessOperation) {

        BootRoot[0] = SystemPartitionDrive;
        BootRoot[1] = ':';
        strcpy(BootRoot+2,FloppylessBootDirectory);

        //
        // Create the boot dir. First get rid of any file by that name.
        //
        SetFileAttributes(BootRoot,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(BootRoot);
        CreateDirectory(BootRoot,NULL);

        RetreiveAndFormatMessageIntoBuffer(
            MSG_PREPARING_FLOPPYLESS,
            TextBuffer,
            SIZECHARS(TextBuffer)
            );

    } else {

        RetreiveAndFormatMessageIntoBuffer(
            MSG_PREPARING_FLOPPY,
            TextBuffer,
            SIZECHARS(TextBuffer),
            Floppy2Name
            );

        lstrcpy(BootRoot,TEXT("A:"));
    }

    System32Dir = MALLOC((lstrlen(BootRoot)*sizeof(TCHAR))+sizeof(TEXT("\\SYSTEM32")));

    lstrcpy(System32Dir,BootRoot);
    lstrcat(System32Dir,TEXT("\\SYSTEM32"));

    //
    // Get a floppy in the drive -- this will be "Windows NT Setup Disk #3"
    //
    if(FloppylessOperation) {
        b = TRUE;
    } else {
        b = DnPromptAndInspectFloppy(
                MSG_FIRST_FLOPPY_PROMPT,
                MSG_GENERIC_FLOPPY_PROMPT,
                &Bpb,
                hdlg,
                Floppy2Name,
                Floppy0Name,
                Floppy1Name,
                Floppy2Name
                );
    }

    if(!b || bCancelled) {
        rc = FALSE;
        goto flop1;
    }

    AuxillaryStatus(hdlg,TextBuffer);

    //
    // Copy files to the disk.
    //
    ec = CopySectionOfFilesToCopy(
            hdlg,
            INF_FLOPPYFILES2,
            BootRoot,
            TRUE,
            0,
            FALSE
            );

    if(ec == (DWORD)(-1) || bCancelled) {
        AuxillaryStatus(hdlg,NULL);
        rc = FALSE;
        goto flop1;
    }

    if(!FloppylessOperation) {
        RetreiveAndFormatMessageIntoBuffer(
            MSG_PREPARING_FLOPPY,
            TextBuffer,
            SIZECHARS(TextBuffer),
            Floppy1Name
            );
    }

    do {

        //
        // Get a floppy in the drive -- this will be "Windows NT Setup Disk #2"
        //
        if(FloppylessOperation) {
            b = TRUE;
        } else {
            b = DnPromptAndInspectFloppy(
                    MSG_GENERIC_FLOPPY_PROMPT,
                    MSG_GENERIC_FLOPPY_PROMPT,
                    &Bpb,
                    hdlg,
                    Floppy1Name,
                    Floppy0Name,
                    Floppy1Name,
                    Floppy2Name
                    );
        }

        if(!b || bCancelled) {
            rc = FALSE;
            goto flop1;
        }

        AuxillaryStatus(hdlg,TextBuffer);

        //
        // Hack: create system32 directory on the floppy.
        //
        SetFileAttributes(System32Dir,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(System32Dir);
        CreateDirectory(System32Dir,NULL);

        //
        // Copy files to the disk.
        //
        ec = CopySectionOfFilesToCopy(
                hdlg,
                INF_FLOPPYFILES1,
                BootRoot,
                TRUE,
                0,
                FALSE
                );

        if(ec == (DWORD)(-1) || bCancelled) {
            AuxillaryStatus(hdlg,NULL);
            rc = FALSE;
            goto flop1;
        }

        //
        // Put a small file on the disk indicating that it's a winnt setup.
        //
        b = DnIndicateWinnt(hdlg,BootRoot,NULL,NULL);
        AuxillaryStatus(hdlg,NULL);
        if(!b) {
            UiMessageBox(
                hdlg,
                MSG_CANT_WRITE_FLOPPY,
                IDS_ERROR,
                MB_OK | MB_ICONEXCLAMATION
                );
        }

    } while(!b);

    if(!FloppylessOperation) {
        RetreiveAndFormatMessageIntoBuffer(
            MSG_PREPARING_FLOPPY,
            TextBuffer,
            SIZECHARS(TextBuffer),
            Floppy0Name
            );
    }

    do {

        ErrorId = 0;

        //
        // Get a floppy in the drive -- this will be "Windows NT Setup Boot Disk"
        //
        if(FloppylessOperation) {
            b = TRUE;
        } else {
            b = DnPromptAndInspectFloppy(
                    MSG_GENERIC_FLOPPY_PROMPT,
                    MSG_GENERIC_FLOPPY_PROMPT,
                    &Bpb,
                    hdlg,
                    Floppy0Name,
                    Floppy0Name,
                    Floppy1Name,
                    Floppy2Name
                    );
        }

        if(!b || bCancelled) {
            rc = FALSE;
            goto flop1;
        }

        AuxillaryStatus(hdlg,TextBuffer);

        if(!FloppylessOperation) {

            CopyMemory(SectorBuffer,FatBootCode,512);

            //
            // Copy the BPB we retreived for the disk into the bootcode template.
            // We only care about the original BPB fields, through the head count
            // field.  We will fill in the other fields ourselves.
            //
            strncpy(SectorBuffer+3,"MSDOS5.0",8);
            CopyMemory(SectorBuffer+11,&Bpb,sizeof(MY_BPB));

            //
            // Set up other fields in the bootsector/BPB/xBPB.
            //
            // Large sector count (4 bytes)
            // Hidden sector count (4 bytes)
            // current head (1 byte, not necessary to set this, but what the heck)
            // physical disk# (1 byte)
            //
            ZeroMemory(SectorBuffer+28,10);

            //
            // Extended BPB signature
            //
            SectorBuffer[38] = 41;

            //
            // Serial number
            //
            srand((unsigned)clock());
            *(DWORD UNALIGNED *)(SectorBuffer+39) = (((DWORD)clock() * (DWORD)rand()) << 8) | rand();

            //
            // volume label/system id
            //
            strncpy(SectorBuffer+43,"NO NAME    ",11);
            strncpy(SectorBuffer+54,"FAT12   ",8);

            //
            // Overwrite the 'ntldr' string with 'setupldr.bin' so the right file gets
            // loaded when the floppy is booted.
            //
            for(i=499; i>0; --i) {
                if(!memcmp("NTLDR      ",SectorBuffer+i,11)) {
                    strncpy(SectorBuffer+i,"SETUPLDRBIN",11);
                    break;
                }
            }
        }

        //
        // Write the boot sector.
        //
        if(FloppylessOperation || MyWriteBootSector(SectorBuffer)) {

            //
            // Copy files into the disk.
            //
            ec = CopySectionOfFilesToCopy(
                    hdlg,
                    INF_FLOPPYFILES0,
                    BootRoot,
                    TRUE,
                    0,
                    FALSE
                    );

            AuxillaryStatus(hdlg,NULL);

            if(ec == (DWORD)(-1) || bCancelled) {
                rc = FALSE;
                goto flop1;
            }

        } else {
            AuxillaryStatus(hdlg,NULL);
            ErrorId = MSG_FLOPPY_WRITE_BS;
        }

        if(ErrorId) {

            UiMessageBox(
                hdlg,
                ErrorId,
                IDS_ERROR,
                MB_OK | MB_ICONEXCLAMATION
                );
        }

    } while(ErrorId);

    //
    // Additionally in the floppyless case, we need to copy some files
    // from the boot directory to the root of the system partition drive.
    //
    if(FloppylessOperation && rc) {

        AuxillaryStatus(hdlg,TextBuffer);

        ec = CopySectionOfFilesToCopy(
                hdlg,
                INF_FLOPPYFILESX,
                BootRoot,
                TRUE,
                0,
                FALSE
                );

        AuxillaryStatus(hdlg,NULL);

        if(ec == (DWORD)(-1) || bCancelled) {
            rc = FALSE;
            goto flop1;
        }

        AuxillaryStatus(hdlg,TextBuffer);

        System32Dir[0] = BootRoot[0];
        System32Dir[1] = TEXT(':');
        System32Dir[2] = 0;

        rc = DnCopyFilesInSection(hdlg,INF_ROOTBOOTFILES,BootRoot,System32Dir);

        AuxillaryStatus(hdlg,NULL);

        if(rc && !bCancelled) {

            AuxillaryStatus(hdlg,TextBuffer);

            if(rc = DnMungeBootIni(hdlg)) {
                rc = DnLayAuxBootSector(hdlg);
            }

            AuxillaryStatus(hdlg,NULL);
        }
    }

flop1:
    FREE(System32Dir);
    return(rc);
}


DWORD
ThreadAuxilliaryAction(
    IN PVOID ThreadParameter
    )
{
    HWND hdlg;
    BOOL b;
    PTSTR Floppy0Name,Floppy1Name;
    PTSTR Floppy2Name;

    hdlg = (HWND)ThreadParameter;

    try {

        if(CreateFloppies || FloppylessOperation) {

            Floppy0Name = MyLoadString(ServerProduct ? IDS_SFLOPPY0_NAME : IDS_WFLOPPY0_NAME);
            Floppy1Name = MyLoadString(ServerProduct ? IDS_SFLOPPY1_NAME : IDS_WFLOPPY1_NAME);
            Floppy2Name = MyLoadString(ServerProduct ? IDS_SFLOPPY2_NAME : IDS_WFLOPPY2_NAME);

            b = DoCreateBootFloppies(
                    hdlg,
                    Floppy0Name,
                    Floppy1Name,
                    Floppy2Name
                    );

            FREE(Floppy0Name);
            FREE(Floppy1Name);
            FREE(Floppy2Name);

            if(!b) {
                PostMessage(hdlg, WM_COMMAND, IDCANCEL, 0);
            }

        } else {
            b = TRUE;
        }

        PostMessage(hdlg,WMX_AUXILLIARY_ACTION_DONE,b,0);

    } except(EXCEPTION_EXECUTE_HANDLER) {

        MessageBoxFromMessage(
            hdlg,
            MSG_GENERIC_EXCEPTION,
            AppTitleStringId,
            MB_ICONSTOP | MB_OK | MB_SETFOREGROUND,
            GetExceptionCode()
            );

        b = FALSE;
        PostMessage(hdlg,WMX_AUXILLIARY_ACTION_DONE,b,0);
    }

    ExitThread(b);
    return(b);
}

UINT
DlgProcSysPartSpaceWarn(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(msg) {

    case WM_INITDIALOG:
    {
        TCHAR Buffer[4096];
        PTSTR p;

        p = MyLoadString(AppTitleStringId);
        SetWindowText(hdlg,p);
        FREE(p);

        //
        //
        // Center the dialog on the screen and bring it to the top.
        //
        CenterDialog(hdlg);
        SetForegroundWindow(hdlg);
        MessageBeep(MB_ICONQUESTION);

        //
        // Set the icon to be the warning exclamation
        //
        SendDlgItemMessage(
            hdlg,
            IDC_EXCLAIM,
            STM_SETICON,
            (WPARAM)LoadIcon(NULL, IDI_EXCLAMATION),
            0
            );

        RetreiveAndFormatMessageIntoBuffer(
            MSG_SYSPART_LOW_X86,
            Buffer,
            SIZECHARS(Buffer),
            SystemPartitionDrive
            );

        SetDlgItemText(hdlg,IDC_TEXT1,Buffer);

        return(FALSE);
    }

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDOK:
        case IDCANCEL:
            EndDialog(hdlg, (UINT)LOWORD(wParam));
            break;

        case ID_HELP:

            MyWinHelp(hdlg, IDD_SYSPART_LOW_X86);
            break;

        default:
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

#endif //def _X86_
