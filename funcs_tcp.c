/* $Id: funcs_tcp.c,v 1.2 2009/08/30 13:23:04 alex Exp alex $ */
/*******************************************************************************

File:

    funcs_tcp.c

    TCP/IP Utilities.


Author:    Alex Measday


Purpose:

    The FUNCS_TCP package defines functions for establishing and communicating
    over TCP/IP network connections:

        (tcp-answer <endpoint> [<timeout>])	=> <endpoint> (Data connection)
        (tcp-call "<service>[@<host>]"|<port>
                  [<noWait?>])			=> <endpoint> (Data connection)
        (tcp-complete <endpoint> [<timeout>
                      [<destroy?>]])		=> <status>   (#t|#f)
        (tcp-debug <value>)
        (tcp-destroy <endpoint>)		=> <status>   (#t|#f)
        (tcp-fd <endpoint>)			=> <integer>  (Socket)
        (tcp-listen "<service>"|<port>
                    [<backlog>])		=> <endpoint> (Listening port)
        (tcp-name <endpoint>)			=> <string>   (Connection name)
        (tcp-pending? <endpoint>)		=> <flag>
        (tcp-read <endpoint> <length>
                  [<timeout>])			=> <string>
        (tcp-readable? <endpoint>)		=> <flag>
        (tcp-up? <endpoint>)			=> <flag>
        (tcp-write <endpoint> <string>
                   [<timeout>])			=> <status>   (#t|#f)
        (tcp-writeable? <endpoint>)		=> <flag>


Public Procedures:

    addFuncsTCP() - registers the functions with the Scheme intepreter.

Private Procedures:

    func_TCP_ANSWER() - implements the TCP-ANSWER function.
    func_TCP_CALL() - implements the TCP-CALL function.
    func_TCP_COMPLETE() - implements the TCP-COMPLETE function.
    func_TCP_DEBUG() - implements the TCP-DEBUG function.
    func_TCP_DESTROY() - implements the TCP-DESTROY function.
    func_TCP_FD() - implements the TCP-FD function.
    func_TCP_LISTEN() - implements the TCP-LISTEN function.
    func_TCP_NAME() - implements the TCP-NAME function.
    func_TCP_PENDINGp() - implements the TCP-PENDING? function.
    func_TCP_READ() - implements the TCP-READ function.
    func_TCP_READABLEp() - implements the TCP-READABLE? function.
    func_TCP_UPp() - implements the TCP-UP? function.
    func_TCP_WRITE() - implements the TCP-WRITE function.
    func_TCP_WRITEABLEp() - implements the TCP-WRITEABLE? function.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  pointer  func_TCP_ANSWER P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_CALL P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_COMPLETE P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_DEBUG P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_DESTROY P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_FD P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_LISTEN P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_NAME P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_PENDINGp P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_READ P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_READABLEp P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_UPp P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_WRITE P_((scheme *sc, pointer args)) ;
static  pointer  func_TCP_WRITEABLEp P_((scheme *sc, pointer args)) ;

/*!*****************************************************************************

Procedure:

    addFuncsTCP ()

    Register the TCP Functions with the Scheme Interpreter.


Purpose:

    Function addFuncsTCP() registers the TCP functions as foreign functions
    with the Scheme interpreter.


    Invocation:

        addFuncsTCP (sc) ;

    where

        <sc>	- I
            is the Scheme interpreter.

*******************************************************************************/


void  addFuncsTCP (

#    if PROTOTYPES
        scheme  *sc)
#    else
        sc)

        scheme  *sc ;
#    endif

{

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-answer"),
                   mk_foreign_func (sc, func_TCP_ANSWER)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-call"),
                   mk_foreign_func (sc, func_TCP_CALL)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-complete"),
                   mk_foreign_func (sc, func_TCP_COMPLETE)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-debug"),
                   mk_foreign_func (sc, func_TCP_DEBUG)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-destroy"),
                   mk_foreign_func (sc, func_TCP_DESTROY)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-fd"),
                   mk_foreign_func (sc, func_TCP_FD)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-listen"),
                   mk_foreign_func (sc, func_TCP_LISTEN)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-name"),
                   mk_foreign_func (sc, func_TCP_NAME)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-pending?"),
                   mk_foreign_func (sc, func_TCP_PENDINGp)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-read"),
                   mk_foreign_func (sc, func_TCP_READ)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-readable?"),
                   mk_foreign_func (sc, func_TCP_READABLEp)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-up?"),
                   mk_foreign_func (sc, func_TCP_UPp)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-write"),
                   mk_foreign_func (sc, func_TCP_WRITE)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "tcp-writeable?"),
                   mk_foreign_func (sc, func_TCP_WRITEABLEp)) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_ANSWER ()

    Answer a Client's Connection Request.


