/**************************************************************************
 *									  *
 * 		 Copyright (C) 2000, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
 * uarray.h - $Revision: 1.1 $
 */

#ifndef __UARRAY_H__
#define __UARRAY_H__

#include "types.h"

class Arc;

class Uarray {
private:
    long		size;
    long		ulines;
public:
			Uarray();
			~Uarray();
    long		init( REAL, Arc *, Arc * );
    REAL *		uarray;
};

#endif /* __UARRAY_H__ */
