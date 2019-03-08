/* $Id: funcs_skt.c,v 1.2 2009/08/30 13:23:04 alex Exp $ */
/*******************************************************************************

File:

    funcs_skt.c

    Socket Utilities.


Author:    Alex Measday


Purpose:

    The FUNCS_SKT package defines functions for manipulating sockets.

        (skt-cleanup)				=> <status>
        (skt-peer <fd>)				=> <string>  (Host name)
        (skt-port <fd>)				=> <integer> (Port number)
        (skt-readable? <fd>)			=> <flag>
        (skt-setbuf <fd> <recvSize> <sendSize>)	=> <status>
        (skt-startup)				=> <status>
        (skt-up? <fd>)				=> <flag>
        (skt-writeable? <fd>)			=> <flag>


Public Procedures:

    addFuncsSKT() - registers the functions with the Scheme intepreter.

Private Procedures:

    func_SKT_CLEANUP() - implements the SKT-CLEANUP function.
    func_SKT_PEER() - implements the SKT-PEER function.
    func_SKT_PORT() - implements the SKT-PORT function.
    func_SKT_READABLEp() - implements the SKT-READABLE? function.
    func_SKT_SETBUF() - implements the SKT-SETBUF function.
    func_SKT_STARTUP() - implements the SKT-STARTUP function.
    func_SKT_UPp() - implements the SKT-UP? function.
    func_SKT_WRITEABLEp() - implements the SKT-WRITEABLE? function.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "skt_util.h"			/* Socket utilities. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  pointer  func_SKT_CLEANUP P_((scheme *sc, pointer args)) ;
static  pointer  func_SKT_PEER P_((scheme *sc, pointer args)) ;
static  pointer  func_SKT_PORT P_((scheme *sc, pointer args)) ;
static  pointer  func_SKT_READABLEp P_((scheme *sc, pointer args)) ;
static  pointer  func_SKT_SETBUF P_((scheme *sc, pointer args)) ;
static  pointer  func_SKT_STARTUP P_((scheme *sc, pointer args)) ;
static  pointer  func_SKT_UPp P_((scheme *sc, pointer args)) ;
static  pointer  func_SKT_WRITEABLEp P_((scheme *sc, pointer args)) ;

/*!*****************************************************************************

Procedure:

    addFuncsSKT ()

    Register the SKT Functions with the Scheme Interpreter.


Purpose:

    Function addFuncsSKT() registers the SKT functions as foreign functions
    with the Scheme interpreter.


    Invocation:

        addFuncsSKT (sc) ;

    where

        <sc>	- I
            is the Scheme interpreter.

*******************************************************************************/


void  addFuncsSKT (

#    if PROTOTYPES
        scheme  *sc)
#    else
        sc)

        scheme  *sc ;
#    endif

{

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "skt-cleanup"),
                   mk_foreign_func (sc, func_SKT_CLEANUP)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "skt-peer"),
                   mk_foreign_func (sc, func_SKT_PEER)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "skt-port"),
                   mk_foreign_func (sc, func_SKT_PORT)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "skt-readable?"),
                   mk_foreign_func (sc, func_SKT_READABLEp)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "skt-setbuf"),
                   mk_foreign_func (sc, func_SKT_SETBUF)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "skt-startup"),
                   mk_foreign_func (sc, func_SKT_STARTUP)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "skt-up?"),
                   mk_foreign_func (sc, func_SKT_UPp)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "skt-writeable?"),
                   mk_foreign_func (sc, func_SKT_WRITEABLEp)) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    func_SKT_CLEANUP ()

    Shut Down the Socket Library.


Purpose:

    Function func_SKT_CLEANUP() shuts down the socket library on platforms
    that require it (e.g., Windows).

        (skt-cleanup)

        Shutdown the socket library; return #t upon success and #f otherwise.
        This function is not needed on most platforms.


    Invocation:

        status = func_SKT_CLEANUP (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments, which are ignored.
        <status>	- O
            returns the status of shutting down the socket library, #t if
            there were no errors and #f otherwise.

*******************************************************************************/


static  pointer  func_SKT_CLEANUP (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{

    return (sktCleanup () ? sc->F : sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_SKT_PEER ()

    Get the Host Name of a Socket's Peer.


Purpose:

    Function func_SKT_PEER() returns the host name (or dotted IP address)
    of the peer at the other end of a network conection.

        (skt-peer <fd>)

        Determine the host at the other end of network socket connection <fd>
        and returns its IP address as an integer in network-byte-order; #f is
        returned in the event of an error.


    Invocation:

        host = func_SKT_PEER (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer for the socket's file
            descriptor.
        <host>		- O
            returns the host name (or dotted IP address) of the peer at the
            other end of the network connection.

*******************************************************************************/


static  pointer  func_SKT_PEER (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    const  char  *peer ;
    IoFd  fd ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (isInteger (argument)) {
        fd = (IoFd) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_SKT_PEER) Socket is not an integer: ") ;
        return (sc->F) ;
    }

/* Get the socket's peer name. */

    peer = sktPeer (fd) ;
    if (peer == NULL) {
        LGE "(func_SKT_PEER) Error getting peer of socket %d: ", fd) ;
        return (sc->F) ;
    }

/* Return the host name to the caller. */

    return (mk_string (sc, peer)) ;

}

/*!*****************************************************************************

Procedure:

    func_SKT_PORT ()

    Get a Socket's Port Number.


Purpose:

    Function func_SKT_PORT() returns the number of the port to which a socket
    (either listening or data) is bound.

        (skt-port <fd>)

        Get the number of the port to which socket <fd> is bound;
        #f is returned in the event of an error.


    Invocation:

        port = func_SKT_PORT (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer for the socket's file
            descriptor.
        <port>		- O
            returns the number of the port to which the socket is bound;
            #f is returned in the event of an error.

*******************************************************************************/


static  pointer  func_SKT_PORT (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    int  port ;
    IoFd  fd ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (isInteger (argument)) {
        fd = (IoFd) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_SKT_PORT) Socket is not an integer: ") ;
        return (sc->F) ;
    }

/* Get the socket's port number. */

    port = sktPort (fd) ;
    if (port == 0) {
        LGE "(func_SKT_PORT) Error getting port number of socket %d: ", fd) ;
        return (sc->F) ;
    }

/* Return the port number to the caller. */

    return (mk_integer (sc, port)) ;

}

/*!*****************************************************************************

Procedure:

    func_SKT_READABLEp ()

    Check if Data is Waiting to be Read from a Socket.


Purpose:

    Function func_SKT_READABLEp() checks to see if data is waiting to be
    read from a socket.

        (skt-readable? <fd>)

        Check if data is waiting to be read from socket <fd>.
        Return #t if the predicate is true and #f otherwise.


    Invocation:

        flag = func_SKT_READABLEp (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer for the socket's file
            descriptor.
        <status>	- O
            returns #t if the socket is readable and #f otherwise.

*******************************************************************************/


static  pointer  func_SKT_READABLEp (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    IoFd  fd ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (isInteger (argument)) {
        fd = (IoFd) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_SKT_READABLEp) Socket is not an integer: ") ;
        return (sc->F) ;
    }

/* Check if the socket is readable. */

    return (sktIsReadable (fd) ? sc->T : sc->F) ;

}

