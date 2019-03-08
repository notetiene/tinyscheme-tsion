/* $Id: funcs_lfn.c,v 1.2 2009/08/30 13:23:04 alex Exp $ */
/*******************************************************************************

File:

    funcs_lfn.c

    Line Feed-Terminated Networking Utilities.


Author:    Alex Measday


Purpose:

    The FUNCS_LFN package defines functions for sending and receiving
    LF-terminated text over a network connection.

        (lfn-create <endpoint> [<options>])	=> <stream>|#f
        (lfn-debug <value>)
        (lfn-destroy <stream>)			=> <status>   (#t|#f)
        (lfn-fd <stream>)			=> <integer>  (Socket)
        (lfn-getline <stream> [<timeout>])	=> <string>|#f
        (lfn-name <stream>)			=> <string>   (Connection name)
        (lfn-putline <stream> <string>
                     [<crlf> [<timeout>]])	=> <status>   (#t|#f)
        (lfn-read <stream> <length>
                  [<timeout>])			=> <string>|#f
        (lfn-readable? <stream>)		=> <flag>
        (lfn-up? <stream>)			=> <flag>
        (lfn-write <stream> <string>
                   [<timeout>])			=> <status>   (#t|#f)
        (lfn-writeable? <stream>)		=> <flag>


Public Procedures:

    addFuncsLFN() - registers the functions with the Scheme intepreter.

Private Procedures:

    func_LFN_CREATE() - implements the LFN-CREATE function.
    func_LFN_DEBUG() - implements the LFN-DEBUG function.
    func_LFN_DESTROY() - implements the LFN-DESTROY function.
    func_LFN_FD() - implements the LFN-FD function.
    func_LFN_GETLINE() - implements the LFN-GETLINE function.
    func_LFN_NAME() - implements the LFN-NAME function.
    func_LFN_PUTLINE() - implements the LFN-PUTLINE function.
    func_LFN_READ() - implements the LFN-READ function.
    func_LFN_READABLEp() - implements the LFN-READABLE? function.
    func_LFN_UPp() - implements the LFN-UP? function.
    func_LFN_WRITE() - implements the LFN-WRITE function.
    func_LFN_WRITEABLEp() - implements the LFN-WRITEABLE? function.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "lfn_util.h"			/* LF-terminated network I/O. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  pointer  func_LFN_CREATE P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_DEBUG P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_DESTROY P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_FD P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_GETLINE P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_NAME P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_PUTLINE P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_READ P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_READABLEp P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_UPp P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_WRITE P_((scheme *sc, pointer args)) ;
static  pointer  func_LFN_WRITEABLEp P_((scheme *sc, pointer args)) ;

/*!*****************************************************************************

Procedure:

    addFuncsLFN ()

    Register the LFN Functions with the Scheme Interpreter.


Purpose:

    Function addFuncsLFN() registers the LFN functions as foreign functions
    with the Scheme interpreter.


    Invocation:

        addFuncsLFN (sc) ;

    where

        <sc>	- I
            is the Scheme interpreter.

*******************************************************************************/


void  addFuncsLFN (

#    if PROTOTYPES
        scheme  *sc)
#    else
        sc)

        scheme  *sc ;
#    endif

{

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-create"),
                   mk_foreign_func (sc, func_LFN_CREATE)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-debug"),
                   mk_foreign_func (sc, func_LFN_DEBUG)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-destroy"),
                   mk_foreign_func (sc, func_LFN_DESTROY)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-fd"),
                   mk_foreign_func (sc, func_LFN_FD)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-getline"),
                   mk_foreign_func (sc, func_LFN_GETLINE)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-name"),
                   mk_foreign_func (sc, func_LFN_NAME)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-putline"),
                   mk_foreign_func (sc, func_LFN_PUTLINE)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-read"),
                   mk_foreign_func (sc, func_LFN_READ)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-readable?"),
                   mk_foreign_func (sc, func_LFN_READABLEp)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-up?"),
                   mk_foreign_func (sc, func_LFN_UPp)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-write"),
                   mk_foreign_func (sc, func_LFN_WRITE)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "lfn-writeable?"),
                   mk_foreign_func (sc, func_LFN_WRITEABLEp)) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_CREATE ()

    Create a LF-Terminated Network Stream.


