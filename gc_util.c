/* $Id: gc_util.c,v 1.1 2009/08/27 00:16:15 alex Exp alex $ */
/*******************************************************************************

File:

    gc_util.c

    Garbage Collection Utilities.


Author:    Alex Measday


Purpose:

    The GC_UTIL functions provide the means for protecting cached Scheme
    values, which become invisible to the garbage collecor, from being
    collected as garbage.  The functions do this by storing the values
    in an associative list (indexed by unique numerical IDs), thus making
    the values again visible to GC.

        [The following "problem" addressed by the GC_UTIL package
        turns out not to be a problem, but a misunderstanding on
        my part.  In an E-mail exchange, Dr. Jonathan Shapiro,
        the author of TinyScheme's garbage collector, informed
        me that objects are *not* relocated during GC.]

    Scheme values may also be relocated during GC.  The indirection provided
    by the associative list solves this problem.  When an application needs
    a cached Scheme value, the application retrieves it up ID from the
    associative list.  Since GC adjusts the pointers in the associative list
    if an value is relocated, the application can be sure of receiving the
    correct value.


Public Procedures:

    gc_protect() - protect a Scheme value from being collected as garbage.
    gc_retrieve() - retrieve a protected Scheme value by ID.
    gc_unprotect() - allow a Scheme value to be collected as garbage.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "plist_util.h"		/* TinyScheme property lists. */
#include  "gc_util.h"			/* Garbage collection utilities. */


int  gc_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  gc_util_debug

/*!*****************************************************************************

Procedure:

    gc_protect ()

    Protect a Scheme Value from GC.


Purpose:

    The gc_protect() function protects a cached Scheme value from being
    collected as garbage.  Whereas the value would otherwise appear to be
    unreferenced during GC, gc_protect() makes a reference by adding the
    value to the TSION-specific structure's ID map, an associative Scheme list.


    Invocation:

        id = gc_protect (sc, value) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <value>		- I
            is the Scheme value to be protected.
        <id>		- O
            returns a unique numeric ID for the protected value.  The program
            can later retrieve the value via its ID; see gc_retrieve().

*******************************************************************************/


UniqueID  gc_protect (

#    if PROTOTYPES
        scheme  *sc,
        pointer  value)
#    else
        sc, value)

        scheme  *sc ;
        pointer  value ;
#    endif

{    /* Local variables. */
    pointer  alist ;
    UniqueID  id ;



/* Get the value of the TSION ID map. */

    alist = plistGet (sc, "*tsion-id-map*", "alist") ;
    if (alist == NULL) {
        LGE "(gc_protect) Symbol *tsion-id-map*, property alist not found.\nplistGet: ") ;
        return (0) ;
    }

/* Prepend the new ID-to-value mapping. */

    id = TS (sc, idCounter++) ;
    alist = acons (sc, mk_integer (sc, id), value, alist) ;

/* Assign the new associative list to its variable. */

    plistPut (sc, "*tsion-id-map*", "alist", alist) ;

    LGI "(gc_protect)   ID: %ld  Value: %p\n", (long) id, (void *) value) ;

    return (id) ;

}

/*!*****************************************************************************

Procedure:

    gc_retrieve ()

    Retrieve a Protected Scheme Value by ID.


Purpose:

    The gc_retrieve() function retrieves a protected Scheme value by the ID
    that was returned by gc_protect().


    Invocation:

        value = gc_retrieve (sc, id) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <id>		- I
            is the unique numeric ID assigned to the protected value by
            gc_protect().
        <value>		- O
            returns the Scheme value bound to the ID.

*******************************************************************************/


pointer  gc_retrieve (

#    if PROTOTYPES
        scheme  *sc,
        UniqueID  id)
#    else
        sc, id)

        scheme  *sc ;
        UniqueID  id ;
#    endif

{    /* Local variables. */
    pointer  alist, binding, current, value ;



/* Get the value of the TSION ID map. */

    alist = plistGet (sc, "*tsion-id-map*", "alist") ;
    if (alist == NULL) {
        LGE "(gc_retrieve) Symbol *tsion-id-map*, property alist not found.\nplistGet: ") ;
        return (NULL) ;
    }

/* Traverse the list, looking for the element with the target ID. */

    for (current = alist ;  current != sc->NIL ;  current = cdr (current)) {
        binding = car (current) ;
        if (ivalue (car (binding)) == (long) id)  break ;
    }

    if (current == sc->NIL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gc_retrieve) ID %ld not found: ", (long) id) ;
        return (NULL) ;
    }

/* Return the value to the caller. */

    value = cdar (current) ;

    LGI "(gc_retrieve)  ID: %ld  Value: %p\n", (long) id, (void *) value) ;

    return (value) ;

}

/*!*****************************************************************************

Procedure:

    gc_unprotect ()

    Allow a Scheme Value to be Collected as Garbage.


Purpose:

    The gc_unprotect() function removes a previously protected Scheme value
    from the TSION ID map, thus making the value eligible for garbage
    collection.


    Invocation:

        gc_unprotect (sc, id) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <id>		- I
            is the unique numeric ID assigned to the protected value by
            gc_protect().

*******************************************************************************/


void  gc_unprotect (

#    if PROTOTYPES
        scheme  *sc,
        UniqueID  id)
#    else
        sc, id)

        scheme  *sc ;
        UniqueID  id ;
#    endif

{    /* Local variables. */
    pointer  alist, binding, current, previous ;



/* Get the value of the TSION ID map. */

    alist = plistGet (sc, "*tsion-id-map*", "alist") ;
    if (alist == NULL) {
        LGE "(gc_unprotect) Symbol *tsion-id-map*, property alist not found.\nplistGet: ") ;
        return ;
    }

/* Traverse the list, looking for the element with the target ID. */

    previous = NULL ;  current = alist ;

    while (current != sc->NIL) {
        binding = car (current) ;
        if (ivalue (car (binding)) == (long) id)  break ;
        previous = current ;  current = cdr (current) ;
    }

    if (current == sc->NIL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gc_unprotect) ID %ld not found: ", (long) id) ;
        return ;
    }

    if (previous == NULL) {
        alist = cdr (current) ;
        plistPut (sc, "*tsion-id-map*", "alist", alist) ;
    } else {
        set_cdr (previous, cdr (current)) ;
    }

    LGI "(gc_unprotect) ID: %ld  Value: %p\n",
        (long) id, (void *) cdar (current)) ;

    return ;

}
