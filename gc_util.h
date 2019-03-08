/* $Id: gc_util.h,v 1.2 2009/09/09 23:35:06 alex Exp $ */
/*******************************************************************************

    gc_util.h

    Garbage Collection Utility Definitions.

*******************************************************************************/

#ifndef  GC_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  GC_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  gc_util_debug  OCD ("gc_util") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  UniqueID  gc_protect P_((scheme *sc,
                                 pointer value))
    OCD ("gc_util") ;

extern  pointer  gc_retrieve P_((scheme *sc,
                                 UniqueID id))
    OCD ("gc_util") ;

extern  void  gc_unprotect P_((scheme *sc,
                               UniqueID id))
    OCD ("gc_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
