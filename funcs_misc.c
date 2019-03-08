/* $Id: funcs_misc.c,v 1.2 2009/08/30 13:23:04 alex Exp $ */
/*******************************************************************************

File:

    funcs_misc.c

    Miscellaneous Utilities.


Author:    Alex Measday


Purpose:

    The FUNCS_MISC package defines a miscellaneous collection of unrelated
    functions:

        (getenv "<name>")	=> <string>  (Environment variable's value)
        (grab <value>)		=> <value>   (Scheme value)
        (tv-tod)		=> <pair>    (Time of day in secs and usecs)


Public Procedures:

    addFuncsMISC() - registers the functions with the Scheme intepreter.

Private Procedures:

    func_MISC_GETENV() - implements the GETENV function.
    func_MISC_GRAB() - implements the GRAB function.
    func_MISC_TV_TOD() - implements the TV-TOD function.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */
#include  "plist_util.h"		/* TinyScheme property lists. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  pointer  func_MISC_GETENV P_((scheme *sc, pointer args)) ;
static  pointer  func_MISC_GRAB P_((scheme *sc, pointer args)) ;
static  pointer  func_MISC_TV_TOD P_((scheme *sc, pointer args)) ;

/*!*****************************************************************************

Procedure:

    addFuncsMISC ()

    Register the MISC Functions with the Scheme Interpreter.


Purpose:

    Function addFuncsMISC() registers the MISC functions as foreign functions
    with the Scheme interpreter.


    Invocation:

        addFuncsMISC (sc) ;

    where

        <sc>	- I
            is the Scheme interpreter.

*******************************************************************************/


void  addFuncsMISC (

#    if PROTOTYPES
        scheme  *sc)
#    else
        sc)

        scheme  *sc ;
#    endif

{

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "getenv"),
                   mk_foreign_func (sc, func_MISC_GETENV)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "grab"),
                   mk_foreign_func (sc, func_MISC_GRAB)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tv-tod"),
                   mk_foreign_func (sc, func_MISC_TV_TOD)) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    func_MISC_GETENV ()

    Get an Environment Variable's Value.


Purpose:

    Function func_MISC_GETENV() looks up an environment variable and returns
    its value.

        (getenv "<name>")

        Get the value of environment variable <name>.  The value is returned
        as a string to the caller.  If the environment variable is not defined,
        then #f is returned.


    Invocation:

        value = func_MISC_GETENV (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments to the function: a string containing
            the environment variable's name.
        <value>		- O
            returns the environment variable's value as a string.  If the
            environment variable is not defined, #f is returned.

*******************************************************************************/


static  pointer  func_MISC_GETENV (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *name, *value ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_string (argument)) {
        name = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_MISC_GETENV) Variable's name is not a string: ") ;
        return (sc->F) ;
    }

/* Get the environment variable's value. */

    value = getenv (name) ;

/* Return the value to the caller. */

    return ((value == NULL) ? sc->F : mk_string (sc, (const char *) value)) ;

}

/*!*****************************************************************************

Procedure:

    func_MISC_GRAB ()

    Grab Scheme Value.


Purpose:

    Function func_MISC_GRAB() returns the Scheme argument(s) it was passed.
    TinyScheme doesn't offer an API call for a C program to retrieve the value
    of an arbitrary expression, so GRAB was written as a Scheme function to do
    just that.

    A pointer to the list of arguments received by GRAB is stored in the
    TSION-specific data structure (accessed via the "ext_data" field of the
    TinyScheme interpreter structure), overwriting the previous value, if any.
    Consequently, a C program can retrieve a value in the following steps:

        (1) Call scheme_load_string() with a "(grab ...)" command.
        (2) Retrieve the list of arguments using the TS(sc,grabValue) macro.

        (grab <value>)

        This function stores a pointer to value internally for use by C code.
        The function is not especially useful otherwise and simply returns its
        input, <value>.


    Invocation:

        value = func_MISC_GRAB (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments to the function, which are returned to
            the calling C function.
        <value>		- O
            returns the list of input arguments to the Scheme function.

*******************************************************************************/


static  pointer  func_MISC_GRAB (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{

    TS (sc, grabValue) = args ;

    return (args) ;

}

/*!*****************************************************************************

Procedure:

    func_MISC_TV_TOD ()

    Get the Current Time-of-Day (GMT).


Purpose:

    Function func_MISC_TV_TOD() returns the current time of day (GMT) as a
    pair consisting of the number of seconds and microseconds since the start
    of January 1, 1970.

        (tv-tod)

        Return the current time of day (GMT) as a pair consisting of the number
        of seconds and microseconds since the start of January 1, 1970.


    Invocation:

        value = func_MISC_TV_TOD (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments to the function, which are ignored.
        <value>		- O
            returns the current time-of-day as a pair consisting of a number
            of seconds and a number of microseconds.

*******************************************************************************/


static  pointer  func_MISC_TV_TOD (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{

    struct  timeval  tod = tvTOD () ;

    return (cons (sc, mk_integer (sc, (long) tod.tv_sec),
                      mk_integer (sc, (long) tod.tv_usec))) ;

}
