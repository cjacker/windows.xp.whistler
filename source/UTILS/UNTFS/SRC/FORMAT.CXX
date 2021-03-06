/*++

Copyright (c) 2000	Microsoft Corporation

Module Name:

	format.cxx

Abstract:

    This module contains the definition of NTFS_SA::Create,
    which performs FORMAT for an NTFS volume.

Author:

	Bill McJohn (billmc) 15-Aug-2000

Environment:

    ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UNTFS_MEMBER_

#include "ulib.hxx"
#include "error.hxx"
#include "untfs.hxx"
#include "..\..\ufat\inc\fatsa.hxx" // for PHYS_REMOVABLE and PHYS_FIXED ;

#include "string.hxx"
#include "wstring.hxx"
#include "numset.hxx"
#include "numset.hxx"

#include "ifssys.hxx"

#include "ntfssa.hxx"
#include "attrib.hxx"
#include "frs.hxx"
#include "mftfile.hxx"
#include "mftref.hxx"
#include "ntfsbit.hxx"
#include "attrdef.hxx"
#include "badfile.hxx"
#include "bootfile.hxx"
#include "bitfrs.hxx"
#include "indxtree.hxx"
#include "upcase.hxx"
#include "upfile.hxx"
#include "logfile.hxx"

#include "rtmsg.h"
#include "message.hxx"

extern "C" {

#include "bootntfs.h"
}


WCHAR FileNameMft[] = {'$', 'M', 'F', 'T', 0};
WCHAR FileNameMftRef[] = {'$', 'M', 'F', 'T', 'M', 'i', 'r', 'r', 0 };
WCHAR FileNameLogFile[] = {'$', 'L', 'o', 'g', 'F', 'i', 'l', 'e', 0 };
WCHAR FileNameDasd[] = {'$', 'V', 'o', 'l', 'u', 'm', 'e', 0 };
WCHAR FileNameAttrDef[] = {'$', 'A', 't', 't', 'r', 'D', 'e', 'f', 0 };
WCHAR FileNameRootIndex[] = {'.', 0 };
WCHAR FileNameBitmap[] = {'$', 'B', 'i', 't', 'm', 'a', 'p', 0 };
WCHAR FileNameBootFile[] = {'$', 'B', 'o', 'o', 't', 0 };
WCHAR FileNameBadFile[] = {'$', 'B', 'a', 'd', 'C', 'l', 'u', 's', 0 };
WCHAR FileNameQuota[] = {'$', 'Q', 'u', 'o', 't', 'a', 0 };

WCHAR FileNameUpcase[] = { '$', 'U', 'p', 'C', 'a', 's', 'e', 0 };


// Buffer for FILE_NAME structure

CONST FileNameBufferSize = 256;
CHAR FileNameBuffer[ FileNameBufferSize ];
CONST PFILE_NAME FileName = (PFILE_NAME)(FileNameBuffer);


UNTFS_EXPORT
ULONG
NTFS_SA::QuerySectorsInElementaryStructures(
    IN  PCDP_DRIVE  Drive,
    IN  ULONG       ClusterFactor,
    IN  ULONG       ClustersPerFrs,
    IN  ULONG       ClustersPerIndexBuffer,
    IN  ULONG       LogFileSize
    )
/*++

Routine Description:

    This method computes the number of sectors required for
    the elementary structures of an NTFS volume.

Arguments:

    Drive                   --  Supplies the drive under consideration.
    ClusterFactor           --  Supplies the number of sectors per
                                cluster.  May be zero, in which case
                                a default value is supplied.
    ClustersPerFrs          --  Supplies the number of clusters per
                                NTFS File Record Segment.  May be zero,
                                in which case a default value is supplied.
    ClustersPerIndexBuffer  --  Supplies the number of clusters per NTFS
                                index allocation buffer.  May be zero,
                                in which case a default value is supplied.
    LogFileSize             --  Supplies the size of the log file.  May
                                be zero, in which case a default value
                                is supplied.

Return Value:

    Returns the number of sectors required by an NTFS volume on
    this drive with the specified parameters.  Returns zero if
    it is unable to compute this figure.

--*/
{
    ULONG SectorsOnVolume, SectorsRequired, SectorSize;


    if( Drive->QuerySectors().GetHighPart() != 0 ) {

        return 0;
    }

    SectorsOnVolume = Drive->QuerySectors().GetLowPart();

    SectorSize = Drive->QuerySectorSize();

    if( SectorSize == 0 ) {

        return 0;
    }

    // compute defaults.
    //
    if( ClusterFactor == 0 ) {

        ClusterFactor = NTFS_SA::QueryDefaultClusterFactor( Drive );
    }

    if( ClustersPerFrs == 0 ) {

        ClustersPerFrs = QueryDefaultClustersPerFrs( Drive, ClusterFactor );
    }

    if( ClustersPerIndexBuffer == 0 ) {

        ClustersPerIndexBuffer = QueryDefaultClustersPerIndexBuffer( Drive, ClusterFactor);
    }

    if( LogFileSize == 0 ) {

        LogFileSize = NTFS_LOG_FILE::QueryDefaultSize( Drive, Drive->QuerySectors() );
    }

    // Now add up the various elementary structures:
    //
    // MFT
    //
    SectorsRequired = FIRST_USER_FILE_NUMBER * ClustersPerFrs * ClusterFactor;

    // MFT Mirror
    //
    SectorsRequired += REFLECTED_MFT_SEGMENTS * ClustersPerFrs * ClusterFactor;

    // Log file
    //
    SectorsRequired += LogFileSize/SectorSize + 1;

    // Attribute Definition Table
    //
    SectorsRequired += NTFS_ATTRIBUTE_DEFINITION_TABLE::QueryDefaultSize()/SectorSize + 1;

    // Bitmap
    //
    SectorsRequired += ((SectorsOnVolume / ClusterFactor ) / 8)/SectorSize + 1;

    // Boot file
    //
    SectorsRequired += BYTES_IN_BOOT_AREA/SectorSize;

    // Upcase Table
    //
    SectorsRequired += NTFS_UPCASE_TABLE::QueryDefaultSize()/SectorSize;

    // The Volume DASD file, the Bad Cluster file, and the Quota
    // Table don't take up any extra space.

    return SectorsRequired;
}



