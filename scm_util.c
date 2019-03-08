/* $Id: scm_util.c,v 1.1 2009/08/27 00:16:15 alex Exp $ */
/*******************************************************************************

File:

    scm_util.c

    Scheme Utilities.


Author:    Alex Measday


Purpose:

    The SCM_UTIL functions fill various shortcomings in the core TinyScheme
    interpreter.


Public Procedures:

    mk_bstring() - make a binary string cell.
    mk_port() - make a port cell.
    string_push() - push a command string onto the input stack.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "scm_util.h"			/* Scheme utilities. */


int  scm_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  scm_util_debug

/*!*****************************************************************************

Procedure:

    mk_bstring ()

    Make a Binary String Cell.


Purpose:

    The mk_bstring() function is a replacement for TinyScheme's built-in
    mk_counted_string() function.  The latter function relies on the
    store_string() function, which assumes a NUL-terminated string (not
    counted) and uses strcpy(3) to copy the source string to the allocated
    string.  As a result, unless the counted string is NUL-terminated
    immediately after its last character, store_string() will write past
    the end of the allocated string.

    To fix this, mk_bstring() uses memmove(3) instead of strcpy(3).  The
    resulting Scheme value is still a counted string, but the string can
    contain NUL characters and need not be NUL-terminated.


    Invocation:

        cell = mk_bstring (sc, string, length) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <string>	- I
            is an array of arbitrary bytes that will be copied into a counted
            string.
        <length>	- I
            is the number of bytes in the string.
        <cell>		- O
            returns a Scheme string containing only the LENGTH bytes of STRING.

*******************************************************************************/


pointer  mk_bstring (

#    if PROTOTYPES
        scheme  *sc,
        const  char  *string,
        size_t  length)
#    else
        sc, string, length)

        scheme  *sc ;
        char  *string ;
        size_t  length ;
#    endif

{

/* TinyScheme's get_cell() function is declared "static" in "scheme.c",
   so a straightforward implementation of mk_bstring() is not possible.
   Instead, make a counted string using "" as the source string; then
   copy the actual source string to the Scheme string. */

    pointer  cell = mk_counted_string (sc, "", (int) length) ;

    memmove (strvalue (cell), string, length) ;

    (strvalue (cell))[length] = '\0' ;		/* mk_counted_string() allocates
						   length + 1 bytes. */

    return (cell) ;

}

/*!*****************************************************************************

Procedure:

    mk_port ()

    Make a Port Cell.


Purpose:

    The mk_port() function is an application-accessible copy of TinyScheme's
    static mk_port() function.


    Invocation:

        cell = mk_port (sc, pyort) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <pyort>		- I
            is a Scheme file or string port.
        <cell>		- O
            returns a Scheme cell containing the port.

*******************************************************************************/


pointer  mk_port (

#    if PROTOTYPES
        scheme  *sc,
        port  *pyort)
#    else
        sc, pyort)

        scheme  *sc ;
        port  *pyort ;
#    endif

{

/* TinyScheme's get_cell() function is declared "static" in "scheme.c",
   so use mk_character to get a cell and then convert it to a port cell. */

    pointer  cell = mk_character (sc, '\0') ;

    typeflag(cell) = T_PORT | T_ATOM ;
    cell->_object._port = pyort ;

    return (cell) ;

}

/*!*****************************************************************************

Procedure:

    string_push ()

    Push a Command String onto the Input Stack.


Purpose:

    The string_push() function ...


    Invocation:

        flag = string_push (sc, command) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <command>	- I
            is a command to be interpreted.
            string.
        <flag>		- O
            returns true if the command was succesfully pushed onto the input
            stack and false otherwise.

*******************************************************************************/


bool  string_push (

#    if PROTOTYPES
        scheme  *sc,
        const  char  *command)
#    else
        sc, command)

        scheme  *sc ;
        char  *command ;
#    endif

{

    sc->file_i++ ;

    sc->load_stack[sc->file_i].kind = port_input | port_string ;
    sc->load_stack[sc->file_i].rep.string.start = (char *) command ;
    sc->load_stack[sc->file_i].rep.string.past_the_end =
        (char *) command + strlen (command) ;
    sc->load_stack[sc->file_i].rep.string.curr = (char *) command ;
    sc->nesting_stack[sc->file_i]=0;
    sc->nesting = 1 ;
    sc->interactive_repl = 0 ;

    sc->inport = sc->loadport = mk_port (sc, &(sc->load_stack[sc->file_i])) ;

    return (true) ;

}
