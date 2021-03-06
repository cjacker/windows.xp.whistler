/*++

Copyright (c) 2000-1993  Microsoft Corporation

Module Name:

    driver.c

Abstract:

    This module contains the DriverEntry and other initialization
    code for the Netbios module of the ISN transport.

Author:

    Adam Barr (adamba) 16-November-1993

Environment:

    Kernel mode

Revision History:


--*/

#include "precomp.h"
#pragma hdrstop

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


PDEVICE NbiDevice = NULL;

#if DBG

ULONG NbiDebug = 0xffffffff;
ULONG NbiDebug2 = 0x00000000;
ULONG NbiMemoryDebug = 0x0002482c;
UCHAR NbiDebugMemory[NB_MEMORY_LOG_SIZE][64];
PUCHAR NbiDebugMemoryLoc = NbiDebugMemory[0];
PUCHAR NbiDebugMemoryEnd = NbiDebugMemory[NB_MEMORY_LOG_SIZE];

VOID
NbiDebugMemoryLog(
    IN PUCHAR FormatString,
    ...
)

{
    va_list ArgumentPointer;

    va_start(ArgumentPointer, FormatString);
    RtlZeroMemory (NbiDebugMemoryLoc, 64);
    vsprintf(NbiDebugMemoryLoc, FormatString, ArgumentPointer);
    va_end(ArgumentPointer);

    NbiDebugMemoryLoc += 64;
    if (NbiDebugMemoryLoc >= NbiDebugMemoryEnd) {
        NbiDebugMemoryLoc = NbiDebugMemory[0];
    }

}   /* NbiDebugMemoryLog */


DEFINE_LOCK_STRUCTURE(NbiMemoryInterlock);
MEMORY_TAG NbiMemoryTag[MEMORY_MAX];

DEFINE_LOCK_STRUCTURE(NbiGlobalInterlock);

#endif


#ifdef NB_PACKET_LOG

ULONG NbiPacketLogDebug = NB_PACKET_LOG_RCV_OTHER | NB_PACKET_LOG_SEND_OTHER;
USHORT NbiPacketLogSocket = 0;
DEFINE_LOCK_STRUCTURE(NbiPacketLogLock);
NB_PACKET_LOG_ENTRY NbiPacketLog[NB_PACKET_LOG_LENGTH];
PNB_PACKET_LOG_ENTRY NbiPacketLogLoc = NbiPacketLog;
PNB_PACKET_LOG_ENTRY NbiPacketLogEnd = &NbiPacketLog[NB_PACKET_LOG_LENGTH];

VOID
NbiLogPacket(
    IN BOOLEAN Send,
    IN PUCHAR DestMac,
    IN PUCHAR SrcMac,
    IN USHORT Length,
    IN PVOID NbiHeader,
    IN PVOID Data
    )

{

    CTELockHandle LockHandle;
    PNB_PACKET_LOG_ENTRY PacketLog;
    LARGE_INTEGER TickCount;
    ULONG DataLength;

    CTEGetLock (&NbiPacketLogLock, &LockHandle);

    PacketLog = NbiPacketLogLoc;

    ++NbiPacketLogLoc;
    if (NbiPacketLogLoc >= NbiPacketLogEnd) {
        NbiPacketLogLoc = NbiPacketLog;
    }
    *(UNALIGNED ULONG *)NbiPacketLogLoc->TimeStamp = 0x3e3d3d3d;    // "===>"

    CTEFreeLock (&NbiPacketLogLock, LockHandle);

    RtlZeroMemory (PacketLog, sizeof(NB_PACKET_LOG_ENTRY));

    PacketLog->SendReceive = Send ? '>' : '<';

    KeQueryTickCount(&TickCount);
    _itoa (TickCount.LowPart % 100000, PacketLog->TimeStamp, 10);

    RtlCopyMemory(PacketLog->DestMac, DestMac, 6);
    RtlCopyMemory(PacketLog->SrcMac, SrcMac, 6);
    PacketLog->Length[0] = Length / 256;
    PacketLog->Length[1] = Length % 256;

    if (Length < sizeof(IPX_HEADER)) {
        RtlCopyMemory(&PacketLog->NbiHeader, NbiHeader, Length);
    } else {
        RtlCopyMemory(&PacketLog->NbiHeader, NbiHeader, sizeof(IPX_HEADER));
    }

    DataLength = Length - sizeof(IPX_HEADER);
    if (DataLength < 14) {
        RtlCopyMemory(PacketLog->Data, Data, DataLength);
    } else {
        RtlCopyMemory(PacketLog->Data, Data, 14);
    }

}   /* NbiLogPacket */

#endif // NB_PACKET_LOG