Purpose:

    Function func_TCP_ANSWER() waits for and answers a client's request for
    a network connection.

        (tcp-answer <endpoint> [<timeout>])

        Wait at most <timeout> seconds for a connection request to be received
        on the listening <endpoint> (previously returned by TCP-LISTEN).
        If a request is received, accept the request.  The operating system
        automatically creates a new endpoint (the "data" endpoint) through
        which the server can talk to the client.  An opaque handle is returned
        for the new data endpoint; #f is returned in the event of an error.


    Invocation:

        endpoint = func_TCP_ANSWER (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the listening endpoint and, optionally,
            a timeout value representing the number of seconds to wait for a
            connection request to be received.
        <endpoint>	- O
            returns a data endpoint for the new client connection; #f is
            returned if an error occurs or if the timeout period elapses
            with no connection requests.

*******************************************************************************/


static  pointer  func_TCP_ANSWER (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    double  timeout ;
    pointer  argument ;
    TcpEndpoint  dataPoint, listeningPoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        listeningPoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_ANSWER) Invalid endpoint specification: ") ;
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
            LGE "(func_TCP_ANSWER) Invalid timeout specification: ") ;
            return (sc->F) ;
        }
    }

/* Wait for and answer the next connection request from a client. */

    if (tcpAnswer (listeningPoint, timeout, &dataPoint)) {
        LGE "(func_TCP_ANSWER) Error answering connection request.\ntcpAnswer: ") ;
        return (sc->F) ;
    }

/* Return the data endpoint to the caller. */

    return (mk_opaque (sc, (opaque) dataPoint)) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_CALL ()

    Request a Network Connection to a Server.


