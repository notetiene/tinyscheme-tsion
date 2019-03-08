/* $Id: opaque.c,v 1.2 2009/08/30 13:24:11 alex Exp $ */
/*******************************************************************************

File:

    opaque.c

    Opaque Data Type.


Author:    Alex Measday


Purpose:

    The OPAQUE package implements an opaque (void *) data type.  Data types
    are defined internally in the "scheme.c" core; note that the enumerated
    type, "scheme_types", is not visible outside of "scheme.c".  To avoid
    having to modify the core file, the OPAQUE package encodes (void *)
    pointers in ASCII strings - slow, but portable to platforms that support
    the "%p" sprintf/sscanf format.


Public Procedures:

    is_opaque() - returns true if a Scheme cell is an opaque value.
    mk_opaque() - makes a Scheme cell containing an opaque value.
    opaque_value() - returns the opaque value from a Scheme cell.

Private Procedures:

    decodeOpaque() - decodes an opaque value from a string.
    encodeOpaque() - encodes an opaque value as a string.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */
#include  "scheme-private.h"		/* TinyScheme internals. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  opaque  decodeOpaque P_((const char *stringValue)) ;
static  char  *encodeOpaque P_((opaque value)) ;

/*!*****************************************************************************

Procedure:

    is_opaque ()

    Check if a Scheme Cell is an Opaque Value.


Purpose:

    Function is_opaque() returns true if a Scheme cell contains an encoded
    opaque value and false otherwise.


    Invocation:

        flag = is_opaque (cell) ;

    where

        <cell>		- I
            is a Scheme cell.
        <flag>		- I
            returns true if the Scheme cell contains an encoded opaque value
            and false otherwise.

*******************************************************************************/


bool  is_opaque (

#    if PROTOTYPES
        pointer  cell)
#    else
        cell)

        pointer  cell ;
#    endif

{

    return (is_string (cell) && (strncmp (strvalue (cell), "OPAQ", 4) == 0)) ;

}

/*!*****************************************************************************

Procedure:

    mk_opaque ()

    Make a Scheme Cell Containing an Opaque Value.


Purpose:

    Function mk_opaque() creates a new Scheme cell containing an encoded
    opaque value.


    Invocation:

        cell = mk_opaque (sc, value) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <value>		- I
            is the opaque (void *) value.
        <cell>		- O
            returns a new Scheme cell containing the encoded opaque value.

*******************************************************************************/


pointer  mk_opaque (

#    if PROTOTYPES
        scheme  *sc,
        opaque  value)
#    else
        sc, value)

        scheme  *sc ;
        opaque  value ;
#    endif

{

    return (mk_string (sc, encodeOpaque (value))) ;

}

/*!*****************************************************************************

Procedure:

    opaque_value ()

    Get the Opaque Value from a Scheme Cell.


Purpose:

    Function opaque_value() returns the decoded opaque value from a Scheme cell.


    Invocation:

        value = opaque_value (cell) ;

    where

        <cell>		- I
            is the Scheme cell containing the encoded opaque value.
        <value>		- I
            is the opaque (void *) value.

*******************************************************************************/


opaque  opaque_value (

#    if PROTOTYPES
        pointer  cell)
#    else
        cell)

        pointer  cell ;
#    endif

{

    if (is_opaque (cell)) {
        return (decodeOpaque (strvalue (cell))) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(opaque_value) Not an opaque value: ") ;
        return (NULL) ;
    }

}

/*!*****************************************************************************

Procedure:

    decodeOpaque ()

    Decode an Opaque Value from a String.


Purpose:

    Function decodeOpaque() decodes an opaque value from a string;
    see encodeOpaque() for the encoding scheme.


    Invocation:

        value = decodeOpaque (stringValue) ;

    where

        <stringValue>	- I
            returns a string containing the encoded opaque value.
        <value>		- O
            returns the opaque (void *) value decoded from the string;
            NULL is returned in the event of an error.

*******************************************************************************/


static  opaque  decodeOpaque (

#    if PROTOTYPES
        const  char  *stringValue)
#    else
        stringValue)

        char  *stringValue ;
#    endif

{    /* Local variables. */
    int  numItems ;
    opaque  value ;



    numItems = sscanf (stringValue, "OPAQ%p", &value) ;
    if (numItems < 1) {
        SET_ERRNO (EINVAL) ;
        LGE "(decodeOpaque) Invalid string: \"%s\"\nsscanf: ", stringValue) ;
        value = NULL ;		/* Conversion error? */
    }

    return (value) ;

}

/*!*****************************************************************************

Procedure:

    encodeOpaque ()

    Encode an Opaque Value in a String.


Purpose:

    Function encodeOpaque() encodes an opaque value in a string using a format
    of "OPAQ%p", where "%p" is the sprintf() specifier for formatting pointers.


    Invocation:

        stringValue = encodeOpaque (value) ;

    where

        <value>		- I
            is the opaque (void *) value to be encoded.
        <stringValue>	- O
            returns a string containing the encoded opaque value.  Storage for
            the string belongs to encodeOpaque(); the string should be used or
            duplicated before calling encodeOpaque() again.

*******************************************************************************/


static  char  *encodeOpaque (

#    if PROTOTYPES
        opaque  value)
#    else
        value)

        opaque  value ;
#    endif

{    /* Local variables. */
    static  char  stringValue[32] ;



    sprintf (stringValue, "OPAQ%p", value) ;

    return (stringValue) ;

}