//
// Forward declaration of various routines used in this module.
//

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
NbiUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
NbiDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NbiDispatchOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NbiDispatchInternal (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
NbiFreeResources (
    IN PVOID Adapter
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif

//
// This prevents us from having a bss section.
//

ULONG _setjmpexused = 0;


//
// These two are used in various places in the driver.
//

UCHAR BroadcastAddress[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

UCHAR NetbiosBroadcastName[16] = { '*', 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0 };

ULONG NbiFailLoad = FALSE;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:

    This routine performs initialization of the Netbios ISN module.
    It creates the device objects for the transport
    provider and performs other driver initialization.

Arguments:

    DriverObject - Pointer to driver object created by the system.

    RegistryPath - The name of Netbios's node in the registry.

Return Value:

    The function value is the final status from the initialization operation.

--*/

{
    NTSTATUS status;
    static const NDIS_STRING ProtocolName = NDIS_STRING_CONST("Netbios/IPX Transport");
    PDEVICE Device;
    PIPX_HEADER IpxHeader;
    CTELockHandle LockHandle;

    PCONFIG Config = NULL;

#if 0
    DbgPrint ("NBI: FailLoad at %lx\n", &NbiFailLoad);
    DbgBreakPoint();

    if (NbiFailLoad) {
        return STATUS_UNSUCCESSFUL;
    }
#endif

#if defined(UP_DRIVER)
    //
    // If built for UP, ensure that we're not being loaded on an MP system.
    //

    if (*KeNumberProcessors != 1) {

        NB_DEBUG (DEVICE, ("NBI: UP driver loaded on MP system\n"));
        NbiWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_UP_DRIVER_ON_MP,
            1,
            STATUS_UNSUCCESSFUL,
            NULL,
            0,
            NULL);
        return STATUS_UNSUCCESSFUL;
    }
#endif

    //
    // Initialize the Common Transport Environment.
    //

    if (CTEInitialize() == 0) {
        NB_DEBUG (DEVICE, ("CTEInitialize() failed\n"));
        NbiWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_TRANSPORT_REGISTER_FAILED,
            101,
            STATUS_UNSUCCESSFUL,
            NULL,
            0,
            NULL);
        return STATUS_UNSUCCESSFUL;
    }

#if DBG
    CTEInitLock (&NbiGlobalInterlock);
    CTEInitLock (&NbiMemoryInterlock);
    {
        UINT i;
        for (i = 0; i < MEMORY_MAX; i++) {
            NbiMemoryTag[i].Tag = i;
            NbiMemoryTag[i].BytesAllocated = 0;
        }
    }
#endif
#ifdef NB_PACKET_LOG
    CTEInitLock (&NbiPacketLogLock);
#endif

    CTEAssert (NDIS_PACKET_SIZE == FIELD_OFFSET(NDIS_PACKET, ProtocolReserved[0]));

    NB_DEBUG2 (DEVICE, ("ISN Netbios loaded\n"));

    //
    // This allocates the CONFIG structure and returns
    // it in Config.
    //

    status = NbiGetConfiguration(DriverObject, RegistryPath, &Config);

    if (!NT_SUCCESS (status)) {

        //
        // If it failed it logged an error.
        //

        PANIC (" Failed to initialize transport, ISN Netbios initialization failed.\n");
        return status;
    }


    //
    // Initialize the driver object with this driver's entry points.
    //

    DriverObject->MajorFunction [IRP_MJ_CREATE] = NbiDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_CLOSE] = NbiDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_CLEANUP] = NbiDispatchOpenClose;
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] = NbiDispatchInternal;
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = NbiDispatchDeviceControl;

    DriverObject->DriverUnload = NbiUnload;


    //
    // Create the device object which exports our name.
    //

    status = NbiCreateDevice (DriverObject, &Config->DeviceName, &Device);

    if (!NT_SUCCESS (status)) {

        NbiWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_IPX_CREATE_DEVICE,
            801,
            status,
            NULL,
            0,
            NULL);

        NbiFreeConfiguration(Config);
        return status;
    }

    NbiDevice = Device;


    //
    // Save the relevant configuration parameters.
    //

    Device->AckDelayTime = (Config->Parameters[CONFIG_ACK_DELAY_TIME] / SHORT_TIMER_DELTA) + 1;
    Device->AckWindow = Config->Parameters[CONFIG_ACK_WINDOW];
    Device->AckWindowThreshold = Config->Parameters[CONFIG_ACK_WINDOW_THRESHOLD];
    Device->EnablePiggyBackAck = Config->Parameters[CONFIG_ENABLE_PIGGYBACK_ACK];
    Device->Extensions = Config->Parameters[CONFIG_EXTENSIONS];
    Device->RcvWindowMax = Config->Parameters[CONFIG_RCV_WINDOW_MAX];
    Device->BroadcastCount = Config->Parameters[CONFIG_BROADCAST_COUNT];
    Device->BroadcastTimeout = Config->Parameters[CONFIG_BROADCAST_TIMEOUT] * 500;
    Device->ConnectionCount = Config->Parameters[CONFIG_CONNECTION_COUNT];
    Device->ConnectionTimeout = Config->Parameters[CONFIG_CONNECTION_TIMEOUT] * 500;
    Device->InitPackets = Config->Parameters[CONFIG_INIT_PACKETS];
    Device->MaxPackets = Config->Parameters[CONFIG_MAX_PACKETS];
    Device->InitialRetransmissionTime = Config->Parameters[CONFIG_INIT_RETRANSMIT_TIME];
    Device->Internet = Config->Parameters[CONFIG_INTERNET];
    Device->KeepAliveCount = Config->Parameters[CONFIG_KEEP_ALIVE_COUNT];
    Device->KeepAliveTimeout = Config->Parameters[CONFIG_KEEP_ALIVE_TIMEOUT];
    Device->RetransmitMax = Config->Parameters[CONFIG_RETRANSMIT_MAX];

    Device->FindNameTimeout =
        ((Config->Parameters[CONFIG_BROADCAST_TIMEOUT] * 500) + (FIND_NAME_GRANULARITY/2)) /
            FIND_NAME_GRANULARITY;

    Device->MaxReceiveBuffers = 20;   // BUGBUG: Make it configurable?

    //
    // Now bind to IPX via the internal interface.
    //

    status = NbiBind (Device, Config);

    if (!NT_SUCCESS (status)) {

        //
        // If it failed it logged an error.
        //

        NbiFreeConfiguration(Config);
        NbiDereferenceDevice (Device, DREF_LOADED);
        return status;
    }

    NB_GET_LOCK (&Device->Lock, &LockHandle);

    //
    // Allocate our initial connectionless packet pool.
    //

    NbiAllocateSendPool (Device);

    //
    // Allocate our initial receive packet pool.
    //

    NbiAllocateReceivePool (Device);

    //
    // Allocate our initial receive buffer pool.
    //

    NbiAllocateReceiveBufferPool (Device);

    NB_FREE_LOCK (&Device->Lock, LockHandle);

    //
    // Start the timer system.
    //

    NbiInitializeTimers (Device);


    //
    // Fill in the default connnectionless header.
    //

    IpxHeader = &Device->ConnectionlessHeader;
    IpxHeader->CheckSum = 0xffff;
    IpxHeader->PacketLength[0] = 0;
    IpxHeader->PacketLength[1] = 0;
    IpxHeader->TransportControl = 0;
    IpxHeader->PacketType = 0;
    *(UNALIGNED ULONG *)(IpxHeader->DestinationNetwork) = 0;
    RtlCopyMemory(IpxHeader->DestinationNode, BroadcastAddress, 6);
    IpxHeader->DestinationSocket = NB_SOCKET;
    RtlCopyMemory(IpxHeader->SourceNetwork, Device->Bind.Network, 4);
    RtlCopyMemory(IpxHeader->SourceNode, Device->Bind.Node, 6);
    IpxHeader->SourceSocket = NB_SOCKET;

    Device->State = DEVICE_STATE_OPEN;

    NbiFreeConfiguration(Config);

    return STATUS_SUCCESS;

}   /* DriverEntry */

