
//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 2000 - 1993.
//
//  File:       defutil.h
//
//  Contents:   Declarations for utility functions used in the default
//              handler and default link
//
//  Classes:
//
//  Functions:
//
//  History:    dd-mmm-yy Author    Comment
//              20-Nov-93 alexgo    32bit port
//
//--------------------------------------------------------------------------

#if !defined( _DEFUTIL_H_ )
#define _DEFUTIL_H_

INTERNAL_(void)         DuLockContainer(IOleClientSite FAR* pCS,
                                        BOOL fLockNew,
                                        BOOL FAR*pfLockCur);
INTERNAL                DuSetClientSite(BOOL fRunning,
                                        IOleClientSite FAR* pCSNew,
                                        IOleClientSite FAR* FAR* ppCSCur,
                                        BOOL FAR*pfLockCur);
INTERNAL_(void FAR*)    DuCacheDelegate(IUnknown FAR** ppUnk,
                                        REFIID iid,
                                        LPVOID FAR* ppv,
                                        IUnknown *pUnkOuter);


#define  GET_FROM_REGDB(scode)  \
        (((scode == OLE_S_USEREG) || (scode == RPC_E_CANTPOST_INSENDCALL) || \
          (scode == RPC_E_CALL_CANCELED) || (scode == RPC_E_CALL_REJECTED)) \
                 ?      TRUE : FALSE)

#define LONG_ABS(x)     (x < 0 ? labs(x) : x)

#endif // _DEFUTIL_H
