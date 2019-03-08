/* $Id$ */
/*******************************************************************************

Program:

    tsiond

    Network TSION Server.


Author:    Alex Measday


Purpose:

    TSIOND is a network server that provides each client with its own
    TinyScheme interpreter.

    TSIOND creates its own I/O event dispatcher and assigns its handle to
    Scheme variable G-DISPATCHER.  Clients and scripts should use this
    dispatcher instead of creating their own via the IOX-CREATE function.


    Invocation:

        % tsiond [-debug] [-Debug] [-listen <port>]

    where:

        "-debug"
        "-Debug"
            enables debug output (written to STDOUT).  Capital "-Debug"
            generates more voluminous debug.
        "-listen <port>"
            specifies a network server port at which TSIOND will listen for and
            accept client connection requests.  A separate TSION interpreter is
            created for each new client and I/O is redirected to the client.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <signal.h>			/* Signal definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#if !defined(HAVE_DUP) || HAVE_DUP
#    if defined(vaxc)			/* DEC C has it in <unistd.h>. */
#        include  <unixio.h>		/* UNIX I/O definitions - dup(). */
#    elif defined(_WIN32)
#        include  <io.h>		/* Low-level I/O definitions - dup(). */
#    else
#        include  <unistd.h>		/* UNIX I/O definitions - dup(). */
#    endif
#endif
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "lfn_util.h"			/* LF-terminated network I/O. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "tpl_util.h"			/* Tuple utilities. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */
#include  "gc_util.h"			/* Garbage collection utilities. */
#include  "plist_util.h"		/* TinyScheme property lists. */


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  errno_t  newClientCB (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

static  errno_t  readClientCB (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

/*******************************************************************************
    TSIOND's Main Program.
*******************************************************************************/

int  main (

#    if PROTOTYPES
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

{    /* Local variables. */
    char  *argument ;
    int  errflg, option ;
    IoxDispatcher  dispatcher ;
    OptContext  scan ;
    TcpEndpoint  server ;

    const  char  *optionList[] = {	/* Command line options. */
        "{Debug}", "{debug}", "{listen:}", NULL
    } ;




#if HAVE_SIGNAL && defined(SIGPIPE)
    signal (SIGPIPE, SIG_IGN) ;
#endif
    aperror_print = 1 ;


/*******************************************************************************
    Create a global I/O event dispatcher.  Whenever a new Scheme interpreter
    is created, the dispatcher handle will be defined as variable G-DISPATCHER.
*******************************************************************************/

    if (ioxCreate (&dispatcher)) {
        LGE "[%s] Error creating I/O event dispatcher.\n", argv[0]) ;
        exit (errno) ;
    }


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    server = NULL ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-Debug" */
            tcp_util_debug = 1 ;
        case 2:			/* "-debug" */
            iox_util_debug = 1 ;
            lfn_util_debug = 1 ;
            break ;
        case 3:			/* "-listen <port>" */
            if (tcpListen (argument, -1, &server))
                errflg++ ;
            else if (NULL == ioxOnIO (dispatcher, newClientCB, (void *) server,
                                      IoxRead, tcpFd (server)))
                errflg++ ;
            break ;
        default:
            errflg++ ;  break ;
        }

    }

    opt_term (scan) ;

    if (errflg || (server == NULL)) {
        fprintf (stderr, "Usage:  tsiond [-debug] [-Debug] [-listen <port>]\n") ;
        exit (EINVAL) ;
    }


/*******************************************************************************
    Loop forever, processing input events as they occur.
*******************************************************************************/

    ioxMonitor (dispatcher, -1.0) ;

    exit (errno) ;

}

/*!*****************************************************************************

Procedure:

    newClientCB ()

    Answer Connection Requests from New Clients.


Purpose:

    Function newClientCB() answers network connection requests from new
    clients.  When a server port is created, the port's listening socket
    is registered with the IOX dispatcher as an input source.  Thereafter,
    when a connection request is received at the listening socket, the IOX
    dispatcher automatically invokes newClientCB() to accept the request
    and set up the new client.  The client's data connection is registered
    as an input source with the IOX dispatcher and a Scheme interpreter is
    created for the client.


    Invocation:

        status = newClientCB (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by ioxOnIO().
        <reason>	- I
            is the reason, IoxRead, the callback is being invoked.
        <userData>	- I
            is the TcpEndpoint for the listening port.
        <status>	- O
            returns the status of answering the connection request, zero if
            there were no errors and ERRNO otherwise.  The status value is
            ignored by the IOX dispatcher, but it may be of use if the
            application calls newClientCB() directly.

*******************************************************************************/


