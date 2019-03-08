/* $Id: funcs_drs.c,v 1.1 2009/09/01 11:31:31 alex Exp $ */
/*******************************************************************************

File:

    funcs_drs.c

    Directory Scanning Functions.


Author:    Alex Measday


Purpose:

    The FUNCS_DRS package defines functions for scanning the list of files
    in a directory.

        (drs-create "<pathname>")		=> <scan>|#f
        (drs-destroy <scan>)			=> <status>   (#t|#f)

        (drs-first <scan>)			=> <fileName>|#f
        (drs-next <scan>)			=> <fileName>|#f

        (drs-count <scan>)			=> <numFiles>
        (drs-get <scan> <index>)		=> <fileName>|#f


Public Procedures:

    addFuncsDRS() - registers the functions with the Scheme intepreter.

Private Procedures:

    func_DRS_CREATE() - implements the DRS-CREATE function.
    func_DRS_DESTROY() - implements the DRS-DESTROY function.
    func_DRS_FIRST() - implements the DRS-FIRST function.
    func_DRS_NEXT() - implements the DRS-NEXT function.
    func_DRS_COUNT() - implements the DRS-COUNT function.
    func_DRS_GET() - implements the DRS-GET function.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "drs_util.h"			/* Directory scanning utilities. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  pointer  func_DRS_CREATE P_((scheme *sc, pointer args)) ;
static  pointer  func_DRS_DESTROY P_((scheme *sc, pointer args)) ;
static  pointer  func_DRS_FIRST P_((scheme *sc, pointer args)) ;
static  pointer  func_DRS_NEXT P_((scheme *sc, pointer args)) ;
static  pointer  func_DRS_COUNT P_((scheme *sc, pointer args)) ;
static  pointer  func_DRS_GET P_((scheme *sc, pointer args)) ;

/*!*****************************************************************************

Procedure:

    addFuncsDRS ()

    Register the DRS Functions with the Scheme Interpreter.


Purpose:

    Function addFuncsDRS() registers the DRS functions as foreign functions
    with the Scheme interpreter.


    Invocation:

        addFuncsDRS (sc) ;

    where

        <sc>	- I
            is the Scheme interpreter.

*******************************************************************************/


void  addFuncsDRS (

#    if PROTOTYPES
        scheme  *sc)
#    else
        sc)

        scheme  *sc ;
#    endif

{

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "drs-create"),
                   mk_foreign_func (sc, func_DRS_CREATE)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "drs-destroy"),
                   mk_foreign_func (sc, func_DRS_DESTROY)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "drs-first"),
                   mk_foreign_func (sc, func_DRS_FIRST)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "drs-next"),
                   mk_foreign_func (sc, func_DRS_NEXT)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "drs-count"),
                   mk_foreign_func (sc, func_DRS_COUNT)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "drs-get"),
                   mk_foreign_func (sc, func_DRS_GET)) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    func_DRS_CREATE ()

    Create a Directory Scan.


Purpose:

    Function func_DRS_CREATE() creates a directory scan.

        (drs-create "<pathname>")

        Create a directory scan for the directory specified in <pathname>.
        The pathname must contain wildcard characters for the files in the
        directory, "*" at a minimum.  If the scan is successfully created,
        an opaque handle is returned for use in other DRS functions; #f is
        returned in the event of an error.


    Invocation:

        pattern = func_DRS_CREATE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a pathname string which must contain
            wildcard characters for the desired files in the directory.
        <scan>		- O
            returns the directory scan as an opaque handle; #f is returned
            in the event of an error.

*******************************************************************************/


static  pointer  func_DRS_CREATE (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *pathname ;
    DirectoryScan  scan ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_string (argument)) {
        pathname = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_DRS_CREATE) Argument is not a string: ") ;
        return (sc->F) ;
    }

/* Create the directory scan. */

    if (drsCreate (pathname, &scan)) {
        LGE "(func_DRS_CREATE) Error creating directory scan: \"%s\"\ndrsCreate: ",
            pathname) ;
        return (sc->F) ;
    }

/* Return the directory scan to the caller. */

    return (mk_opaque (sc, (opaque) scan)) ;

}

/*!*****************************************************************************

Procedure:

    func_DRS_DESTROY ()

    Destroy a Directory Scan.


Purpose:

    Function func_DRS_DESTROY() destroys a directory scan.

        (drs-destroy <scan>)

        Destroy the specified directory scan.


    Invocation:

        status = func_DRS_DESTROY (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the opaque handle for the directory
            scan being destroyed.
        <status>	- O
            returns true (#t) if the scan was destroyed successfully
            and false (#f) otherwise.

*******************************************************************************/