VOID
NbiUnload(
    IN PDRIVER_OBJECT DriverObject
    )

/*++

Routine Description:

    This routine unloads the sample transport driver.
    It unbinds from any NDIS drivers that are open and frees all resources
    associated with the transport. The I/O system will not call us until
    nobody above has Netbios open.

Arguments:

    DriverObject - Pointer to driver object created by the system.

Return Value:

    None. When the function returns, the driver is unloaded.

--*/

{
    PNETBIOS_CACHE CacheName;
    PDEVICE Device = NbiDevice;
    PLIST_ENTRY p;

    UNREFERENCED_PARAMETER (DriverObject);


    Device->State = DEVICE_STATE_STOPPING;

    //
    // Free the cache of netbios names.
    //

    while (!IsListEmpty (&Device->NameCache)) {

        p = RemoveHeadList (&Device->NameCache);
        CacheName = CONTAINING_RECORD (p, NETBIOS_CACHE, Linkage);

        NB_DEBUG2 (CACHE, ("Free cache entry %lx\n", CacheName));

        NbiFreeMemory(
            CacheName,
            sizeof(NETBIOS_CACHE) + ((CacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
            MEMORY_CACHE,
            "Free entries");

    }

    //
    // Cancel the long timer.
    //

    if (CTEStopTimer (&Device->LongTimer)) {
        NbiDereferenceDevice (Device, DREF_LONG_TIMER);
    }

    //
    // Unbind from the IPX driver.
    //

    NbiUnbind (Device);

    //
    // This event will get set when the reference count
    // drops to 0.
    //

    KeInitializeEvent(
        &Device->UnloadEvent,
        NotificationEvent,
        FALSE);
    Device->UnloadWaiting = TRUE;

    //
    // Remove the reference for us being loaded.
    //

    NbiDereferenceDevice (Device, DREF_LOADED);

    //
    // Wait for our count to drop to zero.
    //

    KeWaitForSingleObject(
        &Device->UnloadEvent,
        Executive,
        KernelMode,
        TRUE,
        (PLARGE_INTEGER)NULL
        );

    //
    // Do the cleanup that has to happen at IRQL 0.
    //

    ExDeleteResource (&Device->AddressResource);
    IoDeleteDevice ((PDEVICE_OBJECT)Device);

}   /* NbiUnload */


VOID
NbiFreeResources (
    IN PVOID Adapter
    )
/*++

Routine Description:

    This routine is called by Netbios to clean up the data structures associated
    with a given Device. When this routine exits, the Device
    should be deleted as it no longer has any assocaited resources.

Arguments:

    Device - Pointer to the Device we wish to clean up.

Return Value:

    None.

--*/
{
#if 0
    PLIST_ENTRY p;
    PSINGLE_LIST_ENTRY s;
    PTP_PACKET packet;
    PNDIS_PACKET ndisPacket;
    PBUFFER_TAG BufferTag;
#endif


#if 0
    //
    // Clean up packet pool.
    //

    while ( Device->PacketPool.Next != NULL ) {
        s = PopEntryList( &Device->PacketPool );
        packet = CONTAINING_RECORD( s, TP_PACKET, Linkage );

        NbiDeallocateSendPacket (Device, packet);
    }

    //
    // Clean up receive packet pool
    //

    while ( Device->ReceivePacketPool.Next != NULL) {
        s = PopEntryList (&Device->ReceivePacketPool);

        //
        // HACK: This works because Linkage is the first field in
        // ProtocolReserved for a receive packet.
        //

        ndisPacket = CONTAINING_RECORD (s, NDIS_PACKET, ProtocolReserved[0]);

        NbiDeallocateReceivePacket (Device, ndisPacket);
    }


    //
    // Clean up receive buffer pool.
    //

    while ( Device->ReceiveBufferPool.Next != NULL ) {
        s = PopEntryList( &Device->ReceiveBufferPool );
        BufferTag = CONTAINING_RECORD (s, BUFFER_TAG, Linkage );

        NbiDeallocateReceiveBuffer (Device, BufferTag);
    }

#endif

}   /* NbiFreeResources */


NTSTATUS
NbiDispatchOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine is the main dispatch routine for the IPXNB device driver.
    It accepts an I/O Request Packet, performs the request, and then
    returns with the appropriate status.

Arguments:

    DeviceObject - Pointer to the device object for this driver.

    Irp - Pointer to the request packet representing the I/O request.

Return Value:

    The function value is the status of the operation.

--*/

{
    CTELockHandle LockHandle;
    PDEVICE Device = (PDEVICE)DeviceObject;
    NTSTATUS Status;
    PFILE_FULL_EA_INFORMATION openType;
    BOOLEAN found;
    PADDRESS_FILE AddressFile;
    PCONNECTION Connection;
    PREQUEST Request;
    UINT i;
    NB_DEFINE_LOCK_HANDLE (LockHandle1)
    NB_DEFINE_SYNC_CONTEXT (SyncContext)


    if (Device->State != DEVICE_STATE_OPEN) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }

    //
    // Allocate a request to track this IRP.
    //

    Request = NbiAllocateRequest (Device, Irp);
    IF_NOT_ALLOCATED(Request) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    //
    // Make sure status information is consistent every time.
    //

    MARK_REQUEST_PENDING(Request);
    REQUEST_STATUS(Request) = STATUS_PENDING;
    REQUEST_INFORMATION(Request) = 0;

    //
    // Case on the function that is being performed by the requestor.  If the
    // operation is a valid one for this device, then make it look like it was
    // successfully completed, where possible.
    //


    switch (REQUEST_MAJOR_FUNCTION(Request)) {

    //
    // The Create function opens a transport object (either address or
    // connection).  Access checking is performed on the specified
    // address to ensure security of transport-layer addresses.
    //

    case IRP_MJ_CREATE:

        openType = OPEN_REQUEST_EA_INFORMATION(Request);

        if (openType != NULL) {

            found = TRUE;

            for (i=0;i<openType->EaNameLength;i++) {
                if (openType->EaName[i] == TdiTransportAddress[i]) {
                    continue;
                } else {
                    found = FALSE;
                    break;
                }
            }

            if (found) {
                Status = NbiOpenAddress (Device, Request);
                break;
            }

            //
            // Connection?
            //

            found = TRUE;

            for (i=0;i<openType->EaNameLength;i++) {
                if (openType->EaName[i] == TdiConnectionContext[i]) {
                     continue;
                } else {
                    found = FALSE;
                    break;
                }
            }

            if (found) {
                Status = NbiOpenConnection (Device, Request);
                break;
            }

        } else {

            NB_GET_LOCK (&Device->Lock, &LockHandle);

            REQUEST_OPEN_CONTEXT(Request) = (PVOID)(Device->ControlChannelIdentifier);
            ++Device->ControlChannelIdentifier;
            if (Device->ControlChannelIdentifier == 0) {
                Device->ControlChannelIdentifier = 1;
            }

            NB_FREE_LOCK (&Device->Lock, LockHandle);

            REQUEST_OPEN_TYPE(Request) = (PVOID)TDI_CONTROL_CHANNEL_FILE;
            Status = STATUS_SUCCESS;
        }

        break;

    case IRP_MJ_CLOSE:

        //
        // The Close function closes a transport endpoint, terminates
        // all outstanding transport activity on the endpoint, and unbinds
        // the endpoint from its transport address, if any.  If this
        // is the last transport endpoint bound to the address, then
        // the address is removed from the provider.
        //

        switch ((ULONG)REQUEST_OPEN_TYPE(Request)) {

        case TDI_TRANSPORT_ADDRESS_FILE:

            AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

            //
            // This creates a reference to AddressFile.
            //

            Status = NbiVerifyAddressFile(AddressFile);

            if (!NT_SUCCESS (Status)) {
                Status = STATUS_INVALID_HANDLE;
            } else {
                Status = NbiCloseAddressFile (Device, Request);
                NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);

            }

            break;

        case TDI_CONNECTION_FILE:

            Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

            //
            // We don't call VerifyConnection because the I/O
            // system should only give us one close and the file
            // object should be valid. This helps avoid a window
            // where two threads call HandleConnectionZero at the
            // same time.
            //

            Status = NbiCloseConnection (Device, Request);

            break;

        case TDI_CONTROL_CHANNEL_FILE:

            //
            // See if it is one of the upper driver's control channels.
            //

            Status = STATUS_SUCCESS;

            break;

        default:

            Status = STATUS_INVALID_HANDLE;

        }

        break;

    case IRP_MJ_CLEANUP:

        //
        // Handle the two stage IRP for a file close operation. When the first
        // stage hits, run down all activity on the object of interest. This
        // do everything to it but remove the creation hold. Then, when the
        // CLOSE irp hits, actually close the object.
        //

        switch ((ULONG)REQUEST_OPEN_TYPE(Request)) {

        case TDI_TRANSPORT_ADDRESS_FILE:

            AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

            Status = NbiVerifyAddressFile(AddressFile);

            if (!NT_SUCCESS (Status)) {

                Status = STATUS_INVALID_HANDLE;

            } else {

                NbiStopAddressFile (AddressFile, AddressFile->Address);
                NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);
                Status = STATUS_SUCCESS;
            }

            break;

        case TDI_CONNECTION_FILE:

            Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

            Status = NbiVerifyConnection(Connection);

            if (!NT_SUCCESS (Status)) {

                Status = STATUS_INVALID_HANDLE;

            } else {

                NB_BEGIN_SYNC (&SyncContext);
                NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle1);

                //
                // This call releases the lock.
                //

                NbiStopConnection(
                    Connection,
                    STATUS_INVALID_CONNECTION
                    NB_LOCK_HANDLE_ARG (LockHandle1));

                NB_END_SYNC (&SyncContext);

                NbiDereferenceConnection (Connection, CREF_VERIFY);
                Status = STATUS_SUCCESS;
            }

            break;

        case TDI_CONTROL_CHANNEL_FILE:

            Status = STATUS_SUCCESS;
            break;

        default:

            Status = STATUS_INVALID_HANDLE;

        }

        break;

    default:

        Status = STATUS_INVALID_DEVICE_REQUEST;

    } /* major function switch */

    if (Status != STATUS_PENDING) {
        UNMARK_REQUEST_PENDING(Request);
        REQUEST_STATUS(Request) = Status;
        NbiCompleteRequest (Request);
        NbiFreeRequest (Device, Request);
    }

    //
    // Return the immediate status code to the caller.
    //

    return Status;

}   /* NbiDispatchOpenClose */