/*!*****************************************************************************

Procedure:

    func_SKT_SETBUF ()

    Change the Sizes of a Socket's Receive and Send Buffers.


Purpose:

    Function func_SKT_SETBUF() changes the sizes of a socket's receive and/or
    send buffers.

        (skt-setbuf <fd> <recvSize> <sendSize>)

        Set the size of socket <fd>'s receive buffer to <recvSize> bytes
        and the size of its send buffer to <sendSize> bytes.  If a buffer
        size is less than zero, the respective buffer retains its current
        size.  Return #t if the buffer sizes were modified successfully
        and #f otherwise.


    Invocation:

        status = func_SKT_SETBUF (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer for the socket's file
            descriptor, an integer for the size of the receive buffer, and
            an integer for the size of the send buffer..
        <status>	- O
            returns the status of setting the socket's buffer sizes,
            #t if there were no errors and #f otherwise.

*******************************************************************************/


static  pointer  func_SKT_SETBUF (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    long  receiveSize, sendSize ;
    IoFd  fd ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (isInteger (argument)) {
        fd = (IoFd) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_SKT_SETBUF) Socket is not an integer: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (isInteger (argument)) {
        receiveSize = (long) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_SKT_SETBUF) Receive size is not an integer: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (isInteger (argument)) {
        sendSize = (long) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_SKT_SETBUF) Send size is not an integer: ") ;
        return (sc->F) ;
    }

/* Set the socket's buffer sizes. */

    if (sktSetBuf (fd, receiveSize, sendSize)) {
        LGE "(func_SKT_SETBUF) Error setting buffer sizes (%ld, %ld) for socket %d.\n",
            receiveSize, sendSize, fd) ;
        return (sc->F) ;
    }

/* Return the operation status to the caller. */

    return (sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_SKT_STARTUP ()

    Start Up the Socket Library.


Purpose:

    Function func_SKT_STARTUP() starts up the socket library on platforms
    that require it (e.g., Windows).

        (skt-startup)

        Startup the socket library; return #t upon success and #f otherwise.
        This function is not needed on most platforms.


    Invocation:

        status = func_SKT_STARTUP (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments, which are ignored.
        <status>	- O
            returns the status of starting up the socket library, #t if there
            were no errors and #f otherwise.

*******************************************************************************/


static  pointer  func_SKT_STARTUP (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{

    return (sktStartup () ? sc->F : sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_SKT_UPp ()

    Check if a Connection is Up.


Purpose:

    Function func_SKT_UPp() checks to see if a network connection is still up.

        (skt-up? <fd>)

        Check if <fd>'s network connection is still up.
        Return #t if the predicate is true and #f otherwise.

    Invocation:

        flag = func_SKT_UPp (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer for the socket's file
            descriptor.
        <status>	- O
            returns #t if the connection is up and #f otherwise.

*******************************************************************************/


static  pointer  func_SKT_UPp (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    IoFd  fd ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (isInteger (argument)) {
        fd = (IoFd) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_SKT_UPp) Socket is not an integer: ") ;
        return (sc->F) ;
    }

/* Check if the connection is up. */

    return (sktIsUp (fd) ? sc->T : sc->F) ;

}

/*!*****************************************************************************

Procedure:

    func_SKT_WRITEABLEp ()

    Check if Data can be Written to a Socket.


Purpose:

    Function func_SKT_WRITEABLEp() checks to see if data can be written
    to a socket.

        (skt-writeable? <fd>)

        Check if data can be written to socket <fd>.
        Return #t if the predicate is true and #f otherwise.


    Invocation:

        flag = func_SKT_WRITEABLEp (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer for the socket's file
            descriptor.
        <status>	- O
            returns #t if the socket is writeable and #f otherwise.

*******************************************************************************/


static  pointer  func_SKT_WRITEABLEp (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    IoFd  fd ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (isInteger (argument)) {
        fd = (IoFd) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_SKT_WRITEABLEp) Socket is not an integer: ") ;
        return (sc->F) ;
    }

/* Check if the socket is writeable. */

    return (sktIsWriteable (fd) ? sc->T : sc->F) ;

}