Purpose:

    Function func_LFN_CREATE() creates a LF-terminated network stream.

        (lfn-create <endpoint> [<options>])

            Create a LF-terminated network stream on top of the previously
            created TCP/IP <endpoint>.  The stream takes ownership of
            <endpoint>, which will automatically be destroyed when the
            stream is destroyed.  The stream is returned as an opaque
            handle to the caller.

            The optional options string contains zero or more of the following
            UNIX command line-style options:

                "-input <size>"
                    specifies the size of the stream's internal input buffer;
                    the default is 2048 bytes.  NOTE that this is only a limit
                    on the input buffer, not on incoming strings.

                "-output <length>"
                    specifies the maximum output message size for the stream;
                    the default is 2047 bytes.


    Invocation:

        stream = func_LFN_CREATE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a previously created TCP/IP network
            datapoint returned by TCP-ANSWER or TCP-CALL/TCP-COMPLETE and
            an optional options string.
        <stream>	- O
            returns a LF-terminated network stream layered on top of the TCP
            data endpoint.  #f is returned in the event of an error.

*******************************************************************************/


static  pointer  func_LFN_CREATE (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *options ;
    LfnStream  stream ;
    pointer  argument ;
    TcpEndpoint  dataPoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        dataPoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_CREATE) Invalid datapoint specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    if (args == sc->NIL) {
        options = NULL ;
    } else {
        argument = car (args) ;
        options = strvalue (argument) ;
    }

/* Create the stream. */

    if (lfnCreate (dataPoint, options, &stream)) {
        LGE "(func_LFN_CREATE) Error creating LF-terminated network stream.\nlfnCreate: ") ;
        return (sc->F) ;
    }

/* Return the stream to the caller. */

    return (mk_opaque (sc, (opaque) stream)) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_DEBUG ()

    Enable/Disable LF-Terminated Networking Debug Output.