NTSTATUS
NbiDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine dispatches TDI request types to different handlers based
    on the minor IOCTL function code in the IRP's current stack location.
    In addition to cracking the minor function code, this routine also
    reaches into the IRP and passes the packetized parameters stored there
    as parameters to the various TDI request handlers so that they are
    not IRP-dependent.

Arguments:

    DeviceObject - Pointer to the device object for this driver.

    Irp - Pointer to the request packet representing the I/O request.

Return Value:

    The function value is the status of the operation.

--*/

{
    NTSTATUS Status;
    PDEVICE Device = (PDEVICE)DeviceObject;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (Irp);

    //
    // Branch to the appropriate request handler.  Preliminary checking of
    // the size of the request block is performed here so that it is known
    // in the handlers that the minimum input parameters are readable.  It
    // is *not* determined here whether variable length input fields are
    // passed correctly; this is a check which must be made within each routine.
    //

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        default:

            //
            // Convert the user call to the proper internal device call.
            //

            Status = TdiMapUserRequest (DeviceObject, Irp, IrpSp);

            if (Status == STATUS_SUCCESS) {

                //
                // If TdiMapUserRequest returns SUCCESS then the IRP
                // has been converted into an IRP_MJ_INTERNAL_DEVICE_CONTROL
                // IRP, so we dispatch it as usual. The IRP will
                // be completed by this call.
                //

                Status = NbiDispatchInternal (DeviceObject, Irp);

            } else {

                Irp->IoStatus.Status = Status;
                IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

            }

            break;
    }

    return Status;

}   /* NbiDeviceControl */