static  errno_t  newClientCB (

#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData)
#    else
        dispatcher, reason, userData)

        IoxCallback  callback ;
        IoxReason  reason ;
        void  *userData ;
#    endif

{    /* Local variables. */
    char  *fileName ;
    FILE  *file ;
    LfnStream  stream ;
#if !defined(HAVE_DUP) || HAVE_DUP
    port  *inputPort ;
#endif
    port  *outputPort ;
    scheme  *sc ;
    TcpEndpoint  client, server ;
    TsionSpecific  ts ;
    Tuple  tuple ;




    server = (TcpEndpoint) userData ;

/* Answer the connection request and create a LF-terminated network stream
   for the client. */

    if (tcpAnswer (server, -1.0, &client)) {
        LGE "(newClientCB) Error answering connection request: ") ;
        return (errno) ;
    }

    if (lfnCreate (client, NULL, &stream)) {
        LGE "(newClientCB) Error creating LF-terminated network stream: ") ;
        return (errno) ;
    }

/* Create a Scheme interpreter for the client. */

    sc = scheme_init_new ();
    if (sc == NULL) {
        LGE "(newClientCB) Error initializing Scheme engine for %s.\nscheme_init_new: ",
            tcpName (client)) ;
        return (errno) ;
    }

    ts = (TsionSpecific) calloc (sizeof (_TsionSpecific), 1) ;
    if (ts == NULL) {
        LGE "(newClientCB) Error allocating TSION-specific interpreter structure for %s.\ncalloc: ",
            tcpName (client)) ;
        return (errno) ;
    }
    scheme_set_external_data (sc, (void *) ts) ;

/* Define the ID map, "*tsion-id-map*".  Its value, initially an empty list,
   is stored as property "alist" in the ID map's property list.  Odd, but it
   allows C code to access the "value" of the ID map, which it can't otherwise
   do via TinyScheme itself. */

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "*tsion-id-map*"),
                   sc->NIL) ;
    plistPut (sc, "*tsion-id-map*", "alist", sc->NIL) ;

/* Redirect the I/O ports to use the client's socket. */

#if !defined(HAVE_DUP) || HAVE_DUP
    inputPort = (port *) malloc (sizeof (port)) ;
    if (inputPort == NULL) {
        LGE "(newClientCB) Error creating input port for %s.\nmalloc: ",
            tcpName (client)) ;
        return (errno) ;
    }

    inputPort->kind = port_file | port_input ;
    inputPort->rep.stdio.file = fdopen (dup (lfnFd (stream)), "r") ;
    inputPort->rep.stdio.closeit = 0 ;
    sc->inport = mk_port (sc, inputPort) ;
#else
    /* The OS/platform (e.g., Nintendo DS) doesn't support dup().  Use the
       file descriptor for the output port and leave the input port as stdin.
       The input callback, readClientCB(), doesn't use the input port anyway;
       instead, it calls scheme_load_string() to evaluate each line of input.
       Let's hope scripts and whatnot don't try reading from the input port! */
#endif

    outputPort = (port *) malloc (sizeof (port)) ;
    if (outputPort == NULL) {
        LGE "(newClientCB) Error creating output port for %s.\nmalloc: ",
            tcpName (client)) ;
        return (errno) ;
    }

    outputPort->kind = port_file | port_output ;
    outputPort->rep.stdio.file = fdopen (lfnFd (stream), "w") ;
    outputPort->rep.stdio.closeit = 0 ;
    sc->outport = mk_port (sc, outputPort) ;

#if USE_DL
    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "load-extension"),
                   mk_foreign_func (sc, scm_load_ext)) ;
#endif

