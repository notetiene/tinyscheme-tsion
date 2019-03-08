/* $Id: scm_util.h,v 1.4 2011/04/04 22:27:25 alex Exp $ */
/*******************************************************************************

    scm_util.h

    Scheme Utility Definitions.

*******************************************************************************/

#ifndef  SCM_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  SCM_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <scheme.h>			/* TinyScheme definitions. */
#include  "scheme-private.h"		/* TinyScheme internals. */


/*******************************************************************************
    Various definitions defined in "scheme.c", but not in the header files!
*******************************************************************************/

#define  car(p)           ((p)->_object._cons._car)
#define  cdr(p)           ((p)->_object._cons._cdr)
#define  symprop(p)       cdr(p)
#define  caar(p)          car(car(p))
#define  cadr(p)          car(cdr(p))
#define  cdar(p)          cdr(car(p))
#define  cddr(p)          cdr(cdr(p))
#define  cadar(p)         car(cdr(car(p)))
#define  caddr(p)         car(cdr(cdr(p)))
#define  cadaar(p)        car(cdr(car(car(p))))
#define  cadddr(p)        car(cdr(cdr(cdr(p))))
#define  cddddr(p)        cdr(cdr(cdr(cdr(p))))

#define  is_true(sc, p)  ((sc)->T == (p))
#define  is_false(sc, p)  ((sc)->F == (p))

#define  strvalue(p)  ((p)->_object._string._svalue)
#define  strlength(p)  ((p)->_object._string._length)

typedef  enum  scheme_types {
    T_STRING=1,
    T_NUMBER=2,
    T_SYMBOL=3,
    T_PROC=4,
    T_PAIR=5,
    T_CLOSURE=6,
    T_CONTINUATION=7,
    T_FOREIGN=8,
    T_CHARACTER=9,
    T_PORT=10,
    T_VECTOR=11,
    T_MACRO=12,
    T_PROMISE=13,
    T_ENVIRONMENT=14,
    T_LAST_SYSTEM_TYPE=14
}  scheme_types ;

#define  TYPE_BITS  5
#define  T_MASKTYPE      31    /* 0000000000011111 */
#define  T_SYNTAX      4096    /* 0001000000000000 */
#define  T_IMMUTABLE   8192    /* 0010000000000000 */
#define  T_ATOM       16384    /* 0100000000000000 */   /* only for gc */
#define  CLRATOM      49151    /* 1011111111111111 */   /* only for gc */
#define  MARK         32768    /* 1000000000000000 */
#define  UNMARK       32767    /* 0111111111111111 */

#define  typeflag(p)  ((p)->_flag)
#define  type(p)  (typeflag(p) & T_MASKTYPE)

/* TinyScheme's is_integer() and is_real() functions don't mention the fact
   that they don't check the cell's type - they simply check the cell's
   is_fixnum field.  You *must* check is_number() beforehand.  To make sure
   you don't miss that check by accident, use isInteger() and isReal(). */

#define  isInteger(p)	\
	(is_number ((p)) && is_integer ((p)))

#define  isReal(p)	\
	(is_number ((p)) && is_real ((p)))

				/* Defined in "init.scm". */
#define  acons(sc, x, y, z)     \
        (cons ((sc), cons ((sc), (x), (y)), (z)))


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  scm_util_debug  OCD ("scm_util") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  pointer  mk_bstring P_((scheme *sc,
                                const char *string,
                                size_t length))
    OCD ("scm_util") ;

extern  pointer  mk_port P_((scheme *sc,
                             port *pyort))
    OCD ("scm_util") ;

extern  bool  string_push P_((scheme *sc,
                              const char *command))
    OCD ("scm_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