NB_TDI_DISPATCH_ROUTINE NbiDispatchInternalTable[] = {
    NbiTdiAssociateAddress,
    NbiTdiDisassociateAddress,
    NbiTdiConnect,
    NbiTdiListen,
    NbiTdiAccept,
    NbiTdiDisconnect,
    NbiTdiSend,
    NbiTdiReceive,
    NbiTdiSendDatagram,
    NbiTdiReceiveDatagram,
    NbiTdiSetEventHandler,
    NbiTdiQueryInformation,
    NbiTdiSetInformation,
    NbiTdiAction
    };


NTSTATUS
NbiDispatchInternal(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine dispatches TDI request types to different handlers based
    on the minor IOCTL function code in the IRP's current stack location.
    In addition to cracking the minor function code, this routine also
    reaches into the IRP and passes the packetized parameters stored there
    as parameters to the various TDI request handlers so that they are
    not IRP-dependent.

Arguments:

    DeviceObject - Pointer to the device object for this driver.

    Irp - Pointer to the request packet representing the I/O request.

Return Value:

    The function value is the status of the operation.

--*/

{
    NTSTATUS Status;
    PDEVICE Device = (PDEVICE)DeviceObject;
    PREQUEST Request;
    UCHAR MinorFunction;

    if (Device->State != DEVICE_STATE_OPEN) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }


    //
    // Allocate a request to track this IRP.
    //

    Request = NbiAllocateRequest (Device, Irp);
    IF_NOT_ALLOCATED(Request) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    //
    // Make sure status information is consistent every time.
    //

    MARK_REQUEST_PENDING(Request);
    REQUEST_STATUS(Request) = STATUS_PENDING;
    REQUEST_INFORMATION(Request) = 0;


    //
    // Branch to the appropriate request handler.
    //

    MinorFunction = REQUEST_MINOR_FUNCTION(Request) - 1;

    if (MinorFunction <= (TDI_ACTION-1)) {

        Status = (*NbiDispatchInternalTable[MinorFunction]) (
                     Device,
                     Request);

    } else {

        NB_DEBUG (DRIVER, ("Unsupported minor code %d\n", MinorFunction+1));
        if ((MinorFunction+1) == TDI_DISCONNECT) {
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INVALID_DEVICE_REQUEST;
        }
    }

    if (Status != STATUS_PENDING) {
        UNMARK_REQUEST_PENDING(Request);
        REQUEST_STATUS(Request) = Status;
        NbiCompleteRequest (Request);
        NbiFreeRequest (Device, Request);
    }

    //
    // Return the immediate status code to the caller.
    //

    return Status;

}   /* NbiDispatchInternal */


