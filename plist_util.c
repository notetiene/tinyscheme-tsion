/* $Id: plist_util.c,v 1.2 2009/08/30 13:24:11 alex Exp alex $ */
/*******************************************************************************

File:

    plist_util.c

    TinyScheme Property List Utilities.


Author:    Alex Measday


Purpose:

    The PLIST_UTIL package is an external implementation of property lists
    that is compatible with TinyScheme's internal representation of such lists.
    The PLIST_UTIL property lists are accessible whether or not TinyScheme's
    lists are enabled (see the -DUSE_PLIST=0|1 compilation option).

    A symbol's property list is represented by a list of pair cells hanging
    off the symbol's cdr pointer.  Each pair cell, in turn, points to a
    property/value pair:

        Symbol ----------> Pair ----------> Pair ----------> sc->NIL
         |                  |                |
         V                  V                V
        String (name)      Pair             Pair
                          /    \           /    \
                     Property  Value  Property  Value
                       /                /
                  String (name)    String (name)

    New entries are inserted at the beginning of the list.  As with the
    TinyScheme implementation, when looking up a symbol/property, the property
    is matched by the T_SYMBOL token - returned by mk_symbol() - *not* by
    property name.  And, for some reason, property names are always converted
    to all lower-case.  (According to a comment in TinyScheme, the Scheme
    specification says that names are case-insensitive.)  For example, Scheme
    command "(put 'xyz 'SchemeRocks 67890)" results in "SchemeRocks" being
    stored in lower-case, "schemerocks", although "(get 'xyz 'SchemeRocks)"
    still returns the correct value.


Public Procedures:

    plistGet() - gets the value of a symbol's property.
    plistPut() - sets the value of a symbol's property.

Private Procedures:

    plistFind() - finds a property in a symbol's property list.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */
#include  "plist_util.h"		/* TinyScheme property lists. */


int  plist_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  plist_util_debug


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  pointer  plistFind (
#    if PROTOTYPES
        scheme  *sc,
        pointer  symbol,
        pointer  property
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    plistGet ()

    Get the Value of a Symbol's Property.


Purpose:

    Function plistGet() gets the value of the desired property of a symbol.


    Invocation:

        value = plistGet (sc, symbol, property) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <symbol>	- I
            is the symbol's name.  As with TinyScheme's symbol implementation,
            the name is converted to all lower-case internally.
        <property>	- I
            is the property's name.  As with TinyScheme's symbol implementation,
            the name is converted to all lower-case internally.
        <value>		- O
            returns a pointer to the Scheme cell whose contents are the value
            of the symbol's property; NULL is returned if the symbol has no
            such property.

*******************************************************************************/


pointer  plistGet (

#    if PROTOTYPES
        scheme  *sc,
        const  char  *symbol,
        const  char  *property)
#    else
        sc, symbol, property)

        scheme  *sc ;
        const  char  *symbol ;
        const  char  *property ;
#    endif

{    /* Local variables. */
    char  *s ;
    pointer  pair, propCell, symCell ;



/* Convert the symbol and property names to all lower-case.  The names are
   stored that way internally and mk_symbol() will return the wrong pointers
   if the names are not in all lower-case. */

    s = strdup (symbol) ;
    if (s == NULL) {
        LGE "(plistPut) Error duplicating symbol name: \"%s\"\nstrdup: ",
            symbol) ;
        return (NULL) ;
    }
    strlwr (s) ;
    symCell = mk_symbol (sc, s) ;
    free (s) ;

    s = strdup (property) ;
    if (s == NULL) {
        LGE "(plistFind) Error duplicating property name: \"%s\"\nstrdup: ",
            property) ;
        return (NULL) ;
    }
    strlwr (s) ;
    propCell = mk_symbol (sc, s) ;
    free (s) ;

/* Verify that the property is in the symbol's property list. */

    pair = plistFind (sc, symCell, propCell) ;
    if (pair == NULL) {
        SET_ERRNO (EINVAL) ;
        LGI "(plistGet) Symbol %s, property %s was not found.\nplistFind: ",
            symbol, property) ;
        return (NULL) ;
    }

/* Return the property's value to the caller. */

    return (cdr (pair)) ;

}

