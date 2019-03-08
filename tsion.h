/* $Id: tsion.h,v 1.3 2009/09/09 23:35:06 alex Exp $ */
/*******************************************************************************

    tsion.h

    TinyScheme with I/O Event Dispatcher and Network extenstions (TSION).

*******************************************************************************/

#ifndef  TSION_H			/* Has the file been INCLUDE'd already? */
#define  TSION_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <scheme.h>			/* TinyScheme definitions. */
#include  "scheme-private.h"		/* TinyScheme internals. */
#include  "scm_util.h"			/* Scheme utilities. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

/* Register foreign functions. */

extern  void  addFuncsDRS P_((scheme *sc)) ;
extern  void  addFuncsIOX P_((scheme *sc)) ;
extern  void  addFuncsLFN P_((scheme *sc)) ;
extern  void  addFuncsMISC P_((scheme *sc)) ;
extern  void  addFuncsNET P_((scheme *sc)) ;
extern  void  addFuncsREX P_((scheme *sc)) ;
extern  void  addFuncsSKT P_((scheme *sc)) ;
extern  void  addFuncsTCP P_((scheme *sc)) ;


/*******************************************************************************
    Implement opaque data type.
*******************************************************************************/

typedef  void  *opaque ;

bool  is_opaque P_((pointer p)) ;
pointer  mk_opaque P_((scheme *sc, opaque value)) ;
opaque  opaque_value P_((pointer p)) ;


/*******************************************************************************
    TSION-Specific Per-Interpreter External Data Structure - this should be
        allocated using calloc(3) and assigned to the "ext_data" field of the
        TinyScheme interpreter structure.
*******************************************************************************/

typedef  long  UniqueID ;

typedef  struct  _TsionSpecific {
    UniqueID  idCounter ;		/* Unique ID counter. */
    pointer  grabValue ;		/* Value from most recent GRAB. */
}  _TsionSpecific, *TsionSpecific ;

				/* Get or set field. */
#define  TS(sc, field)		\
	(((TsionSpecific) ((sc)->ext_data))->field)


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