PVOID
NbipAllocateMemory(
    IN ULONG BytesNeeded,
    IN ULONG Tag,
    IN BOOLEAN ChargeDevice
    )

/*++

Routine Description:

    This routine allocates memory, making sure it is within
    the limit allowed by the device.

Arguments:

    BytesNeeded - The number of bytes to allocated.

    ChargeDevice - TRUE if the device should be charged.

Return Value:

    None.

--*/

{
    PVOID Memory;
    PDEVICE Device = NbiDevice;

    if (ChargeDevice) {
        if ((Device->MemoryLimit != 0) &&
                (((LONG)(Device->MemoryUsage + BytesNeeded) >
                    Device->MemoryLimit))) {

            NbiPrint1 ("Nbi: Could not allocate %d: limit\n", BytesNeeded);
            NbiWriteResourceErrorLog (Device, BytesNeeded, Tag);
            return NULL;
        }
    }

#if ISN_NT
    Memory = ExAllocatePoolWithTag (NonPagedPool, BytesNeeded, ' IBN');
#else
    Memory = CTEAllocMem (BytesNeeded);
#endif

    if (Memory == NULL) {

        NbiPrint1("Nbi: Could not allocate %d: no pool\n", BytesNeeded);

        if (ChargeDevice) {
            NbiWriteResourceErrorLog (Device, BytesNeeded, Tag);
        }

        return NULL;
    }

    if (ChargeDevice) {
        Device->MemoryUsage += BytesNeeded;
    }

    return Memory;

}   /* NbipAllocateMemory */


VOID
NbipFreeMemory(
    IN PVOID Memory,
    IN ULONG BytesAllocated,
    IN BOOLEAN ChargeDevice
    )

/*++

Routine Description:

    This routine frees memory allocated with NbipAllocateMemory.

Arguments:

    Memory - The memory allocated.

    BytesAllocated - The number of bytes to freed.

    ChargeDevice - TRUE if the device should be charged.

Return Value:

    None.

--*/

{
    PDEVICE Device = NbiDevice;

#if ISN_NT
    ExFreePool (Memory);
#else
    CTEFreeMem (Memory);
#endif

    if (ChargeDevice) {
        Device->MemoryUsage -= BytesAllocated;
    }

}   /* NbipFreeMemory */

#if DBG


PVOID
NbipAllocateTaggedMemory(
    IN ULONG BytesNeeded,
    IN ULONG Tag,
    IN PUCHAR Description
    )