/*!*****************************************************************************

Procedure:

    plistPut ()

    Set the Value of a Symbol's Property.


Purpose:

    Function plistPut() sets the value of the desired property of a symbol.


    Invocation:

        status = plistPut (sc, symbol, property, value) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <symbol>	- I
            is the symbol's name.  As with TinyScheme's symbol implementation,
            the name is converted to all lower-case internally.
        <property>	- I
            is the property's name.  As with TinyScheme's symbol implementation,
            the name is converted to all lower-case internally.
        <value>		- I
            is a pointer to the Scheme cell whose contents are the new value of
            the symbol's property.
        <status>	- O
            returns the status of setting or adding the symbol's property value,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  plistPut (

#    if PROTOTYPES
        scheme  *sc,
        const  char  *symbol,
        const  char  *property,
        pointer  value)
#    else
        sc, symbol, property, value)

        scheme  *sc ;
        const  char  *symbol ;
        const  char  *property ;
        pointer  value ;
#    endif

{    /* Local variables. */
    char  *s ;
    pointer  pair, propCell, symCell ;



/* Convert the symbol and property names to all lower-case.  The names are
   stored that way internally and mk_symbol() will return the wrong pointers
   if the names are not in all lower-case. */

    s = strdup (symbol) ;
    if (s == NULL) {
        LGE "(plistPut) Error duplicating symbol name: \"%s\"\nstrdup: ",
            symbol) ;
        return (errno) ;
    }
    strlwr (s) ;
    symCell = mk_symbol (sc, s) ;
    free (s) ;

    s = strdup (property) ;
    if (s == NULL) {
        LGE "(plistFind) Error duplicating property name: \"%s\"\nstrdup: ",
            property) ;
        return (errno) ;
    }
    strlwr (s) ;
    propCell = mk_symbol (sc, s) ;
    free (s) ;

/* Check if the property is already in the symbol's property list. */

    pair = plistFind (sc, symCell, propCell) ;	/* Property/value pair. */

/* If the property is not yet in the symbol's property list, then add it. */

    if (pair == NULL) {
        pair = cons (sc, propCell, sc->NIL) ;	/* Property/value pair. */
        pair = cons (sc, pair, cdr (symCell)) ;	/* Property list entry. */
        cdr (symCell) = pair ;			/* Prepend entry to list. */
        pair = car (pair) ;			/* Back to property/value pair. */
    }

/* Now, replace the property's value with the new value. */

    cdr (pair) = value ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    plistFind ()

    Find a Property in a Symbol's Property List.


Purpose:

    Function plistFind() finds a desired property in a symbol's property list.


    Invocation:

        pair = plistFind (sc, symbol, property) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <symbol>	- I
            is a pointer to the symbol's T_SYMBOL cell.  This is retrieved
            by quoting the symbol name in a Scheme expression or by calling
            mk_symbol() from a C program.
        <property>	- I
            is a pointer to the property's T_SYMBOL cell.  This is retrieved
            by quoting the property name in a Scheme expression or by calling
            mk_symbol() from a C program.
        <pair>		- O
            returns a pointer to the pair cell whose car() is the property's
            T_SYMBOL cell and whose cdr() is the property's value.  NULL is
            returned if the symbol has no such property.

*******************************************************************************/


static  pointer  plistFind (

#    if PROTOTYPES
        scheme  *sc,
        pointer  symbol,
        pointer  property)
#    else
        sc, symbol, property)

        scheme  *sc ;
        pointer  symbol ;
        pointer  property ;
#    endif

{    /* Local variables. */
    pointer  pair ;



/* Search the symbol's list of properties for the desired property.  NOTE that
   the comparison between properties is *not* by name, but by the pointers to
   the properties' symbol cells. */

    for (pair = cdr (symbol) ;  pair != sc->NIL ;  pair = cdr (pair)) {
        if (caar (pair) == property)  break ;
    }

/* If the property was found, then return its property/value pair. */

    if (caar (pair) == property) {
        LGI "(plistFind) Symbol: %s  Property: %s  Value: %p\n",
            strvalue (car (symbol)), strvalue (car (property)),
            (void *) car (pair)) ;
        return (car (pair)) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGI "(plistFind) Symbol %s, property %s not found: \n",
            strvalue (car (symbol)), strvalue (car (property))) ;
        return (NULL) ;
    }

}
