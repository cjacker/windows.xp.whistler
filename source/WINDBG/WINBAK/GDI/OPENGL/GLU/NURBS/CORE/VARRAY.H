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
 * varray.h - $Revision: 1.1 $
 */

#ifndef __VARRAY_H__
#define __VARRAY_H__

#include "types.h"

class Arc;

class Varray {
public:
			Varray();
			~Varray();
    long		init( REAL, Arc *, Arc * );
    REAL *		varray;
    REAL		vval[1000];
    long		voffset[1000];
    long 		numquads;

private:
    long		size;
    inline void		update( Arc *, long[2], REAL );
    void		grow( long );
    inline void		append( REAL );
};

inline void
Varray::append( REAL v ) 
{
    if( v != vval[numquads] )
        vval[++numquads] = v; 
}


#endif /* __VARRAY_H__ */