/*++

Routine Description:

    This routine allocates memory, charging it to the device.
    If it cannot allocate memory it uses the Tag and Descriptor
    to log an error.

Arguments:

    BytesNeeded - The number of bytes to allocated.

    Tag - A unique ID used in the error log.

    Description - A text description of the allocation.

Return Value:

    None.

--*/

{
    PVOID Memory;

    UNREFERENCED_PARAMETER(Description);

    Memory = NbipAllocateMemory(BytesNeeded, Tag, (BOOLEAN)(Tag != MEMORY_CONFIG));

    if (Memory) {
        ExInterlockedAddUlong(
            &NbiMemoryTag[Tag].BytesAllocated,
            BytesNeeded,
            &NbiMemoryInterlock);
    }

    return Memory;

}   /* NbipAllocateTaggedMemory */


VOID
NbipFreeTaggedMemory(
    IN PVOID Memory,
    IN ULONG BytesAllocated,
    IN ULONG Tag,
    IN PUCHAR Description
    )

/*++

Routine Description:

    This routine frees memory allocated with NbipAllocateTaggedMemory.

Arguments:

    Memory - The memory allocated.

    BytesAllocated - The number of bytes to freed.

    Tag - A unique ID used in the error log.

    Description - A text description of the allocation.

Return Value:

    None.

--*/

{

    UNREFERENCED_PARAMETER(Description);

    ExInterlockedAddUlong(
        &NbiMemoryTag[Tag].BytesAllocated,
        (ULONG)(-(LONG)BytesAllocated),
        &NbiMemoryInterlock);

    NbipFreeMemory (Memory, BytesAllocated, (BOOLEAN)(Tag != MEMORY_CONFIG));

}   /* NbipFreeTaggedMemory */

#endif


VOID
NbiWriteResourceErrorLog(
    IN PDEVICE Device,
    IN ULONG BytesNeeded,
    IN ULONG UniqueErrorValue
    )

/*++

Routine Description:

    This routine allocates and writes an error log entry indicating
    an out of resources condition.

Arguments:

    Device - Pointer to the device context.

    BytesNeeded - If applicable, the number of bytes that could not
        be allocated.

    UniqueErrorValue - Used as the UniqueErrorValue in the error log
        packet.

Return Value:

    None.

--*/

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    PUCHAR StringLoc;
    ULONG TempUniqueError;
    static WCHAR UniqueErrorBuffer[4] = L"000";
    INT i;


    EntrySize = sizeof(IO_ERROR_LOG_PACKET) +
                Device->DeviceNameLength +
                sizeof(UniqueErrorBuffer);

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        (PDEVICE_OBJECT)Device,
        EntrySize
    );

    //
    // Convert the error value into a buffer.
    //

    TempUniqueError = UniqueErrorValue;
    for (i=1; i>=0; i--) {
        UniqueErrorBuffer[i] = (WCHAR)((TempUniqueError % 10) + L'0');
        TempUniqueError /= 10;
    }

    if (errorLogEntry != NULL) {

        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = sizeof(ULONG);
        errorLogEntry->NumberOfStrings = 2;
        errorLogEntry->StringOffset = sizeof(IO_ERROR_LOG_PACKET);
        errorLogEntry->EventCategory = 0;
        errorLogEntry->ErrorCode = EVENT_TRANSPORT_RESOURCE_POOL;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
        errorLogEntry->SequenceNumber = (ULONG)-1;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->DumpData[0] = BytesNeeded;

        StringLoc = ((PUCHAR)errorLogEntry) + errorLogEntry->StringOffset;
        RtlCopyMemory (StringLoc, Device->DeviceName, Device->DeviceNameLength);

        StringLoc += Device->DeviceNameLength;
        RtlCopyMemory (StringLoc, UniqueErrorBuffer, sizeof(UniqueErrorBuffer));

        IoWriteErrorLogEntry(errorLogEntry);

    }

}   /* NbiWriteResourceErrorLog */


VOID
NbiWriteGeneralErrorLog(
    IN PDEVICE Device,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PWSTR SecondString,
    IN ULONG DumpDataCount,
    IN ULONG DumpData[]
    )