static  pointer  func_DRS_DESTROY (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    DirectoryScan  scan ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        scan = (DirectoryScan) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_DRS_DESTROY) Argument is not a scan: ") ;
        return (sc->F) ;
    }

/* Destroy the directory scan. */

    return (drsDestroy (scan) ? sc->F : sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_DRS_FIRST ()

    Get the First File in a Directory Scan.


Purpose:

    Function func_DRS_FIRST() returns the first matching file in a directory
    scan.

        (drs-first <scan>)

        Get the first matching file in a directory scan.  The file's full
        pathname is returned as a string; #f is returned if there are no
        matching files.


    Invocation:

        fileName = func_DRS_FIRST (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the opaque handle for the directory
            scan.
        <fileName>	- O
            returns the full pathname of the first matching file as a string;
            #f is returned if there were no matching files in the directory.

*******************************************************************************/


static  pointer  func_DRS_FIRST (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *fileName ;
    DirectoryScan  scan ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        scan = (DirectoryScan) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_DRS_FIRST) Argument is not a scan: ") ;
        return (sc->F) ;
    }

/* Get the name of the first matching file in the directory scan. */

    fileName = (char *) drsFirst (scan) ;

    return ((fileName == NULL) ? sc->F : mk_string (sc, fileName)) ;

}

/*!*****************************************************************************

Procedure:

    func_DRS_NEXT ()

    Get the Next File in a Directory Scan.


Purpose:

    Function func_DRS_NEXT() returns the next matching file in a directory
    scan.

        (drs-next <scan>)

        Get the next matching file in a directory scan.  The file's full
        pathname is returned as a string; #f is returned if there are no
        more matching files.


    Invocation:

        fileName = func_DRS_NEXT (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the opaque handle for the directory
            scan.
        <fileName>	- O
            returns the full pathname of the next matching file as a string;
            #f is returned if there were no more matching files in the
            directory.

*******************************************************************************/


static  pointer  func_DRS_NEXT (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *fileName ;
    DirectoryScan  scan ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        scan = (DirectoryScan) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_DRS_NEXT) Argument is not a scan: ") ;
        return (sc->F) ;
    }

/* Get the name of the next matching file in the directory scan. */

    fileName = (char *) drsNext (scan) ;

    return ((fileName == NULL) ? sc->F : mk_string (sc, fileName)) ;

}

/*!*****************************************************************************

Procedure:

    func_DRS_COUNT ()

    Get the Number of Files in a Directory Scan.


Purpose:

    Function func_DRS_COUNT() returns the number of files that match the
    pathname specification in a directory scan.

        (drs-count <scan>)

        Get the number of files in a directory scan that matched the wildcard
        file specification supplied to DRS-CREATE.


    Invocation:

        numFiles = func_DRS_COUNT (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the opaque handle for the directory
            scan.
        <numFiles>	- O
            returns the number of matching files in a directory scan.

*******************************************************************************/


static  pointer  func_DRS_COUNT (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    DirectoryScan  scan ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        scan = (DirectoryScan) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_DRS_COUNT) Argument is not a scan: ") ;
        return (sc->F) ;
    }

/* Get the count of matching files in the directory scan. */

    return (mk_integer (sc, (long) drsCount (scan))) ;

}

/*!*****************************************************************************

Procedure:

    func_DRS_GET ()

    Get the I-th File in a Directory Scan.


Purpose:

    Function func_DRS_GET() returns the I-th matching file in a directory
    scan.

        (drs-get <scan> <index>)

        Get the indexed, <index>, matching file in a directory scan.  Indices
        are numbered from 1 to the value returned by DRS-COUNT.  The file's
        full pathname is returned as a string; #f is returned if the index
        is out of range.  Getting a file name by index does not affect the
        sequence of file names returned by DRS-FIRST and DRS-NEXT.


    Invocation:

        fileName = func_DRS_GET (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the opaque handle for the directory
            scan and the index (1..N) of the file name to get..
        <fileName>	- O
            returns the full pathname of the I-th matching file as a string;
            #f is returned if the index is out of range.

*******************************************************************************/


static  pointer  func_DRS_GET (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *fileName ;
    DirectoryScan  scan ;
    int  index ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        scan = (DirectoryScan) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_DRS_GET) Argument is not a scan: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (isInteger (argument)) {
        index = (int) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_DRS_GET) Argument is not a number: ") ;
        return (sc->F) ;
    }

/* Get the I-th matching file in the scan. */

    fileName = (char *) drsGet (scan, index - 1) ;

    return ((fileName == NULL) ? sc->F : mk_string (sc, fileName)) ;

}