Purpose:

    Function func_LFN_DEBUG() enables or disables LF-terminated networking
    debug.

        (lfn-debug <value>)

            Set the LF-terminated networking debug flag to <value>,
            an integer number.  A value of 0 disables debug; a non-zero
            value enables debug.  Debug is written to standard output.


    Invocation:

        flag = func_LFN_DEBUG (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer indicating the desired
            debug level.
        <flag>		- O
            returns #t always.

*******************************************************************************/


static  pointer  func_LFN_DEBUG (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (!is_number (argument)) {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_DEBUG) Argument is not a number: ") ;
        return (sc->F) ;
    }

/* Set the debug level. */

    lfn_util_debug = is_real (argument) ? (int) rvalue (argument)
                                        : (int) ivalue (argument) ;

    return (sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_DESTROY ()

    Close a LF-Terminated Network Stream.


Purpose:

    Function func_LFN_DESTROY() deletes a LF-terminated network stream
    created by LFN-CREATE.

        (lfn-destroy <stream>)

        Close LF-terminated network <stream> and its underlying TCP/IP
        endpoint; <stream> should no longer be referenced.


    Invocation:

        status = func_LFN_DESTROY (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a LF-terminated network stream.
        <status>	- O
            returns #t if the stream was closed without error and
            #f otherwise.

*******************************************************************************/


static  pointer  func_LFN_DESTROY (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    pointer  argument ;
    LfnStream  stream ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        stream = (LfnStream) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_DESTROY) Argument is not a stream: ") ;
        return (sc->F) ;
    }

/* Close the stream. */

    return (lfnDestroy (stream) ? sc->F : sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_FD ()

    Get a LF-Terminated Stream's Socket.


Purpose:

    Function func_LFN_FD() returns the socket associated with a LF-terminated
    network stream.

        (lfn-fd <stream>)

        Get <stream>'s socket. The socket is an integer; in the event of
        an error, #f is returned.


    Invocation:

        fd = func_LFN_FD (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a LF-terminated network stream.
        <fd>		- O
            returns the stream's socket.

*******************************************************************************/


static  pointer  func_LFN_FD (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    pointer  argument ;
    LfnStream  stream ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        stream = (LfnStream) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_FD) Argument is not a stream: ") ;
        return (sc->F) ;
    }

/* Return the stream's socket to the caller. */

    return (mk_integer (sc, (long) lfnFd (stream))) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_GETLINE ()

    Read a Line of Input.


Purpose:

    Function func_LFN_GETLINE() reads the next, CR/LF-delimited line of input
    from a LF-terminated network stream.

        (lfn-getline stream [<timeout>])

        Read the next CR/LF-delimited line of input from <stream> and
        return the input as a string with the line terminators removed;
        #f is returned in the event of an error.

        LFN-GETLINE will wait at most <timeout> seconds for buffered or
        pending socket data to be available.  If <timeout> is not present
        or is negative, LFN-GETLINE will wait as long as necessary to read
        the next line of input; a zero timeout allows a read only if input
        is immediately available.  If <timeout> is specified and is greater
        than zero, LFN-GETLINE will wait that many seconds for the first
        piece of data to become available.  Once the first data arrives,
        LFN-GETLINE will take as long as necessary to read a full line of
        input, regardless of the timeout.


    Invocation:

        data = func_LFN_GETLINE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the LF-terminated network stream
            returned by LFN-CREATE and, optionally, a timeout value
            representing the number of seconds to wait for a line of data
            to be read.
        <data>		- O
            returns the input data in a string, minus the CR and LF characters.
            If an error occurs, #f is returned.

*******************************************************************************/


static  pointer  func_LFN_GETLINE (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *string ;
    double  timeout ;
    pointer  argument, data ;
    LfnStream  stream ;




/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        stream = (LfnStream) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_GETLINE) Invalid stream specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    if (args == sc->NIL) {
        timeout = -1.0 ;
    } else {
        argument = car (args) ;
        if (isInteger (argument)) {
            timeout = (double) ivalue (argument) ;
        } else if (isReal (argument)) {
            timeout = rvalue (argument) ;
        } else {
            SET_ERRNO (EINVAL) ;
            LGE "(func_LFN_GETLINE) Invalid timeout specification: ") ;
            return (sc->F) ;
        }
    }

/* Read the next input line from the network stream. */

    if (lfnGetLine (stream, timeout, &string)) {
        LGE "(func_LFN_GETLINE) Error reading input line from %s.\nlfnGetLine: ",
            lfnName (stream)) ;
        return (sc->F) ;
    }

/* Return the input line to the caller. */

    data = mk_string (sc, (const char *) string) ;

    return (data) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_NAME ()

    Get a LF-Terminated Stream's Name.


Purpose:

    Function func_LFN_NAME() returns the name of a LF-terminated network stream.

        (lfn-name <stream>)

        Return <stream>'s name as a string.  The name is in one of two formats:

            "<port>#<host>" - if the stream's underlying TCP/IP endpoit is a
                    server-side connection or
            "<port>@<host>" - if the underlying endpoint is a client--side
                    connection.


    Invocation:

        name = func_LFN_NAME (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a LF-terminated network stream.
        <name>		- O
            returns the stream's name.

*******************************************************************************/


static  pointer  func_LFN_NAME (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    pointer  argument ;
    LfnStream  stream ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        stream = (LfnStream) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_NAME) Argument is not a stream: ") ;
        return (sc->F) ;
    }

/* Return the stream's name to the caller. */

    return (mk_string (sc, lfnName (stream))) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_PUTLINE ()

    Write a Line of Output.


Purpose:

    Function func_LFN_PUTLINE() writes output to a LF-terminated network stream.

        (lfn-putline <stream> <string> [<crlf> [<timeout>]])

        Write a line of output, <string>, to <stream>; #t is returned
        if the write is successful and #f otherwise.

        Optional bit mask <crlf> specifies line terminators to be appended
        to the output string: 0 = no terminator, 1 = LF only, 2 = CR only,
        and 3 = CR/LF.  Zero (the default if this argument is not present)
        is typically used if the application explicitly puts the line
        terminators in the output string.

        LFN-PUTLINE will wait at most <timeout> seconds to begin writing
        <string>.  If this argument is not present or is negative, LFN-PUTLINE
        will wait as long as necessary to output all of the data.  A zero
        timeout specifies no wait: if <stream> is not ready for writing,
        LFN-PUTLINE returns immediately.  If the connection is ready for
        writing, LFN-PUTLINE returns after writing the entire string.

        If <timeout> is present and positive, LFN-PUTLINE will wait that many
        seconds to begin writing <string>.  Once output begins, LFN-PUTLINE
        will continue to write the entire string, regardless of the timeout
        value.


    Invocation:

        status = func_LFN_PUTLINE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the LF-terminated network stream
            returned by LFN-CREATE, the string of data to be output, an
            optional mask specifying which line terminators to append to
            the output string, and an optional timeout value representing
            the number of seconds to wait for a line of data to be written.
            The mask can have the following values; 0 = no terminator,
            1 = LF only, 2 = CR only, and 3 = CR/LF.  Zero is typically
            used if the application explicitly puts the line terminators
            in the output string.
        <status>	- O
            returns true (#t) if the data was successfully written and
            false (#f) otherwise.

*******************************************************************************/


static  pointer  func_LFN_PUTLINE (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *string ;
    double  timeout ;
    int  terminators ;
    LfnStream  stream ;
    long  length ;
    pointer  argument ;
    size_t  numBytesToWrite, numBytesWritten ;




/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        stream = (LfnStream) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_PUTLINE) Invalid stream specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (is_string (argument)) {
        string = strvalue (argument) ;
        length = strlength (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_PUTLINE) Invalid string specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    if (args == sc->NIL) {
        terminators = 0 ;
    } else if (isInteger (car (args))) {
        terminators = ivalue (car (args)) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_PUTLINE) Invalid terminator(s) specification: ") ;
        return (sc->F) ;
    }

    if (args != sc->NIL)  args = cdr (args) ;
    if (args == sc->NIL) {
        timeout = -1.0 ;
    } else {
        argument = car (args) ;
        if (isInteger (argument)) {
            timeout = (double) ivalue (argument) ;
        } else if (isReal (argument)) {
            timeout = rvalue (argument) ;
        } else {
            SET_ERRNO (EINVAL) ;
            LGE "(func_LFN_PUTLINE) Invalid timeout specification: ") ;
            return (sc->F) ;
        }
    }

/* Append the line terminators, if any. */

    string = strndup (string, length+2) ;
    if (string == NULL) {
        LGE "(func_LFN_PUTLINE) Error duplicating %ld-bye output string.\nstrndup: ",
            length) ;
        return (sc->F) ;
    }
    if (terminators & 0x02)  string[length++] = '\r' ;
    if (terminators & 0x01)  string[length++] = '\n' ;
    string[length] = '\0' ;

/* Write the line to the network stream. */

    numBytesToWrite = length ;
    if (lfnWrite (stream, timeout, numBytesToWrite, string, &numBytesWritten)) {
        LGE "(func_LFN_PUTLINE) Error writing output line to %s.\nlfnWrite: ",
            lfnName (stream)) ;
        return (sc->F) ;
    }