UNTFS_EXPORT
BOOLEAN
NTFS_SA::WriteRemainingBootCode(
    )
/*++

Routine Description:

    This method writes the remainder of the boot code, ie. the
    portion that is not stored in the first sector (which is
    written when the superarea itself is written).

Arguments:

    None.

Return Value:

    TRUE upon successful completion.

--*/
{
    HMEM BootCodeMem;
    SECRUN BootCodeSecrun;
    ULONG SectorsInBootArea, SectorSize;

    SectorSize = _drive->QuerySectorSize();

    SectorsInBootArea = ( BYTES_IN_BOOT_AREA % SectorSize ) ?
                            ( BYTES_IN_BOOT_AREA / SectorSize + 1 ) :
                            ( BYTES_IN_BOOT_AREA / SectorSize );

    if( !BootCodeMem.Initialize() ||
        !BootCodeSecrun.Initialize( &BootCodeMem,
                                    _drive,
                                    1,
                                    SectorsInBootArea - 1 ) ) {

        return FALSE;
    }

    memcpy( BootCodeSecrun.GetBuf(),
            (PUCHAR)NtfsBootCode + SectorSize,
            sizeof( NtfsBootCode ) - SectorSize );

    if( !BootCodeSecrun.Write( ) ) {

        return FALSE;
    }

    SetSystemId();

    return TRUE;
}


UNTFS_EXPORT
BOOLEAN
NTFS_SA::CreateElementaryStructures(
    IN OUT  PNTFS_BITMAP            VolumeBitmap,
    IN      ULONG                   ClusterFactor,
    IN      ULONG                   ClustersPerFrs,
    IN      ULONG                   ClustersPerIndexBuffer,
    IN      ULONG                   InitialLogFileSize,
    IN      PCNUMBER_SET            BadSectors,
    IN OUT  PMESSAGE                Message,
    IN      PBIOS_PARAMETER_BLOCK   OldBpb,
    IN      PCWSTRING               Label
    )