/*++

Routine Description:

    This routine allocates and writes an error log entry indicating
    a general problem as indicated by the parameters. It handles
    event codes REGISTER_FAILED, BINDING_FAILED, ADAPTER_NOT_FOUND,
    TRANSFER_DATA, TOO_MANY_LINKS, and BAD_PROTOCOL. All these
    events have messages with one or two strings in them.

Arguments:

    Device - Pointer to the device context, or this may be
        a driver object instead.

    ErrorCode - The transport event code.

    UniqueErrorValue - Used as the UniqueErrorValue in the error log
        packet.

    FinalStatus - Used as the FinalStatus in the error log packet.

    SecondString - If not NULL, the string to use as the %3
        value in the error log packet.

    DumpDataCount - The number of ULONGs of dump data.

    DumpData - Dump data for the packet.

Return Value:

    None.

--*/

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    ULONG SecondStringSize;
    PUCHAR StringLoc;
    static WCHAR DriverName[8] = L"NwlnkNb";

    EntrySize = sizeof(IO_ERROR_LOG_PACKET) +
                (DumpDataCount * sizeof(ULONG));

    if (Device->Type == IO_TYPE_DEVICE) {
        EntrySize += (UCHAR)Device->DeviceNameLength;
    } else {
        EntrySize += sizeof(DriverName);
    }

    if (SecondString) {
        SecondStringSize = (wcslen(SecondString)*sizeof(WCHAR)) + sizeof(UNICODE_NULL);
        EntrySize += (UCHAR)SecondStringSize;
    }

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        (PDEVICE_OBJECT)Device,
        EntrySize
    );

    if (errorLogEntry != NULL) {

        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = (USHORT)(DumpDataCount * sizeof(ULONG));
        errorLogEntry->NumberOfStrings = (SecondString == NULL) ? 1 : 2;
        errorLogEntry->StringOffset =
            sizeof(IO_ERROR_LOG_PACKET) + ((DumpDataCount-1) * sizeof(ULONG));
        errorLogEntry->EventCategory = 0;
        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;
        errorLogEntry->SequenceNumber = (ULONG)-1;
        errorLogEntry->IoControlCode = 0;

        if (DumpDataCount) {
            RtlCopyMemory(errorLogEntry->DumpData, DumpData, DumpDataCount * sizeof(ULONG));
        }

        StringLoc = ((PUCHAR)errorLogEntry) + errorLogEntry->StringOffset;
        if (Device->Type == IO_TYPE_DEVICE) {
            RtlCopyMemory (StringLoc, Device->DeviceName, Device->DeviceNameLength);
            StringLoc += Device->DeviceNameLength;
        } else {
            RtlCopyMemory (StringLoc, DriverName, sizeof(DriverName));
            StringLoc += sizeof(DriverName);
        }
        if (SecondString) {
            RtlCopyMemory (StringLoc, SecondString, SecondStringSize);
        }

        IoWriteErrorLogEntry(errorLogEntry);

    }

}   /* NbiWriteGeneralErrorLog */


VOID
NbiWriteOidErrorLog(
    IN PDEVICE Device,
    IN NTSTATUS ErrorCode,
    IN NTSTATUS FinalStatus,
    IN PWSTR AdapterString,
    IN ULONG OidValue
    )

/*++

Routine Description:

    This routine allocates and writes an error log entry indicating
    a problem querying or setting an OID on an adapter. It handles
    event codes SET_OID_FAILED and QUERY_OID_FAILED.

Arguments:

    Device - Pointer to the device context.

    ErrorCode - Used as the ErrorCode in the error log packet.

    FinalStatus - Used as the FinalStatus in the error log packet.

    AdapterString - The name of the adapter we were bound to.

    OidValue - The OID which could not be set or queried.

Return Value:

    None.

--*/

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    ULONG AdapterStringSize;
    PUCHAR StringLoc;
    static WCHAR OidBuffer[9] = L"00000000";
    INT i;
    UINT CurrentDigit;

    AdapterStringSize = (wcslen(AdapterString)*sizeof(WCHAR)) + sizeof(UNICODE_NULL);
    EntrySize = sizeof(IO_ERROR_LOG_PACKET) -
                sizeof(ULONG) +
                Device->DeviceNameLength +
                AdapterStringSize +
                sizeof(OidBuffer);

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        (PDEVICE_OBJECT)Device,
        EntrySize
    );

    //
    // Convert the OID into a buffer.
    //

    for (i=7; i>=0; i--) {
        CurrentDigit = OidValue & 0xf;
        OidValue >>= 4;
        if (CurrentDigit >= 0xa) {
            OidBuffer[i] = (WCHAR)(CurrentDigit - 0xa + L'A');
        } else {
            OidBuffer[i] = (WCHAR)(CurrentDigit + L'0');
        }
    }

    if (errorLogEntry != NULL) {

        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = 0;
        errorLogEntry->NumberOfStrings = 3;
        errorLogEntry->StringOffset = sizeof(IO_ERROR_LOG_PACKET) - sizeof(ULONG);
        errorLogEntry->EventCategory = 0;
        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->UniqueErrorValue = 0;
        errorLogEntry->FinalStatus = FinalStatus;
        errorLogEntry->SequenceNumber = (ULONG)-1;
        errorLogEntry->IoControlCode = 0;

        StringLoc = ((PUCHAR)errorLogEntry) + errorLogEntry->StringOffset;
        RtlCopyMemory (StringLoc, Device->DeviceName, Device->DeviceNameLength);
        StringLoc += Device->DeviceNameLength;

        RtlCopyMemory (StringLoc, OidBuffer, sizeof(OidBuffer));
        StringLoc += sizeof(OidBuffer);

        RtlCopyMemory (StringLoc, AdapterString, AdapterStringSize);

        IoWriteErrorLogEntry(errorLogEntry);

    }

}   /* NbiWriteOidErrorLog */

