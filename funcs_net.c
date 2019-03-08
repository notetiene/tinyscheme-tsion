/* $Id: funcs_net.c,v 1.2 2009/08/30 13:23:04 alex Exp $ */
/*******************************************************************************

File:

    funcs_net.c

    Network Utilities.


Author:    Alex Measday


Purpose:

    The FUNCS_NET package defines functions for translating IP addresses,
    host names, and service ports:

        (net-addr "<host>")		=> <integer> (IP address)
        (net-host <address> [dotted?])	=> <string>  (Host name)
        (net-port "<service>" [udp?])	=> <integer> (Port number)


Public Procedures:

    addFuncsNET() - registers the functions with the Scheme intepreter.

Private Procedures:

    func_NET_ADDR() - implements the NET-ADDR function.
    func_NET_HOST() - implements the NET-HOST function.
    func_NET_PORT() - implements the NET-PORT function.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "net_util.h"			/* Networking utilities. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  pointer  func_NET_ADDR P_((scheme *sc, pointer args)) ;
static  pointer  func_NET_HOST P_((scheme *sc, pointer args)) ;
static  pointer  func_NET_PORT P_((scheme *sc, pointer args)) ;

/*!*****************************************************************************

Procedure:

    addFuncsNET ()

    Register the NET Functions with the Scheme Interpreter.


Purpose:

    Function addFuncsNET() registers the NET functions as foreign functions
    with the Scheme interpreter.


    Invocation:

        addFuncsNET (sc) ;

    where

        <sc>	- I
            is the Scheme interpreter.

*******************************************************************************/


void  addFuncsNET (

#    if PROTOTYPES
        scheme  *sc)
#    else
        sc)

        scheme  *sc ;
#    endif

{

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "net-addr"),
                   mk_foreign_func (sc, func_NET_ADDR)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "net-host"),
                   mk_foreign_func (sc, func_NET_HOST)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "net-port"),
                   mk_foreign_func (sc, func_NET_PORT)) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    func_NET_ADDR ()

    Translate Host Name to IP Address.


Purpose:

    Function func_NET_ADDR() translates a host name to an IP address.
    The address is returned in network-byte-order.

        (net-addr "<host>")

        Lookup <host> and return its IP address as an integer in
        network-byte-order.


    Invocation:

        value = func_NET_ADDR (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments to the function: a string containing
            the host name.
        <value>		- O
            returns the value of the function: a 32-bit number containing the
            host's IP address in network-byte-order.

*******************************************************************************/


static  pointer  func_NET_ADDR (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *hostname ;
    pointer  argument, value ;
    unsigned  long  ipAddress ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_string (argument)) {
        hostname = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_NET_ADDR) Argument is not a string: ") ;
        return (sc->F) ;
    }

/* Translate the host name to its IP address. */

    ipAddress = netAddrOf (hostname) ;
    if (ipAddress == 0) {
        LGE "(func_NET_ADDR) \"%s\" not found.\nnetAddrOf: ", hostname) ;
        return (sc->F) ;
    }

/* Return the IP address to the caller. */

    value = mk_integer (sc, (long) ipAddress) ;

    return (value) ;

}

/*!*****************************************************************************

Procedure:

    func_NET_HOST ()

    Translate IP Address to Host Name.


Purpose:

    Function func_NET_HOST() looks up an IP address and returns the
    corresponding host name.

        (net-host <IP-address> [<dotted?>])

        Lookup <IP-address> (an integer in network-byte-order) and return the
        corresponding host name string.  If <dotted?> is present and #f, the
        address is returned in dotted IP format; otherwise, the host name is
        returned.


    Invocation:

        value = func_NET_HOST (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments to the function: an integer representing
            the IP address and, optionally, a flag indicating if the translation
            string is to be returned in dotted IP address format (#t) or host
            name (#f).
        <value>		- O
            returns the value of the function: the host name string.

*******************************************************************************/


static  pointer  func_NET_HOST (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    bool  dotted ;
    const  char  *hostname ;
    pointer  argument, value ;
    unsigned  long  ipAddress ;



/* Get the argument(s). */

    argument = car (args) ;
    if (isInteger (argument)) {
        ipAddress = (unsigned long) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_NET_HOST) IP address is not an integer: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    if (args == sc->NIL) {
        dotted = false ;
    } else {
        dotted = is_true (sc, car (args)) ;
    }

/* Translate the IP address to its host name. */

    hostname = netHostOf (ipAddress, dotted) ;

/* Return the host name to the caller. */

    value = mk_string (sc, hostname) ;

    return (value) ;

}

/*!*****************************************************************************

Procedure:

    func_NET_PORT ()

    Translate Service Name to Server Port.


Purpose:

    Function func_NET_PORT() looks up a server's name in the network services
    database (the "/etc/services" file) and returns the server's port number.

        (net-port "<service>" [<udp?>])

        Lookup <service> in the network services database (the "/etc/services"
        file) and return the corresponding port number.  If <udp?> is present
        and #t, the UDP port is returned; otherwise, the TCP port is returned.


    Invocation:

        value = func_NET_PORT (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments to the function: a string containing
            the service name and, optionally, a flag indicating if the UDP
            port (#t) or the TCP port (#f) is to be returned.
        <value>		- O
            returns the value of the function: the requested port number.

*******************************************************************************/


static  pointer  func_NET_PORT (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    bool  isUDP ;
    char  *service ;
    int  port ;
    pointer  argument, value ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_string (argument)) {
        service = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_NET_PORT) Service name is not a string: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    if (args == sc->NIL) {
        isUDP = false ;
    } else {
        isUDP = is_true (sc, car (args)) ;
    }

/* Translate the service name to its port number. */

    port = netPortOf (service, isUDP ? "udp" : "tcp") ;
    if (port < 0) {
        LGE "(func_NET_PORT) \"%s\" not found.\nnetPortOf: ", service) ;
        return (sc->F) ;
    }

/* Return the port number to the caller. */

    value = mk_integer (sc, (long) port) ;

    return (value) ;

}