/* Return a successfull status to the caller. */

    return (sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_READ ()

    Read Unformatted Data.


Purpose:

    Function func_LFN_READ() reads unformatted data from a LF-terminated
    network stream.

        (lfn-read <stream> <length> [<timeout>])

        Read <length> bytes of arbitrary data from <stream> into a string
        buffer and return the buffer to the caller.  The data can be arbitrary
        binary data and can contain embedded NULs.

        Because of the way network I/O works, a single record written to a
        connection by one task may be read in multiple "chunks" by the task
        at the other end of the connection.  This is taken into account by
        LFN-READ and, if you ask it for 100 bytes, it will automatically
        perform however many network reads are necessary to collect the
        100 bytes.

        If <length> is negative, LFN-READ returns after reading the first
        "chunk" of input received; the number of bytes read from that first
        "chunk" is limited to the absolute value of <length>.  The actual
        string of bytes read is returned to the caller.

        LFN-READ will wait at most <timeout> seconds for the first data to
        arrive.  If <timeout> is not present or is negative, LFN-READ will
        wait as long as necessary to read the requested amount of data.  A
        zero timeout allows a read only if input is immediately available.
        If <timeout> is specified and is greater than zero, LFN-READ will
        return the amount of data actually read when <timeout> expires.
        Finally, in the event of an error, #f is returned.


    Invocation:

        data = func_LFN_READ (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the LF-terminated network stream
            returned by LFN-CREATE, a timeout value representing the number
            of seconds to wait for the desired amount of data to be read,
            and the number of bytes to read.
        <data>		- O
            returns the input data in a *counted* string; i.e., the number of
            bytes of data in the "string" is stored internally in the Scheme
            cell and is *not* dependent upon being a NUL-terminated string.
            If an error occurs, #f is returned.

*******************************************************************************/


static  pointer  func_LFN_READ (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *buffer ;
    double  timeout ;
    pointer  argument, data ;
    size_t  length, numBytesRead ;
    ssize_t  numBytesToRead ;
    LfnStream  stream ;




/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        stream = (LfnStream) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_READ) Invalid stream specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (isInteger (argument)) {
        numBytesToRead = (ssize_t) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_READ) Invalid length specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    if (args == sc->NIL) {
        timeout = -1.0 ;
    } else {
        argument = car (args) ;
        if (isInteger (argument)) {
            timeout = (double) ivalue (argument) ;
        } else if (isReal (argument)) {
            timeout = rvalue (argument) ;
        } else {
            SET_ERRNO (EINVAL) ;
            LGE "(func_LFN_READ) Invalid timeout specification: ") ;
            return (sc->F) ;
        }
    }

/* Read the data from the network stream. */

    length = (numBytesToRead < 0) ? -numBytesToRead : numBytesToRead ;
    buffer = calloc (length, 1) ;
    if (buffer == NULL) {
        LGE "(func_LFN_READ) Error allocating %lu-byte buffer.\ncalloc: ",
            (unsigned long) length) ;
        return (sc->F) ;
    }

    if (lfnRead (stream, timeout, numBytesToRead, buffer, &numBytesRead)) {
        LGE "(func_LFN_READ) Error reading %lu bytes from %s.\nlfnRead: ",
            (unsigned long) length, lfnName (stream)) ;
        PUSH_ERRNO ;  free (buffer) ;  POP_ERRNO ;
        return (sc->F) ;
    }