/*++

Routine Description:

    This method creates the system-defined files on the volume.  Note
    that it does not write the superarea (ie. the boot sectors).

Arguments:

    VolumeBitmap            --  Supplies the bitmap for the volume.
    ClusterFactor           --  Supplies the number of sectors per cluster.
    ClustersPerFrs          --  Supplies the number of clusters per FRS.
    ClustersPerIndexBuffer  --  Supplies the volume default number of
                                clusters per index allocation buffer.
    InitialLogFileSize      --  Supplies the initial size of the log file.
                                If zero is given for this parameter, this
                                method will choose a default size based on
                                the size of the volume.
    BadSectors              --  Supplies a list of the bad sectors on the disk.
    Message                 --  Supplies an outlet for messages.
    OldBpb                  --  Supplies a pointer to the volume's existing
                                Bios Parameter Block.  If this parameter
                                is present, then the disk geometry information
                                (Sectors per Track, Heads, and HiddenSectors)
                                are copied from it; otherwise, they are
                                queried from the drive.
    Label                   --  Supplies an optional volume label (may be NULL)

Return Value:

    TRUE upon successful completion.

Notes:

    The supplied Bitmap is updated and written to disk.

--*/
{
    NUMBER_SET BadClusters;
    NTFS_MFT_FILE MftFile;
    NTFS_REFLECTED_MASTER_FILE_TABLE MftReflection;
    NTFS_ATTRIBUTE_DEFINITION_TABLE AttributeDefinitionTable;
    NTFS_BAD_CLUSTER_FILE BadClusterFile;
    NTFS_BITMAP_FILE BitmapFile;
    NTFS_BOOT_FILE BootFile;
    NTFS_LOG_FILE LogFile;
    NTFS_FILE_RECORD_SEGMENT RootIndexFile;
    NTFS_FILE_RECORD_SEGMENT QuotaFile;
    NTFS_FILE_RECORD_SEGMENT VolumeDasdFile;
    NTFS_FILE_RECORD_SEGMENT GenericFrs;
    NTFS_INDEX_TREE RootIndex;
    NTFS_ATTRIBUTE BitmapAttribute;
    NTFS_ATTRIBUTE VolumeInformationAttribute;
    DSTRING RootIndexName;
    NTFS_UPCASE_FILE UpcaseFile;
    NTFS_UPCASE_TABLE UpcaseTable;

    MFT_SEGMENT_REFERENCE RootFileIndexSegment;
    STANDARD_INFORMATION StandardInformation;
    VOLUME_INFORMATION VolumeInformation;
    LARGE_INTEGER SystemTime;
    LCN MftLcn, Lcn;
    ULONG NumberOfSectors, i,
            ClusterSize, ClustersInBootArea;
    PWSTR LabelString;
    ULONG NumBootClusters;

    CONST PVOID FileNameValue = NtfsFileNameGetName( FileName );


    // Determine the size of the volume:
    //
    if (_drive->QuerySectors().GetHighPart() != 0) {

        DbgAbort("Number of sectors exceeds 32 bits");
        return FALSE;
    }

    NumberOfSectors = _drive->QuerySectors().GetLowPart();

    // Set up the Standard Information structure that will
    // be used by all the special files.  The creation and modification
    // times are now, and all files created by format are hidden.

    memset( (PVOID)&StandardInformation,
            0,
            sizeof(STANDARD_INFORMATION) );

    IFS_SYSTEM::QueryNtfsTime( &SystemTime );


    StandardInformation.CreationTime =
        StandardInformation.LastModificationTime =
        StandardInformation.LastChangeTime =
        StandardInformation.LastAccessTime = SystemTime;

    StandardInformation.FileAttributes = FILE_ATTRIBUTE_HIDDEN |
                                         FILE_ATTRIBUTE_SYSTEM;

    // Get the default system upcase table
    //
    if( !UpcaseTable.Initialize() ) {

        DbgAbort( "Can't initialize upcase table.\n" );
        return FALSE;
    }

    // Clear the boot block and the backup boot cluster in the
    // bitmap.  Note that these will get set immediately after
    // the bad sectors are marked as in use--this allows me to
    // detect if these sectors are on the bad sector list.
    //
    ClusterSize = ClusterFactor * _drive->QuerySectorSize();

    ClustersInBootArea = (BYTES_IN_BOOT_AREA % ClusterSize) ?
                            BYTES_IN_BOOT_AREA / ClusterSize + 1 :
                            BYTES_IN_BOOT_AREA / ClusterSize;

    NumBootClusters = max(1, BYTES_PER_BOOT_SECTOR/ClusterSize);

    VolumeBitmap->SetFree( 0, ClustersInBootArea );
    VolumeBitmap->SetFree( (NumberOfSectors/2)/ClusterFactor, NumBootClusters );


    // Convert the Bad Sectors to Bad Clusters, and mark those
    // clusters as in-use in the bitmap.  Note that we have to
    // check for duplicates, or else the Bad Cluster File will
    // choke on the list.

    if( !BadClusters.Initialize() ) {

        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );

        DbgPrint( "Can't initialize bad clusters numset.\n" );
        return FALSE;
    }

    for( i = 0; i < BadSectors->QueryCardinality(); i++ ) {

        Lcn = BadSectors->QueryNumber(i)/ClusterFactor;

        BadClusters.Add( Lcn );
        VolumeBitmap->SetAllocated( Lcn, 1 );
    }


    // The first BYTES_IN_BOOT_AREA bytes on the volume and
    // the cluster which contains the middle sector of the volume
    // are reserved for the Boot file.  If these sectors are not
    // free, it means that we have bad sectors in one of these
    // reserved spots.  We won't allow such a volume to be formatted
    // to NTFS.

    if( !VolumeBitmap->IsFree( 0, ClustersInBootArea ) ||
        !VolumeBitmap->IsFree( (NumberOfSectors/2)/ClusterFactor, 1 ) ) {

        DbgPrint( "Boot sector is in bad cluster list.\n" );
        return FALSE;
    }

    VolumeBitmap->SetAllocated( 0, ClustersInBootArea );
    VolumeBitmap->SetAllocated( (NumberOfSectors/2)/ClusterFactor,
                                NumBootClusters );


    // Allocate space for the MFT itself

    if( !VolumeBitmap->AllocateClusters( 0,
                                        FIRST_USER_FILE_NUMBER *
                                            ClustersPerFrs,
                                        &MftLcn ) ) {

        DbgPrint( "Can't allocate space for the MFT.\n" );
        return FALSE;
    }

    // Another bit of housecleaning:  I need the file segment reference
    // of the root file name index so I can add file-name attributes
    // to the system files.  The initial sequence number of the Root
    // File Index FRS is the same as its file number.

    RootFileIndexSegment.LowPart = ROOT_FILE_NAME_INDEX_NUMBER;
    RootFileIndexSegment.HighPart = 0;
    RootFileIndexSegment.SequenceNumber = ROOT_FILE_NAME_INDEX_NUMBER;


    // Set up an index tree for the root file name index:

    if( !RootIndexName.Initialize( FileNameIndexNameData ) ||
        !RootIndex.Initialize( $FILE_NAME,
                               _drive,
                               ClusterFactor,
                               VolumeBitmap,
                               &UpcaseTable,
                               COLLATION_FILE_NAME,
                               ClustersPerIndexBuffer,
                               ClusterFactor * ClustersPerFrs *
                                    _drive->QuerySectorSize()/2,
                               &RootIndexName ) ) {

        DbgPrint( "Cannot initialize Index Tree for root file name index.\n" );
        return FALSE;
    }


    // Initialize and create the MFT.  Note that this will not
    // actually write the MFT to disk.

    // Set up the FILE_NAME attribute.

    memset( FileName,
            0,
            FileNameBufferSize );

    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameMft );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameMft,
            FileName->FileNameLength * sizeof( WCHAR ) );

    if( !MftFile.Initialize( _drive,
                             MftLcn,
                             ClusterFactor,
                             ClustersPerFrs,
                             _drive->QuerySectors(),
                             VolumeBitmap,
                             &UpcaseTable ) ||
        !MftFile.Create( FIRST_USER_FILE_NUMBER,
                         &StandardInformation,
                         VolumeBitmap ) ||
        !MftFile.AddFileNameAttribute( FileName ) ||
        !MftFile.AddSecurityDescriptor( ReadCannedSd, VolumeBitmap ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                MftFile.QuerySegmentReference() ) ) {

        DbgPrint( "Can't create MFT.\n" );
        return FALSE;
    }

    // Initialize, create, and write the reflection of the Master
    // File Table.  Note that this allocates space for the MFT
    // Reflection's data attribute, but does not write the data
    // attribute.

    // Set up the FILE_NAME attribute.

    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameMftRef );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameMftRef,
            FileName->FileNameLength * sizeof( WCHAR ) );

    if( !MftReflection.Initialize( MftFile.GetMasterFileTable() ) ||
        !MftReflection.Create( &StandardInformation,
                               VolumeBitmap ) ||
        !MftReflection.AddFileNameAttribute( FileName ) ||
        !MftReflection.AddSecurityDescriptor( ReadCannedSd,
                                              VolumeBitmap ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                MftReflection.QuerySegmentReference() ) ||
        !MftReflection.Flush(VolumeBitmap, &RootIndex) ) {

        DbgPrint( "Can't create MFT Reflection.\n" );
        return FALSE;
    }


    // Initialize, create, and write the log file.

    // Set up the FILE_NAME attribute.

    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameLogFile );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameLogFile,
            FileName->FileNameLength * sizeof( WCHAR ) );

    if( !LogFile.Initialize( MftFile.GetMasterFileTable() ) ||
        !LogFile.Create( &StandardInformation,
                         InitialLogFileSize,
                         VolumeBitmap ) ||
        !LogFile.AddFileNameAttribute( FileName ) ||
        !LogFile.AddSecurityDescriptor( ReadCannedSd, VolumeBitmap ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                LogFile.QuerySegmentReference() ) ||
        !LogFile.Flush(VolumeBitmap, &RootIndex) ) {

        DbgPrint( "Can't create Log File.\n" );
        return FALSE;
    }


    // Initialize, create, and write an empty file for the Volume DASD info.

    // Set up the FILE_NAME attribute.

    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameDasd );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameDasd,
            FileName->FileNameLength * sizeof( WCHAR ) );

    // Set up the volume information attribute.
    //
    memset(&VolumeInformation, 0, sizeof(VOLUME_INFORMATION));

    VolumeInformation.MajorVersion = NTFS_CURRENT_MAJOR_VERSION;
    VolumeInformation.MinorVersion = NTFS_CURRENT_MINOR_VERSION;
    VolumeInformation.VolumeFlags = 0;

    if (!VolumeInformationAttribute.Initialize(_drive, ClusterFactor,
            &VolumeInformation, sizeof(VOLUME_INFORMATION),
            $VOLUME_INFORMATION)) {

        DbgPrint( "Can't create volume information attribute.\n" );
        return FALSE;
    }

    if (Label) {
        LabelString = Label->QueryWSTR();
    } else {
        LabelString = NULL;
    }

    if( !VolumeDasdFile.Initialize( VOLUME_DASD_NUMBER,
                                    MftFile.GetMasterFileTable() ) ||
        !VolumeDasdFile.Create( &StandardInformation ) ||
        !VolumeDasdFile.AddFileNameAttribute( FileName ) ||
        !VolumeDasdFile.AddSecurityDescriptor( WriteCannedSd, VolumeBitmap ) ||
        !VolumeDasdFile.AddEmptyAttribute( $DATA ) ||
        !(LabelString == NULL ||
          VolumeDasdFile.AddAttribute( $VOLUME_NAME, NULL, LabelString,
                                       Label->QueryChCount()*sizeof(WCHAR),
                                       VolumeBitmap )) ||
        !VolumeInformationAttribute.InsertIntoFile( &VolumeDasdFile,
                                                    NULL ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                VolumeDasdFile.QuerySegmentReference() ) ||
        !VolumeDasdFile.Flush(VolumeBitmap, &RootIndex) ) {

        DbgPrint( "Can't create Volume DASD file.\n" );
        return FALSE;
    }

    DELETE(LabelString);


    // Initialize, create, and write the Attribute Definition Table
    // File Record Segment.  This will also allocate and write the
    // Attribute Definition Table's DATA attribute, which is the
    // actual attribute definition table.

    // Set up the FILE_NAME attribute.

    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameAttrDef );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameAttrDef,
            FileName->FileNameLength * sizeof( WCHAR ) );

    if( !AttributeDefinitionTable.Initialize( MftFile.GetMasterFileTable() ) ||
        !AttributeDefinitionTable.Create( &StandardInformation,
                                          VolumeBitmap ) ||
        !AttributeDefinitionTable.AddFileNameAttribute( FileName ) ||
        !AttributeDefinitionTable.AddSecurityDescriptor( ReadCannedSd,
                                                         VolumeBitmap ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                AttributeDefinitionTable.
                                            QuerySegmentReference() ) ||
        !AttributeDefinitionTable.Flush(VolumeBitmap, &RootIndex) ) {

        DbgPrint( "Can't create Attribute Definition Table.\n" );
        return FALSE;
    }


    // Initialize, create, and write the FRS for the root file name
    // index.

    // Set up the FILE_NAME attribute.

    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameRootIndex );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameRootIndex,
            FileName->FileNameLength * sizeof( WCHAR ) );

    if( !RootIndexFile.Initialize( ROOT_FILE_NAME_INDEX_NUMBER,
                                   MftFile.GetMasterFileTable() ) ||
        !RootIndexFile.Create( &StandardInformation,
                               FILE_FILE_NAME_INDEX_PRESENT ) ||
        !RootIndexFile.AddFileNameAttribute( FileName ) ||
        !RootIndexFile.AddSecurityDescriptor( NoAclCannedSd,
                                              VolumeBitmap ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                RootIndexFile.QuerySegmentReference() ) ||
        !RootIndexFile.Flush(VolumeBitmap, &RootIndex) ) {

        DbgPrint( "Can't create Root Index FRS.\n" );
        return FALSE;
    }


    // Initialize, create, and write the bitmap File Record Segment.
    // Note that this does not write the bitmap, just its File Record
    // Segment.  Note also that the disk space for the bitmap is
    // allocated at this time.

    // Set up the FILE_NAME attribute.

    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameBitmap );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameBitmap,
            FileName->FileNameLength * sizeof( WCHAR ) );

    if( !BitmapFile.Initialize( MftFile.GetMasterFileTable() ) ||
        !BitmapFile.Create( &StandardInformation, VolumeBitmap ) ||
        !BitmapFile.AddFileNameAttribute( FileName ) ||
        !BitmapFile.AddSecurityDescriptor( ReadCannedSd, VolumeBitmap ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                BitmapFile.QuerySegmentReference() ) ||
        !BitmapFile.Flush(VolumeBitmap, &RootIndex) ) {

        DbgPrint( "Can't create Bitmap File.\n" );
        return FALSE;
    }


    // Initialize, create, and write the Boot-File File Record Segment.

    // Set up the FILE_NAME attribute.

    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameBootFile );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameBootFile,
            FileName->FileNameLength * sizeof( WCHAR ) );

    if( !BootFile.Initialize( MftFile.GetMasterFileTable() ) ||
        !BootFile.Create( &StandardInformation ) ||
        !BootFile.AddFileNameAttribute( FileName ) ||
        !BootFile.AddSecurityDescriptor( ReadCannedSd, VolumeBitmap ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                BootFile.QuerySegmentReference() ) ||
        !BootFile.Flush(VolumeBitmap, &RootIndex) ) {

        DbgPrint( "Can't create boot file.\n" );
        return FALSE;
    }

    // Initialize, create, and write the Bad Cluster File.
    //
    // Set up the FILE_NAME attribute.
    //
    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameBadFile );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameBadFile,
            FileName->FileNameLength * sizeof( WCHAR ) );

    if( !BadClusterFile.Initialize( MftFile.GetMasterFileTable() ) ||
        !BadClusterFile.Create( &StandardInformation,
                                VolumeBitmap,
                                &BadClusters ) ||
        !BadClusterFile.AddFileNameAttribute( FileName ) ||
        !BadClusterFile.AddSecurityDescriptor( ReadCannedSd, VolumeBitmap ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                BadClusterFile.QuerySegmentReference() ) ||
        !BadClusterFile.Flush(VolumeBitmap, &RootIndex) ) {

        DbgPrint( "Can't create Bad Cluster File.\n" );
        return FALSE;
    }


    // Initialize, create, and write the Quota Table.

    // Set up the FILE_NAME attribute.

    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameQuota );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameQuota,
            FileName->FileNameLength * sizeof( WCHAR ) );

    if( !QuotaFile.Initialize( QUOTA_TABLE_NUMBER,
                               MftFile.GetMasterFileTable() ) ||
        !QuotaFile.Create( &StandardInformation ) ||
        !QuotaFile.AddEmptyAttribute( $DATA ) ||
        !QuotaFile.AddFileNameAttribute( FileName ) ||
        !QuotaFile.AddSecurityDescriptor( WriteCannedSd, VolumeBitmap ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                QuotaFile.QuerySegmentReference() ) ||
        !QuotaFile.Flush(VolumeBitmap, &RootIndex) ) {

        DbgPrint( "Can't create Quota Table File Record Segment.\n" );
        return FALSE;
    }

    // Create the Upcase Table file.
    //
    // Set up the FILE_NAME attribute.
    //
    FileName->ParentDirectory = RootFileIndexSegment;
    FileName->FileNameLength = wcslen( FileNameUpcase );
    FileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    memset( FileNameValue,
            0,
            FileNameBufferSize - sizeof( FILE_NAME ) );

    memcpy( FileNameValue,
            FileNameUpcase,
            FileName->FileNameLength * sizeof( WCHAR ) );

    if( !UpcaseFile.Initialize( MftFile.GetMasterFileTable() ) ||
        !UpcaseFile.Create( &StandardInformation,
                            &UpcaseTable,
                            VolumeBitmap ) ||
        !UpcaseFile.AddFileNameAttribute( FileName ) ||
        !UpcaseFile.AddSecurityDescriptor( ReadCannedSd, VolumeBitmap ) ||
        !RootIndex.InsertEntry( NtfsFileNameGetLength( FileName ),
                                FileName,
                                UpcaseFile.QuerySegmentReference() ) ||
        !UpcaseFile.Flush( VolumeBitmap, &RootIndex ) ) {

        DbgPrint( "Can't create Upcase Table File Record Segment.\n" );
        return FALSE;
    }



    // The reserved FRS's between the Upcase Table and the first user
    // file must be valid and in-use.

    for( i = UPCASE_TABLE_NUMBER + 1; i < FIRST_USER_FILE_NUMBER; i++ ) {

        if( !GenericFrs.Initialize( i,
                                    MftFile.GetMasterFileTable() ) ||
            !GenericFrs.Create( &StandardInformation ) ||
            !GenericFrs.AddEmptyAttribute( $DATA ) ||
            !GenericFrs.AddSecurityDescriptor( WriteCannedSd,
                                               VolumeBitmap ) ||
            !GenericFrs.Flush( VolumeBitmap ) ) {

            DbgPrintf( "Can't create a generic FRS.\n" );
            return FALSE;
        }
    }


    // Construct the root file name index.


    if( !RootIndex.Save( &RootIndexFile ) ||
        !RootIndexFile.Flush(VolumeBitmap) ) {

        DbgPrint( "Can't save root index.\n" );
        return FALSE;
    }


    // Flush the MFT.  Note that flushing the MFT writes the volume
    // bitmap and the MFT Mirror.

    if( !MftFile.Flush() ) {

        DbgPrint( "Can't flush MFT.\n" );
        return FALSE;
    }



    // Fill in sector zero.  First, copy the boot code in.  Then
    // set the fields of interest.
    //
    memcpy( _boot_sector, NtfsBootCode, _drive->QuerySectorSize() );

    memcpy( _boot_sector->Oem, "NTFS    ", 8 );

    _bpb.BytesPerSector = _drive->QuerySectorSize();
    _bpb.SectorsPerCluster = (UCHAR)ClusterFactor;
    _bpb.ReservedSectors = 0;
    _bpb.Fats = 0;
    _bpb.RootEntries = 0;
    _bpb.Sectors = 0;
    _bpb.Media = _drive->QueryMediaByte();
    _bpb.SectorsPerFat = 0;

    if( OldBpb == NULL ) {

        // Use geometry supplied by the driver.
        //
        _bpb.SectorsPerTrack = (USHORT) _drive->QuerySectorsPerTrack();
        _bpb.Heads = (USHORT) _drive->QueryHeads();
        _bpb.HiddenSectors = _drive->QueryHiddenSectors().GetLowPart();

    } else {

        // Use geometry recorded in the existing Bios
        // Parameter Block.
        //
        _bpb.SectorsPerTrack = OldBpb->SectorsPerTrack;
        _bpb.Heads = OldBpb->Heads;
        _bpb.HiddenSectors = OldBpb->HiddenSectors;
    }

    _bpb.LargeSectors = 0;

    // Unused[0] is used by the boot code to indicate Drive Number.
    //
    memset( _boot_sector->Unused, '\0', 4 );

    _boot_sector->Unused[0] = _drive->IsRemovable() ? PHYS_REMOVABLE :
                                                      PHYS_FIXED ;
    _boot_sector->NumberSectors.LowPart =
                        _drive->QuerySectors().GetLowPart();
    _boot_sector->NumberSectors.HighPart =
                        _drive->QuerySectors().GetHighPart();

    _boot_sector->MftStartLcn = MftLcn;
    _boot_sector->Mft2StartLcn = MftReflection.QueryFirstLcn();
    _boot_sector->ClustersPerFileRecordSegment = ClustersPerFrs;
    _boot_sector->DefaultClustersPerIndexAllocationBuffer =
                                        ClustersPerIndexBuffer;

    _boot_sector->SerialNumber.LowPart = SUPERAREA::ComputeVolId();
    _boot_sector->SerialNumber.HighPart =
            SUPERAREA::ComputeVolId(_boot_sector->SerialNumber.LowPart);

    _boot_sector->Checksum = 0;


    // The elementary disk structures have been created.

    return TRUE;
}