Purpose:

    Function func_TCP_CALL() is used by a client task to "call" a server task
    and request a network connection to the server.

        (tcp-call "<service>[@<host>]"|<port> [<noWait?>])

        Request a network connection to the server at "<service>[@<host>]".
        (Alternatively, you may specify a port number on the local host.)
        If the <noWait?> flag is not present or is #f, TCP-CALL waits until
        the connection is established (or refused) before returning.  If the
        <noWait?> flag is #t, TCP-CALL initiates the connection attempt and
        returns immediately; the application should subsequently invoke
        TCP-COMPLETE to complete the connection. In all cases, an opaque
        handle for the data endpoint is returned; #f is returned in the
        event of an error.


    Invocation:

        endpoint = func_TCP_CALL (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the port number and optional host name
            of the target network server and, optionally, a flag indicating
            whether (#f) or not (#t) the application should wait for the
            connection attempt to complete.
        <endpoint>	- O
            returns a data endpoint for the new client connection; #f is
            returned if an error occurs.  If the no-wait flag passed into the
            function was #t, then the application must later call TCP-COMPLETE
            to finish establishing the connection.

*******************************************************************************/


static  pointer  func_TCP_CALL (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    bool  noWait ;
    char  buffer[16], *server ;
    pointer  argument ;
    TcpEndpoint  dataPoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (isInteger (argument)) {
        sprintf (buffer, "%ld", ivalue (argument)) ;
        server = buffer ;
    } else if (is_string (argument)) {
        server = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_CALL) Invalid server specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    if (args == sc->NIL) {
        noWait = false ;
    } else {
        noWait = is_true (sc, car (args)) ? true : false ;
    }

/* Attempt to establish a connection to the server. */

    if (tcpCall (server, noWait, &dataPoint)) {
        LGE "(func_TCP_CALL) Error attempting to connect to \"%s\".\ntcpCall: ",
            server) ;
        return (sc->F) ;
    }

/* Return the data endpoint to the caller. */

    return (mk_opaque (sc, (opaque) dataPoint)) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_COMPLETE ()

    Complete a Call to a Server.


Purpose:

    Function func_TCP_COMPLETE() waits for an asynchronous, network connection
    attempt (initiated by TCP-CALL) to complete.

        (tcp-complete <endpoint> [<timeout> [<destroy?>])

        Wait for an asynchronous, network connection attempt to complete
        (if it is fated to complete).

        TCP-COMPLETE will wait at most <timeout> seconds for the call
        to complete.  If this argument is not present or is negative,
        TCP-COMPLETE will wait as long as necessary.  A timeout of zero
        seconds causes an immediate return if the connection is not yet
        established.

        If the connection attempt times out or otherwise fails and
        <destroy?> is not present or is #t, <endpoint> will automatically
        be destroyed.  This mode is useful when the application plans to
        make a single go/no-go call to TCP-COMPLETE.

        If, under the same circumstances, <destroy?> is #f, <endpoint> will not
        be destroyed; the application is responsible for executing TCP-DESTROY
        explicitly in the event of an error.  This mode is useful when the
        application plans to periodically call TCP-COMPLETE (perhaps with a
        timeout of zero) until the connection is successfully established.

        If a connection is not (or not yet) successfully established, #f is
        returned.


    Invocation:

        status = func_TCP_COMPLETE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the data endpoint returned by TCP-CALL,
            an optional timeout value representing the number of seconds to
            wait for the connection request to complete, and an optional flag
            indicating whether (#t) or not (#f) TCP-COMPLETE should destroy the
            data endpoint if the timeout period elapses before the connection
            is complete.
        <status>	- O
            returns true (#t) if the network connection was successfully
            established and false (#f) otherwise.

*******************************************************************************/


static  pointer  func_TCP_COMPLETE (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    bool  destroyOnError ;
    double  timeout ;
    pointer  argument ;
    TcpEndpoint  dataPoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        dataPoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_COMPLETE) Invalid endpoint specification: ") ;
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
            LGE "(func_TCP_COMPLETE) Invalid timeout specification: ") ;
            return (sc->F) ;
        }
    }

    if (args != sc->NIL)  args = cdr (args) ;
    if (args == sc->NIL) {
        destroyOnError = true ;
    } else {
        destroyOnError = is_true (sc, car (args)) ? true : false ;
    }

/* Wait for the connection to be established. */

    if (tcpComplete (dataPoint, timeout, destroyOnError)) {
        LGE "(func_TCP_COMPLETE) Error attempting to complete connection.\ntcpComplete: ") ;
        return (sc->F) ;
    }

/* The connection was successfully established.  Return #t to the caller. */

    return (sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_DEBUG ()

    Enable/Disable Networking Debug Output.


Purpose:

    Function func_TCP_DEBUG() enables or disables TCP/IP networking debug.

        (tcp-debug <value>)

        Set the TCP/IP networking debug flag to value, an integer number.
        A value of 0 disables debug; a non-zero value enables debug.
        Debug is written to standard output.


    Invocation:

        status = func_TCP_DEBUG (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer indicating the desired
            debug level.
        <status>	- O
            returns #t always.

*******************************************************************************/


static  pointer  func_TCP_DEBUG (

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
        LGE "(func_TCP_DEBUG) Argument is not a number: ") ;
        return (sc->F) ;
    }

/* Set the debug level. */

    tcp_util_debug = is_real (argument) ? (int) rvalue (argument)
                                        : (int) ivalue (argument) ;

    return (sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_DESTROY ()

    Close a Network Endpoint.


Purpose:

    Function func_TCP_DESTROY() deletes listening endpoints created by
    TCP-LISTEN and data endpoints created by TCP-ANSWER or TCP-CALL.

        (tcp-destroy <endpoint>)

        Close a listening or data endpoint; <endpoint> should no longer
        be referenced.


    Invocation:

        status = func_TCP_DESTROY (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a listening or data endpoint.
        <status>	- O
            returns #t if the endpoint was closed without error and
            #f otherwise.

*******************************************************************************/


static  pointer  func_TCP_DESTROY (

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
    TcpEndpoint  endpoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        endpoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_DESTROY) Argument is not an endpoint: ") ;
        return (sc->F) ;
    }

/* Close the endpoint. */

    return (tcpDestroy (endpoint) ? sc->F : sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_FD ()

    Get an Endpoint's Socket.


Purpose:

    Function func_TCP_FD() returns a listening or data endpoint's socket.

        (tcp-fd <endpoint>)

        Get <endpoint>'s socket; <endpoint< may be a listening or a data
        endpoint.  The socket is an integer; in the event of an error,
        #f is returned.


    Invocation:

        fd = func_TCP_FD (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a listening or data endpoint.
        <fd>		- O
            returns the endpoint's socket.

*******************************************************************************/


static  pointer  func_TCP_FD (

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
    TcpEndpoint  endpoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        endpoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_FD) Argument is not an endpoint: ") ;
        return (sc->F) ;
    }

/* Return the endpoint's socket to the caller. */

    return (mk_integer (sc, (long) tcpFd (endpoint))) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_LISTEN ()

    Listen for Network Connection Requests from Clients.


Purpose:

    Function func_TCP_LISTEN() creates a "listening" endpoint on which a
    network server can listen for connection requests from clients.

        (tcp-listen "<service>"|<port> [<backlog>])

        Create a "listening" endpoint bound to the named <service>
        port at which the application will listen for connection
        requests from clients.  (Alternatively, you may specify a
        <port> number directly.)  At most <backlog> requests may
        be pending; if this argument is not specified, a
        platform-specific maximum is used.  An opaque handle for
        the listening endpoint is returned for use in subsequent
        calls to TCP-ANSWER; #f is returned in the event of an error.


    Invocation:

        endpoint = func_TCP_LISTEN (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer or string indicating the
            service port and, optionally, an integer specifying the maximum
            backlog of pending clients.
        <endpoint>	- O
            returns a listening endpoint; #f is returned if an error occurs.

*******************************************************************************/


static  pointer  func_TCP_LISTEN (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  buffer[16], *service ;
    int  backlog ;
    pointer  argument ;
    TcpEndpoint  listeningPoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (isInteger (argument)) {
        sprintf (buffer, "%ld", ivalue (argument)) ;
        service = buffer ;
    } else if (is_string (argument)) {
        service = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_LISTEN) Invalid port specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    if (args == sc->NIL) {
        backlog = -1 ;
    } else if (isInteger (car (args))) {
        backlog = (int) ivalue (car (args)) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_LISTEN) Invalid backlog specification: ") ;
        return (sc->F) ;
    }

/* Create the listening endpoint. */

    if (tcpListen (service, backlog, &listeningPoint)) {
        LGE "(func_TCP_LISTEN) Error creating listening point: \"%s\"\ntcpListen: ",
            service) ;
        return (sc->F) ;
    }

/* Return the endpoint to the caller. */

    return (mk_opaque (sc, (opaque) listeningPoint)) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_NAME ()

    Get an Endpoint's Name.


Purpose:

    Function func_TCP_NAME() returns a listening or data endpoint's name.

        (tcp-name <endpoint>)

        Return <endpoint>'s name as a string.  The name is in one of three
        formats:

            "<port>" - if <endpoint> is a listening point,
            "<port>#<host>" - if <endpoint> is a server-side connection
                (i.e., created by TCP-ANSWER), or
            "<port>@<host>" - if <endpoint> is a client-side connection
                (i.e., created by TCP-CALL).


    Invocation:

        name = func_TCP_NAME (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a listening or data endpoint.
        <name>		- O
            returns the endpoint's name.

*******************************************************************************/


static  pointer  func_TCP_NAME (

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
    TcpEndpoint  endpoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        endpoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_NAME) Argument is not an endpoint: ") ;
        return (sc->F) ;
    }

/* Return the endpoint's name to the caller. */

    return (mk_string (sc, tcpName (endpoint))) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_PENDINGp ()

    Check a Listening Port for Pending Connection Requests.


Purpose:

    Function func_TCP_PENDINGp() checks to see if any connection requests are
    waiting to be answered on a listening endpoint.

        (tcp-pending? <endpoint>)

        Check if any connection requests from potential clients are waiting
        to be answered on listening <endpoint>.  Return #t if the predicate
        is true and #f otherwise.


    Invocation:

        flag = func_TCP_PENDINGp (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an endpoint for the listening port.
        <status>	- O
            returns #t if the connection is up and #f otherwise.

*******************************************************************************/


static  pointer  func_TCP_PENDINGp (

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
    TcpEndpoint  endpoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        endpoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_PENDINGp) Argument is not an endpoint: ") ;
        return (sc->F) ;
    }

/* Check if the endpoint has pending connection requests. */

    return (tcpRequestPending (endpoint) ? sc->T : sc->F) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_READ ()

    Read Data from a Network Connection.


Purpose:

    Function func_TCP_READ() reads data from a network connection.

        (tcp-read <endpoint> <length> [<timeout>])

        Read <length> bytes of arbitrary data from endpoint into a
        string buffer and return the buffer to the caller.  The data
        can be arbitrary binary data and can contain embedded NULs.

        Because of the way network I/O works, a single record written to a
        connection by one task may be read in multiple "chunks" by the task
        at the other end of the connection.  This is taken into account by
        TCP-READ and, if you ask it for 100 bytes, it will automatically
        perform however many network reads are necessary to collect the
        100 bytes.

        If length is negative, TCP-READ returns after reading the first
        "chunk" of input received; the number of bytes read from that
        first "chunk" is limited to the absolute value of <length>.
        The actual string of bytes read is returned to the caller.

        The function will wait at most <timeout> seconds for the first data
        to arrive.  If <timeout> is not present or is negative, TCP-READ
        will wait as long as necessary to read the requested amount of data.
        A timeout of zero allows a read only if input is immediately available.
        If <timeout> is specified and is greater than zero, TCP-READ will
        return the amount of data actually read when <timeout> expires.
        Finally, in the event of an error, #f is returned.


    Invocation:

        data = func_TCP_READ (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the data endpoint returned by TCP-CALL,
            the number of bytes to read, and, optionally, a timeout value
            representing the number of seconds to wait for the desired amount
            of data to be read.
        <data>		- O
            returns the input data in a *counted* string; i.e., the number of
            bytes of data in the "string" is stored internally in the Scheme
            cell and is *not* dependent upon being a NUL-terminated string.
            If an error occurs, #f is returned.

*******************************************************************************/


static  pointer  func_TCP_READ (

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
    TcpEndpoint  dataPoint ;




/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        dataPoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_READ) Invalid endpoint specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (isInteger (argument)) {
        numBytesToRead = (ssize_t) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_READ) Invalid length specification: ") ;
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
            LGE "(func_TCP_READ) Invalid timeout specification: ") ;
            return (sc->F) ;
        }
    }

/* Read the data from the network connection. */

    length = (numBytesToRead < 0) ? -numBytesToRead : numBytesToRead ;
    buffer = calloc (length, 1) ;
    if (buffer == NULL) {
        LGE "(func_TCP_READ) Error allocating %lu-byte buffer.\ncalloc: ",
            (unsigned long) length) ;
        return (sc->F) ;
    }

    if (tcpRead (dataPoint, timeout, numBytesToRead, buffer, &numBytesRead)) {
        LGE "(func_TCP_READ) Error reading %lu bytes from %s.\ntcpRead: ",
            (unsigned long) length, tcpName (dataPoint)) ;
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

    func_TCP_READABLEp ()

    Check if Data is Waiting to be Read from a Network Connection.


Purpose:

    Function func_TCP_READABLEp() checks to see if data is waiting to be
    read from a network connection.

        (tcp-readable? <endpoint>)

        Check if data is waiting to be read from <endpoint>.
        Return #t if the predicate is true and #f otherwise.


    Invocation:

        flag = func_TCP_READABLEp (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an endpoint for the network connection.
        <status>	- O
            returns #t if the connection is readable and #f otherwise.

*******************************************************************************/


static  pointer  func_TCP_READABLEp (

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
    TcpEndpoint  endpoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        endpoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_READABLEp) Argument is not an endpoint: ") ;
        return (sc->F) ;
    }

/* Check if the endpoint is readable. */

    return (tcpIsReadable (endpoint) ? sc->T : sc->F) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_UPp ()

    Check if a Network Connection is Up.


Purpose:

    Function func_TCP_UPp() checks to see if a network connection is still up.

        (tcp-up? <endpoint>)

        Check if <endpoint>'s network connection is still up.
        Return #t if the predicate is true and #f otherwise.


    Invocation:

        flag = func_TCP_UPp (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an endpoint for the network connection.
        <status>	- O
            returns #t if the connection is up and #f otherwise.

*******************************************************************************/


static  pointer  func_TCP_UPp (

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
    TcpEndpoint  endpoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        endpoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_UPp) Argument is not an endpoint: ") ;
        return (sc->F) ;
    }

/* Check if the endpoint is up. */

    return (tcpIsUp (endpoint) ? sc->T : sc->F) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_WRITE ()

    Write Data to a Network Connection.


Purpose:

    Function func_TCP_WRITE() writes data to a network connection.

        (tcp-write <endpoint> <string> [<timeout>])

        Write arbitrary data from <string> to <endpoint>.

        Because of the way network I/O works, attempting to output a given
        amount of data to a network connection may require multiple network
        writers.  This is taken into account by TCP-WRITE and, if you ask it
        to output 100 bytes, it will perform however many network writes are
        necessary to output the full 100 bytes of data to the connection.

        TCP-WRITE will wait at most <timeout> seconds for the data to be
        output.  If this argument is not present or is negative, TCP-WRITE
        will wait as long as necessary to output all of the data.  A zero
        timeout specifies no wait: if <endpoint> is not ready for writing,
        TCP-WRITE returns immediately.  If the connection is ready for
        writing, TCP-WRITE returns after outputting whatever it can.

        If <timeout> is present and positive, TCP-WRITE outputs whatever it
        can in the given time interval.  In all cases, the number of bytes
        actually written is returned to the caller; #f is returned in the
        event of an error.


    Invocation:

        length = func_TCP_WRITE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the data endpoint returned by TCP-CALL,
            a *counted* string of data to be output, and, optionally, a timeout
            value representing the number of seconds to wait for the desired
            amount of data to be written.
        <length>	- O
            returns the number of bytes successfully written, given any timeout
            contraint; false (#f) is returned if there was an error.

*******************************************************************************/


static  pointer  func_TCP_WRITE (

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
    TcpEndpoint  dataPoint ;




/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        dataPoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_WRITE) Invalid endpoint specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (is_string (argument)) {
        buffer = strvalue (argument) ;
        numBytesToWrite = (size_t) strlength (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_WRITE) Invalid data specification: ") ;
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
            LGE "(func_TCP_WRITE) Invalid timeout specification: ") ;
            return (sc->F) ;
        }
    }

/* Write the data to the network connection. */

    if (tcpWrite (dataPoint, timeout, numBytesToWrite, buffer,
                  &numBytesWritten)
        && (errno != EWOULDBLOCK)) {
        LGE "(func_TCP_WRITE) Error writing %lu bytes to %s.\ntcpWrite: ",
            (unsigned long) numBytesToWrite, tcpName (dataPoint)) ;
        return (sc->F) ;
    }

/* Return the number of bytes actually written. */

    return (mk_integer (sc, (long) numBytesWritten)) ;

}

/*!*****************************************************************************

Procedure:

    func_TCP_WRITEABLEp ()

    Check if a Network Connection is Ready for Writing.


Purpose:

    Function func_TCP_WRITEABLEp() checks to see if a network connection is
    ready for writing.

        (tcp-writeable? <endpoint>)

        Check if data can be written to <endpoint>.  Return #t if the
        predicate is true and #f otherwise.


    Invocation:

        flag = func_TCP_WRITEABLEp (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an endpoint for the network connection.
        <status>	- O
            returns #t if the connection is writeable and #f otherwise.

*******************************************************************************/


static  pointer  func_TCP_WRITEABLEp (

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
    TcpEndpoint  endpoint ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        endpoint = (TcpEndpoint) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_TCP_WRITEABLEp) Argument is not an endpoint: ") ;
        return (sc->F) ;
    }

/* Check if the endpoint is writeable. */

    return (tcpIsWriteable (endpoint) ? sc->T : sc->F) ;

}