/* Return the input data to the caller as a *counted* string. */

    data = mk_bstring (sc, (const char *) buffer, numBytesRead) ;

    free (buffer) ;

    return (data) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_READABLEp ()

    Check if Data is Waiting to be Read from a LF-Terminated Network Stream.


Purpose:

    Function func_LFN_READABLEp() checks to see if data is waiting to be read
    from a LF-termianted network stream.

        (lfn-readable? <stream>)

        Check if data (either buffered or outstanding on the socket) is
        waiting to be read from <stream>.  Return #t if the predicate is
        true and #f otherwise.

        NOTE that LF-terminated input is buffered, so an application must keep
        reading and processing lines of input until LFN-READABLE? returns #f.


    Invocation:

        flag = func_LFN_READABLEp (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a LF-terminated network stream.
        <status>	- O
            returns #t if the stream is readable and #f otherwise.

*******************************************************************************/


static  pointer  func_LFN_READABLEp (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    pointer  argument ;
    LfnStream  stream ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        stream = (LfnStream) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_READABLEp) Argument is not a stream: ") ;
        return (sc->F) ;
    }

/* Check if the stream is readable. */

    return (lfnIsReadable (stream) ? sc->T : sc->F) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_UPp ()

    Check if a LF-Terminated Network Stream is Up.


