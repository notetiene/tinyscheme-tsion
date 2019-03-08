/* $Id: plist_util.h,v 1.2 2009/09/09 23:35:06 alex Exp $ */
/*******************************************************************************

    plist_util.h

    TinyScheme Property List Utilities.

*******************************************************************************/

#ifndef  PLIST_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  PLIST_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <scheme.h>			/* TinyScheme definitions. */
#include  "scheme-private.h"		/* TinyScheme internals. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  plist_util_debug  OCD ("plist_ut") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  pointer  plistGet P_((scheme *sc,
                              const char *symbol,
                              const char *property))
    OCD ("plist_ut") ;

extern  errno_t  plistPut P_((scheme *sc,
                              const char *symbol,
                              const char *property,
                              pointer value))
    OCD ("plist_ut") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