/* Load the initialization file. */

    fileName = getenv ("TINYSCHEMEINIT") ;
    if (fileName == NULL)  fileName = "init.scm" ;
    file = fopen (fileName, "r") ;
    if (file == NULL) {
        LGE "(newClientCB) Error opening initialization file, \"%s\".\nfopen: ",
            fileName) ;
        exit (errno) ;
    }
    scheme_load_file (sc, file) ;
    fclose (file) ;

/* Add the TSION extensions. */

    addFuncsDRS (sc) ;
    addFuncsIOX (sc) ;
    addFuncsLFN (sc) ;
    addFuncsMISC (sc) ;
    addFuncsNET (sc) ;
    addFuncsREX (sc) ;
    addFuncsSKT (sc) ;
    addFuncsTCP (sc) ;

/* Define a variable for the global dispatcher. */

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "G-DISPATCHER"),
                   mk_opaque (sc, (void *) ioxDispatcher (callback))) ;

/* Print the Scheme command-line prompt. */

    putstr (sc, "> ") ;
    fflush (sc->outport->_object._port->rep.stdio.file) ;

/* Register the new client as an input source with the I/O event dispatcher. */

    tuple = tplCreate (2, (void *) sc, (void *) stream) ;
    if (tuple == NULL) {
        LGE "(newClientCB) Error creating tuple for %s.\ntplCreate: ",
            tcpName (client)) ;
        return (errno) ;
    }

    if (NULL == ioxOnIO (ioxDispatcher (callback), readClientCB,
                         (void *) tuple, IoxRead, lfnFd (stream))) {
        LGE "(newClientCB) Error registering client with I/O event dispatcher for %s.\nioxOnIO: ",
            tcpName (client)) ;
        return (errno) ;
    }


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    readClientCB ()

    Read and Evaluate Input from a Client.


Purpose:

    Function readClientCB() reads lines of Scheme input from a client's
    LF-terminated network connection and passes the lines to the client's
    Scheme interpreter for evaluation.  When the data connection to a client
    is established, the connection's socket is registered with the I/O event
    dispatcher as an input source.  Thereafter, when input is detected on the
    socket, the dispatcher automatically invokes readClientCB() to read and
    process the input.


    Invocation:

        status = readClientCB (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by ioxOnIO().
        <reason>	- I
            is the reason, IoxRead, the callback is being invoked.
        <userData>	- I
            is the address of a 2-tuple containing a pointer to the client's
            Scheme interpreter and the LfnStream for the client's network
            connection.
        <status>	- O
            returns the status of reading and processing the input, zero if
            there were no errors and ERRNO otherwise.  The status value is
            ignored by the IOX dispatcher, but it may be of use if the
            application calls readClientCB() directly.

*******************************************************************************/


static  errno_t  readClientCB (

#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData)
#    else
        dispatcher, reason, userData)

        IoxCallback  callback ;
        IoxReason  reason ;
        void  *userData ;
#    endif

{    /* Local variables. */
    char  *inbuf ;
    LfnStream  stream ;
    scheme  *sc ;
    Tuple  tuple ;




    tuple = (Tuple) userData ;
    sc = tplGet (tuple, 0) ;
    stream = tplGet (tuple, 1) ;

/* While more input is available, read and process the next input line. */

    while (lfnIsReadable (stream)) {
					/* Read the next message. */
        if (lfnGetLine (stream, -1.0, &inbuf)) {
            LGE "(readClientCB) Error reading from %s.\nlfnGetLine: ",
                lfnName (stream)) ;
            break ;
        }

        scheme_load_string (sc, inbuf) ;
        putstr (sc, "> ") ;
        fflush (sc->outport->_object._port->rep.stdio.file) ;

    }

/* Check to see if the stream's network connection has been broken.  If so,
   close the connection and destroy the client's Scheme interpreter. */

    if (!lfnIsUp (stream)) {
        errno = EPIPE ;
        LGE "(readClientCB) Broken connection to %s.\nlfnIsUp: ",
            lfnName (stream)) ;
        PUSH_ERRNO ;
        fclose (sc->outport->_object._port->rep.stdio.file) ;
        ioxCancel (callback) ;
        lfnDestroy (stream) ;
        scheme_deinit (sc) ;
        tplDestroy (tuple) ;
        POP_ERRNO ;
        return (errno) ;
    }

    return (0) ;

}