Purpose:

    Function func_LFN_UPp() checks to see if a LF-terminated network stream
    is still up.

        (lfn-up? <stream>)

        Check if <stream>'s network connection is still up.
        Return #t if the predicate is true and #f otherwise.


    Invocation:

        flag = func_LFN_UPp (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a LF-terminated network stream.
        <status>	- O
            returns #t if the stream is up and #f otherwise.

*******************************************************************************/


static  pointer  func_LFN_UPp (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    pointer  argument ;
    LfnStream  stream ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        stream = (LfnStream) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_UPp) Argument is not a stream: ") ;
        return (sc->F) ;
    }

/* Check if the stream is up. */

    return (lfnIsUp (stream) ? sc->T : sc->F) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_WRITE ()

    Write Unformatted Data.


Purpose:

    Function func_LFN_WRITE() writes unformatted data to a LF-terminated
    network stream.

        (lfn-write <stream> <string> [<timeout>])

        Write arbitrary data from <string> to <stream>.

        Because of the way network I/O works, attempting to output a given
        amount of data to a network connection may require multiple network
        writers.  This is taken into account by LFN-WRITE and, if you ask it
        to output 100 bytes, it will perform however many network writes are
        necessary to output the full 100 bytes of data to the connection.

        LFN-WRITE will wait at most <timeout> seconds for the data to be
        output.  If this argument is not present or is negative, LFN-WRITE
        will wait as long as necessary to output all of the data.  A zero
        timeout specifies no wait: if <stream> is not ready for writing,
        LFN-WRITE returns immediately.  If the connection is ready for
        writing, LFN-WRITE returns after outputting whatever it can.

        If <timeout> is present and positive, LFN-WRITE outputs whatever it
        can in the given time interval.  In all cases, the number of bytes
        actually written is returned to the caller; #f is returned in the
        event of an error.


    Invocation:

        status = func_LFN_WRITE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the LF-terminated network stream
            returned by LFN-CREATE, a *counted* string of data to be output,
            and an optional timeout value representing the number of seconds
            to wait for the desired amount of data to be written.
        <status>	- O
            returns true (#t) if the data was successfully written and
            false (#f) otherwise.

*******************************************************************************/


static  pointer  func_LFN_WRITE (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *buffer ;
    double  timeout ;
    pointer  argument ;
    size_t  numBytesToWrite, numBytesWritten ;
    LfnStream  stream ;




/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        stream = (LfnStream) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_WRITE) Invalid stream specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (is_string (argument)) {
        buffer = strvalue (argument) ;
        numBytesToWrite = (size_t) strlength (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_WRITE) Invalid data specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    if (args == sc->NIL) {
        timeout = -1.0 ;
    } else {
        argument = car (args) ;
        if (isInteger (argument)) {
            timeout = (double) ivalue (argument) ;
        } else if (isReal (argument)) {
            timeout = rvalue (argument) ;
        } else {
            SET_ERRNO (EINVAL) ;
            LGE "(func_LFN_WRITE) Invalid timeout specification: ") ;
            return (sc->F) ;
        }
    }

/* Write the data to the network connection. */

    if (lfnWrite (stream, timeout, numBytesToWrite, buffer, &numBytesWritten)) {
        LGE "(func_LFN_WRITE) Error writing %lu bytes to %s.\nlfnWrite: ",
            (unsigned long) numBytesToWrite, lfnName (stream)) ;
        return (sc->F) ;
    }

    return (sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_LFN_WRITEABLEp ()

    Check if a LF-Terminated Network Stream is Ready for Writing.


Purpose:

    Function func_LFN_WRITEABLEp() checks to see if a LF-terminated network
    stream is ready for writing.

        (lfn-writeable? <stream>)

        Check if data can be written to <stream>.  Return
        #t if the predicate is true and #f otherwise.


    Invocation:

        flag = func_LFN_WRITEABLEp (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a LF-terminated network stream.
        <status>	- O
            returns #t if the stream is writeable and #f otherwise.

*******************************************************************************/


static  pointer  func_LFN_WRITEABLEp (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    pointer  argument ;
    LfnStream  stream ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        stream = (LfnStream) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_LFN_WRITEABLEp) Argument is not a stream: ") ;
        return (sc->F) ;
    }

/* Check if the stream is writeable. */

    return (lfnIsWriteable (stream) ? sc->T : sc->F) ;

}
