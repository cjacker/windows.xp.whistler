############################################################################
#
#   Copyright (C) 2000, Microsoft Corporation.
#
#   All rights reserved.
#
############################################################################


#
#   Name of target.  Include an extension (.dll, .lib, .exe)
#   If the target is part of the release, set RELEASE to 1.
#

TARGET	    =	drag.lib

RELEASE     =


#
#   Source files.  Remember to prefix each name with .\
#

CPPFILES    = \
		.\drag.cpp \

CFILES	    =

RCFILES     =


#
#   Libraries and other object files to link.
#

DEFFILE     =

LIBS	    =

OBJFILES    =

#
#   Precompiled headers.
#

PFILE       =


!include $(CAIROLE)\ole232\ole.mk