BOOLEAN
NTFS_SA::Create(
    IN      PCNUMBER_SET    BadSectors,
    IN OUT  PMESSAGE        Message,
    IN      PCWSTRING       Label,
    IN      ULONG           ClusterSize,
    IN      ULONG           VirtualSectors
    )
/*++

Routine Description:

    This routine creates a new NTFS volume on disk based on defaults.

Arguments:

    BadSectors  - Supplies a list of the bad sectors on the disk.
    Message     - Supplies an outlet for messages.
    Label       - Supplies an optional volume label (may be NULL).
    ClusterSize - Supplies the desired size of a cluster in bytes.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG ClusterFactor, ClustersPerFrs, ClustersPerIndexBuffer;

    UNREFERENCED_PARAMETER( VirtualSectors );

    if (ClusterSize) {
        ClusterFactor = max(1, ClusterSize/_drive->QuerySectorSize());
    } else {
        ClusterFactor = QueryDefaultClusterFactor( _drive );
    }

    ClustersPerFrs = QueryDefaultClustersPerFrs(
                        _drive, ClusterFactor );
    ClustersPerIndexBuffer = QueryDefaultClustersPerIndexBuffer(
                        _drive, ClusterFactor );

    return( Create( BadSectors,
                    ClusterFactor,
                    ClustersPerFrs,
                    ClustersPerIndexBuffer,
                    0,
                    Message,
                    Label ) );
}

BOOLEAN
NTFS_SA::Create(
    IN      PCNUMBER_SET    BadSectors,
    IN      ULONG           ClusterFactor,
    IN      ULONG           ClustersPerFrs,
    IN      ULONG           ClustersPerIndexBuffer,
    IN      ULONG           InitialLogFileSize,
    IN OUT  PMESSAGE        Message,
    IN      PCWSTRING       Label
    )
/*++

Routine Description:

    This routine creates a new NTFS volume on disk.

Arguments:

    BadSectors              --  Supplies a list of the bad sectors
                                on the disk.
    ClusterFactor           --  Supplies the volume cluster factor
    ClustersPerFrs          --  Supplies the number of clusters per
                                FRS for the volume
    ClustersPerIndexBuffer  --  Supplies the default number of clusters
                                per index allocation block.
    InitialLogFileSize      --  Supplies the log file size.  May be zero,
                                in which case a default value is used.
    Message                 --  Supplies an outlet for messages.
    Label                   --  Supplies an optional volume label
                                (may be NULL).

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_BITMAP VolumeBitmap;
    DSTRING InternalLabel;
    SECRUN BootCodeSecrun;
    HMEM BootCodeMem;

    BIG_INT KBytesInVolume;
    ULONG NumberOfSectors, NumberOfClusters, SectorSize,
            ClusterSize, SectorsInBootArea;


#if 0
    // For testing, add a bad cluster.
    //
    ((PNUMBER_SET) BadSectors)->Add( _drive->QuerySectors() - 16 );
#endif


    // Determine the number of sectors and clusters on the drive.

    if (_drive->QuerySectors().GetHighPart() != 0) {

        DbgAbort("Number of sectors exceeds 32 bits");
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    NumberOfSectors = _drive->QuerySectors().GetLowPart();
    NumberOfClusters = NumberOfSectors/ClusterFactor;


    // Generate a bitmap to cover the number of clusters on the drive.

    if( !VolumeBitmap.Initialize( NumberOfClusters, FALSE ) ) {

        DbgPrint( "Cannot initialize bitmap.\n" );
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    // If the user did not specify a label, prompt for it:
    //
    if (Label) {
        if (!InternalLabel.Initialize(Label)) {
            return FALSE;
        }
    } else {
        Message->Set(MSG_VOLUME_LABEL_NO_MAX);
        Message->Display("");
        Message->QueryStringInput(&InternalLabel);
    }

    while( !IsValidLabel(&InternalLabel)) {

        Message->Set(MSG_INVALID_LABEL_CHARACTERS);
        Message->Display("");

        Message->Set(MSG_VOLUME_LABEL_NO_MAX);
        Message->Display("");
        Message->QueryStringInput(&InternalLabel);
    }

    Message->Set( MSG_FMT_CREATING_FILE_SYSTEM );
    Message->Display( "" );

    // Create the elementary file system structures.  Pass in
    // zero for the initial log file size to indicate that
    // CreateElementaryStructures should choose the size of
    // the log file, and NULL for the OldBpb to indicate that
    // it should use the geometry information from the drive.
    //
    if( !CreateElementaryStructures( &VolumeBitmap,
                                     ClusterFactor,
                                     ClustersPerFrs,
                                     ClustersPerIndexBuffer,
                                     InitialLogFileSize,
                                     BadSectors,
                                     Message,
                                     NULL,
                                     &InternalLabel ) ) {

        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }


    if( !Write() ) {

        DbgPrint( "UNTFS: Unable to write superarea.\n" );
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    // Write the rest of the boot code:
    //
    SectorSize = _drive->QuerySectorSize();

    SectorsInBootArea = ( BYTES_IN_BOOT_AREA % SectorSize ) ?
                            BYTES_IN_BOOT_AREA / SectorSize + 1 :
                            BYTES_IN_BOOT_AREA / SectorSize;

    if( !BootCodeMem.Initialize() ||
        !BootCodeSecrun.Initialize( &BootCodeMem,
                                    _drive,
                                    1,
                                    SectorsInBootArea - 1 ) ) {

        DbgPrint( "UNTFS: Unable to write boot code.\n" );
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    memcpy( BootCodeSecrun.GetBuf(),
            (PUCHAR)NtfsBootCode + _drive->QuerySectorSize(),
            sizeof( NtfsBootCode ) - _drive->QuerySectorSize() );

    if( !BootCodeSecrun.Write( ) ) {

        DbgPrint( "UNTFS: Unable to write boot code.\n" );
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }


    if (!SetSystemId()) {
        Message->Set(MSG_WRITE_PARTITION_TABLE);
        Message->Display("");
        return FALSE;
    }

    Message->Set(MSG_FORMAT_COMPLETE);
    Message->Display("");

    // -----------------------
    // Generate a nice report.
    // -----------------------
    //
    ClusterSize = ClusterFactor * _drive->QuerySectorSize();

    KBytesInVolume = NumberOfClusters;
    KBytesInVolume = KBytesInVolume * ClusterSize / 1024;

    Message->Set(MSG_TOTAL_KILOBYTES);
    Message->Display("%9d", KBytesInVolume.GetLowPart()  );

    Message->Set(MSG_AVAILABLE_KILOBYTES);
    Message->Display("%9d",
       ((ClusterSize * VolumeBitmap.QueryFreeClusters())/1024).GetLowPart() );

    return TRUE;
}
