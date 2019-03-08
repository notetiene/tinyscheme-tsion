/* $Id: funcs_iox.c,v 1.2 2009/08/30 13:23:04 alex Exp alex $ */
/*******************************************************************************

File:

    funcs_iox.c

    I/O Event Dispatching Utilities.


Author:    Alex Measday


Purpose:

    The FUNCS_IOX package defines functions for monitoring and responding to
    network I/O events.

        (iox-after <dp> <function> <user>
                   <seconds>)			=> <cb>|#f    (Callback)
        (iox-cancel <cb>)			=> <status>   (#t|#f)
        (iox-create)				=> <dp>|#f    (Dispatcher)
        (iox-debug <value>)
        (iox-destroy <dp>)			=> <status>   (#t|#f)
        (iox-dispatcher <cb>)			=> <dp>|#f    (Dispatcher)
        (iox-every <dp> <function> <user>
                   <delay> <interval>)		=> <cb>|#f    (Callback)
        (iox-monitor <dp> [<seconds>])		=> <status>   (#f)
        (iox-onio <dp> <function> <user>
                  <reason> <fd>)		=> <cb>|#f    (Callback)
        (iox-whenidle <dp> <function> <user>)	=> <cb>|#f    (Callback)


Public Procedures:

    addFuncsIOX() - registers the functions with the Scheme intepreter.

Private Procedures:

    func_IOX_AFTER() - implements the IOX-AFTER function.
    func_IOX_CANCEL() - implements the IOX-CANCEL function.
    func_IOX_CREATE() - implements the IOX-CREATE function.
    func_IOX_DEBUG() - implements the IOX-DEBUG function.
    func_IOX_DESTROY() - implements the IOX-DESTROY function.
    func_IOX_DISPATCHER() - implements the IOX-DISPATCHER function.
    func_IOX_EVERY() - implements the IOX-EVERY function.
    func_IOX_MONITOR() - implements the IOX-MONITOR function.
    func_IOX_ONIO() - implements the IOX-ONIO function.
    func_IOX_WHENIDLE() - implements the IOX-WHENIDLE function.
    funcIOXCB() - is a generic C callback function that calls the Scheme
        callback function when a monitored event occurs.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */
#include  "gc_util.h"			/* Garbage collection utilities. */


/*******************************************************************************
    SoxCallback - binds a Scheme function to an IOX callback.
*******************************************************************************/

typedef  struct  SoxCallback {
    IoxCallback  callback ;	/* The registered IOX callback. */
    scheme  *sc ;		/* Scheme interpreter. */
    UniqueID  functionID ;	/* ID bound to Scheme function. */
    UniqueID  userDataID ;	/* ID bound to Scheme user data. */
}  SoxCallback ;


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  pointer  func_IOX_AFTER P_((scheme *sc, pointer args)) ;
static  pointer  func_IOX_CANCEL P_((scheme *sc, pointer args)) ;
static  pointer  func_IOX_CREATE P_((scheme *sc, pointer args)) ;
static  pointer  func_IOX_DEBUG P_((scheme *sc, pointer args)) ;
static  pointer  func_IOX_DESTROY P_((scheme *sc, pointer args)) ;
static  pointer  func_IOX_DISPATCHER P_((scheme *sc, pointer args)) ;
static  pointer  func_IOX_EVERY P_((scheme *sc, pointer args)) ;
static  pointer  func_IOX_MONITOR P_((scheme *sc, pointer args)) ;
static  pointer  func_IOX_ONIO P_((scheme *sc, pointer args)) ;
static  pointer  func_IOX_WHENIDLE P_((scheme *sc, pointer args)) ;

static  errno_t  funcIOXCB (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    addFuncsIOX ()

    Register the IOX Functions with the Scheme Interpreter.


Purpose:

    Function addFuncsIOX() registers the IOX functions as foreign functions
    with the Scheme interpreter.


    Invocation:

        addFuncsIOX (sc) ;

    where

        <sc>	- I
            is the Scheme interpreter.

*******************************************************************************/


void  addFuncsIOX (

#    if PROTOTYPES
        scheme  *sc)
#    else
        sc)

        scheme  *sc ;
#    endif

{

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "iox-after"),
                   mk_foreign_func (sc, func_IOX_AFTER)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "iox-cancel"),
                   mk_foreign_func (sc, func_IOX_CANCEL)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "iox-create"),
                   mk_foreign_func (sc, func_IOX_CREATE)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "iox-debug"),
                   mk_foreign_func (sc, func_IOX_DEBUG)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "iox-destroy"),
                   mk_foreign_func (sc, func_IOX_DESTROY)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "iox-dispatcher"),
                   mk_foreign_func (sc, func_IOX_DISPATCHER)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "iox-every"),
                   mk_foreign_func (sc, func_IOX_EVERY)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "iox-monitor"),
                   mk_foreign_func (sc, func_IOX_MONITOR)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "iox-onio"),
                   mk_foreign_func (sc, func_IOX_ONIO)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "iox-whenidle"),
                   mk_foreign_func (sc, func_IOX_WHENIDLE)) ;

    scheme_load_string (sc, "(define IOX_READ 1)") ;
    scheme_load_string (sc, "(define IOX_WRITE 2)") ;
    scheme_load_string (sc, "(define IOX_EXCEPT 4)") ;
    scheme_load_string (sc, "(define IOX_IO 7)") ;
    scheme_load_string (sc, "(define IOX_FIRE 8)") ;
    scheme_load_string (sc, "(define IOX_IDLE 16)") ;

    return ;

}

/*!*****************************************************************************

Procedure:

    func_IOX_AFTER ()

    Register a Single-Shot Timer.


Purpose:

    Function func_IOX_AFTER() registers a single-shot timer with an I/O event
    dispatcher.

        (iox-after <dispatcher> <function> <userData> <delay>)

        Register a single-shot timer of <delay> seconds duration with
        <dispatcher>.  The timer interval can include a fractional number
        of seconds; e.g., 2.75 seconds.  An opaque handle for the registered
        callback is returned to the caller and can be used to cancel the
        callback with IOX-CANCEL cancel before the timer fires.  When <delay>
        seconds have elapsed, <function> is called with 3 arguments: the
        callback handle, the application-supplied <userData>, and the reason
        (IOX_FIRE) the callback is being invoked.  After being invoked, the
        single-shot timer is automatically canceled.


    Invocation:

        callback = func_IOX_AFTER (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the dispatcher, a function to be called
            when the timer fires, a user-supplied value to pass to the callback
            function, and the timer interval in seconds.  The user-supplied
            value can be an arbitrary Scheme value, so you can use CONS or LIST
            to pass multiple values to the callback.
        <callback>	- O
            returns a callback handle if the callback was successfully
            registered and #f if there was an error.

*******************************************************************************/


static  pointer  func_IOX_AFTER (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    double  interval ;
    IoxDispatcher  dispatcher ;
    pointer  argument, function, userData ;
    SoxCallback  *sox ;




/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        dispatcher = (IoxDispatcher) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_AFTER) Invalid dispatcher specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    function = argument ;

    args = cdr (args) ;
    argument = car (args) ;
    userData = argument ;

    args = cdr (args) ;
    argument = car (args) ;
    if (isInteger (argument)) {
        interval = (double) ivalue (argument) ;
    } else if (isReal (argument)) {
        interval = rvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_AFTER) Invalid interval specification: ") ;
        return (sc->F) ;
    }

/* Allocate a structure to hold the Scheme callback function and the user
   parameter; a pointer to this structure will be passed to funcIOXCB()
   when it is invoked for a callback. */

    sox = (SoxCallback *) malloc (sizeof (SoxCallback)) ;
    if (sox == NULL) {
        LGE "(func_IOX_AFTER) Error allocating SoxCallback structure.\nmalloc: ") ;
        return (sc->F) ;
    }

    sox->callback = NULL ;
    sox->sc = sc ;

/* Register the timer with the dispatcher.  When the specified interval has
   elapsed, the dispatcher will call funcIOXCB(), which, in turn, will call
   the Scheme function in the SoxCallback structure. */

    sox->callback = ioxAfter (dispatcher, funcIOXCB, sox, interval) ;
    if (sox->callback == NULL) {
        LGE "(func_IOX_AFTER) Error registering callback.\nioxAfter: ") ;
        PUSH_ERRNO ;  free (sox) ;  POP_ERRNO ;
        return (sc->F) ;
    }

/* Protect the function object and the user-supplied data from the garbage
   collector. */

    sox->functionID = gc_protect (sc, function) ;
    sox->userDataID = gc_protect (sc, userData) ;

/* Return the callback to the caller. */

    return (mk_opaque (sc, (opaque) sox->callback)) ;

}

/*!*****************************************************************************

Procedure:

    func_IOX_CANCEL ()

    Cancel a Callback.


Purpose:

    Function func_IOX_CANCEL() cancels a previously registered callback.

        (iox-cancel <callback>)

        Cancel <callback>, where <callback> is the opaque handle returned when
        the callback was registered.  The status of canceling the callback,
        #t or #f, is returned to the caller.


    Invocation:

        status = func_IOX_CANCEL (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the callback to be cancelled.
        <status>	- O
            returns true (#t) if the callback was cancelled successfully
            and false (#f) otherwise.

*******************************************************************************/


static  pointer  func_IOX_CANCEL (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    IoxCallback  callback ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        callback = (IoxCallback) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_CANCEL) Argument is not a callback: ") ;
        return (sc->F) ;
    }

/* Cancel the callback. */

    return (ioxCancel (callback) ? sc->F : sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_IOX_CREATE ()

    Create an I/O Event Dispatcher.


Purpose:

    Function func_IOX_CREATE() creates an I/O event dispatcher.

        (iox-create)

        Create an I/O event dispatcher.  An opaque handle is returned for the
        dispatcher; #f is returned in the event of an error.


    Invocation:

        dispatcher = func_IOX_CREATE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments, which are ignored.
        <dispatcher>	- O
            returns an I/O event dispatcher.

*******************************************************************************/


static  pointer  func_IOX_CREATE (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    IoxDispatcher  dispatcher ;



/* Create the dispatcher. */

    if (ioxCreate (&dispatcher)) {
        LGE "(func_IOX_CREATE) Error creating dispatcher.\nioxCreate: ") ;
        return (sc->F) ;
    }

/* Return the dispatcher to the caller. */

    return (mk_opaque (sc, (opaque) dispatcher)) ;

}

/*!*****************************************************************************

Procedure:

    func_IOX_DEBUG ()

    Enable/Disable I/O Event Dispatching Debug Output.


Purpose:

    Function func_IOX_DEBUG() enables or disables I/O event dispatching debug.

        (iox-debug <value>)

        Set the I/O event dispatcher debug flag to <value>, an integer number.
        A value of 0 disables debug; a non-zero value enables debug.  Debug is
        written to standard output.


    Invocation:

        func_IOX_DEBUG (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer indicating the desired
            debug level.

*******************************************************************************/


static  pointer  func_IOX_DEBUG (

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
        LGE "(func_IOX_DEBUG) Argument is not a number: ") ;
        return (sc->F) ;
    }

/* Set the debug level. */

    iox_util_debug = is_real (argument) ? (int) rvalue (argument)
                                        : (int) ivalue (argument) ;

    return (sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_IOX_DESTROY ()

    Destroy an I/O Event Dispatcher.


Purpose:

    Function func_IOX_DESTROY() destroys an I/O event dispatcher.

        (iox-destroy <dispatcher>)

        Invoke each of <dispatcher>'s registered callbacks with reason
        IOX_CANCEL and then destroy the dispatcher.


    Invocation:

        status = func_IOX_DESTROY (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the dispatcher to be destroyed.
        <status>	- O
            returns true (#t) if the dispatcher was destroyed successfully
            and false (#f) otherwise.

*******************************************************************************/


static  pointer  func_IOX_DESTROY (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    IoxDispatcher  dispatcher ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        dispatcher = (IoxDispatcher) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_DESTROY) Argument is not a dispatcher: ") ;
        return (sc->F) ;
    }

/* Destroy the dispatcher. */

    return (ioxDestroy (dispatcher) ? sc->F : sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_IOX_DISPATCHER ()

    Get a Callback's Dispatcher.


Purpose:

    Function func_IOX_DISPATCHER() returns the dispatcher with which the
    callback is registered.

        (iox-dispatcher <callback>)

        Get <callback>'s dispatcher, where <callback> is the opaque handle
        returned when the callback was registered.  This capability is useful
        when a callback needs to access its dispatcher.  For example,
        a callback that answers a network connection request may wish to
        register an I/O callback for the new data connection.


    Invocation:

        dispatcher = func_IOX_DISPATCHER (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the callback to be cancelled.
        <dispatcher>	- O
            returns the I/O event dispatcher with which the callback was
            registered; #f is returned in the event of an error.

*******************************************************************************/


static  pointer  func_IOX_DISPATCHER (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    IoxCallback  callback ;
    IoxDispatcher  dispatcher ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        callback = (IoxCallback) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_DISPATCHER) Argument is not a callback: ") ;
        return (sc->F) ;
    }

/* Get the callback's dispatcher. */

    dispatcher = ioxDispatcher (callback) ;

/* Return the dispatcher to the caller. */

    return ((dispatcher == NULL) ? sc->F
                                 : mk_opaque (sc, (opaque) dispatcher)) ;

}

/*!*****************************************************************************

Procedure:

    func_IOX_EVERY ()

    Register a Periodic Timer.


Purpose:

    Function func_IOX_EVERY() registers a periodic timer with an I/O event
    dispatcher.

        (iox-every <dispatcher> <function> <userData> <delay> <interval>)

        Register a periodic timer with <dispatcher>.  The timer fires
        after an initial <delay> number of seconds and then every
        <interval> seconds after that.  The time durations can include
        fractional numbers of seconds; e.g., 2.75 seconds.  An opaque
        handle for the registered callback is returned to the caller
        and can be used to cancel the callback with IOX-CANCEL when
        the timer is no longer needed.

        When the timer fires, <function> is called with 3 arguments:
        the callback handle, the application-supplied <userData>, and
        the reason (IOX_FIRE) the callback is being invoked.  To stop
        the timer, it must be explicitly canceled by calling IOX-CANCEL.


    Invocation:

        callback = func_IOX_EVERY (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the dispatcher, a function to be called
            when the timer fires, a user-supplied value to pass to the callback
            function, an initial timer delay in seconds, and the periodic
            interval in seconds.  The user-supplied value can be an arbitrary
            Scheme value, so you can use CONS or LIST to pass multiple values
            to the callback.
        <callback>	- O
            returns a callback handle if the callback was successfully
            registered and #f if there was an error.

*******************************************************************************/


static  pointer  func_IOX_EVERY (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    double  delay, interval ;
    IoxDispatcher  dispatcher ;
    pointer  argument, function, userData ;
    SoxCallback  *sox ;




/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        dispatcher = (IoxDispatcher) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_EVERY) Invalid dispatcher specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    function = argument ;

    args = cdr (args) ;
    argument = car (args) ;
    userData = argument ;

    args = cdr (args) ;
    argument = car (args) ;
    if (isInteger (argument)) {
        delay = (double) ivalue (argument) ;
    } else if (isReal (argument)) {
        delay = rvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_EVERY) Invalid delay specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (isInteger (argument)) {
        interval = (double) ivalue (argument) ;
    } else if (isReal (argument)) {
        interval = rvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_EVERY) Invalid interval specification: ") ;
        return (sc->F) ;
    }

/* Allocate a structure to hold the Scheme callback function and the user
   parameter; a pointer to this structure will be passed to funcIOXCB()
   when it is invoked for a callback. */

    sox = (SoxCallback *) malloc (sizeof (SoxCallback)) ;
    if (sox == NULL) {
        LGE "(func_IOX_EVERY) Error allocating SoxCallback structure.\nmalloc: ") ;
        return (sc->F) ;
    }

    sox->callback = NULL ;
    sox->sc = sc ;

/* Register the timer with the dispatcher.  When the specified interval has
   elapsed, the dispatcher will call funcIOXCB(), which, in turn, will call
   the Scheme function in the SoxCallback structure. */

    sox->callback = ioxEvery (dispatcher, funcIOXCB, sox, delay, interval) ;
    if (sox->callback == NULL) {
        LGE "(func_IOX_EVERY) Error registering callback.\nioxEvery: ") ;
        PUSH_ERRNO ;  free (sox) ;  POP_ERRNO ;
        return (sc->F) ;
    }

/* Save the function object and the user-supplied data so that they remain
   visible to the garbage collector and are, thus, not garbage collected. */

    sox->functionID = gc_protect (sc, function) ;
    sox->userDataID = gc_protect (sc, userData) ;

/* Return the callback to the caller. */

    return (mk_opaque (sc, (opaque) sox->callback)) ;

}

/*!*****************************************************************************

Procedure:

    func_IOX_MONITOR ()

    Monitor I/O Events.


Purpose:

    Function func_IOX_MONITOR() passes control to a dispatcher to monitor I/O events, timers, and
    idle tasks.

        (iox-monitor <dispatcher> [<timeout>])

        Monitor and dispatch I/O events, timers, and idle tasks for
        <timeout> seconds using <dispatcher>.  If the timeout argument
        is not present or is less than zero, the dispatcher will monitor
        events forever.  When the timeout interval is complete, IOX-MONITOR
        returns #t.  In the event of an error (usually no more events to
        monitor), #f is returned.


    Invocation:

        status = func_IOX_MONITOR (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the dispatcher doing the monitoring
            and an optional timeout in seconds after which the dispatcher
            returns to the caller.
        <status>	- O
            returns true (#t) if the dispatcher was monitoring events without
            errors and false (#f) otherwise.  NOTE that IOX-MONITOR only returns
            if the timeout period elapses (so the status would be #t) or if an
            error was detected (so status would be #f).

*******************************************************************************/


static  pointer  func_IOX_MONITOR (

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
    IoxDispatcher  dispatcher ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        dispatcher = (IoxDispatcher) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_MONITOR) Argument is not a dispatcher: ") ;
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
            LGE "(func_IOX_MONITOR) Invalid timeout specification: ") ;
            return (sc->F) ;
        }
    }

/* Monitor events for the specified interval. */

    return (ioxMonitor (dispatcher, timeout) ? sc->F : sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_IOX_ONIO ()

    Register an I/O Source.


Purpose:

    Function func_IOX_ONIO() registers an I/O source (e.g., a network
    connection) with an I/O event dispatcher.

        (iox-onio <dispatcher> <function> <userData> <reason> <fd>)

        Register I/O file descriptor <fd> with <dispatcher>.  Mask <reason>
        is the bit-wise OR of the types of I/O events to monitor: IOX_READ
        for input-pending, IOX_WRITE for output-ready, and IOX_EXCEPT for
        OOB-input-pending.  An opaque handle for the registered callback
        is returned to the caller and can be used to cancel the callback
        with IOX-CANCEL.

        When an event of the monitored types is detected on the I/O source,
        <function> is called with 3 arguments: the callback handle, the
        application-supplied <userData>, and the reason (IOX_READ, IOX_WRITE,
        or IOX_EXCEPT) the callback is being invoked.

        To register a callback for a TCP/IP listening socket (created by
        TCP-LISTEN), specify IOX_READ as the event type to be monitored.
        When a connection request is received from a client, the listening
        socket becomes readable.  The callback should then execute TCP-ANSWER
        to accept the connection request.


    Invocation:

        callback = func_IOX_ONIO (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the dispatcher, a function to be called
            when a specified type of event is detected on the I/O source,
            a user-supplied value to pass to the callback function, the type
            of event (IOX_READ, IOX_WRITE, IOX_EXCEPT, or IOX_IO) for which
            the I/O source is to be monitored, and the file descriptor for the
            I/O source.  The user-supplied value can be an arbitrary Scheme
            value, so you can use CONS or LIST to pass multiple values to the
            callback.
        <callback>	- O
            returns a callback handle if the callback was successfully
            registered and #f if there was an error.

*******************************************************************************/


static  pointer  func_IOX_ONIO (

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
    IoxDispatcher  dispatcher ;
    IoxReason  reason ;
    pointer  argument, function, userData ;
    SoxCallback  *sox ;




/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        dispatcher = (IoxDispatcher) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_ONIO) Invalid dispatcher specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    function = argument ;

    args = cdr (args) ;
    argument = car (args) ;
    userData = argument ;

    args = cdr (args) ;
    argument = car (args) ;
    if (isInteger (argument)) {
        reason = (IoxReason) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_ONIO) Invalid reason specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (isInteger (argument)) {
        fd = (IoFd) ivalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_ONIO) Invalid file descriptor specification: ") ;
        return (sc->F) ;
    }

/* Allocate a structure to hold the Scheme callback function and the user
   parameter; a pointer to this structure will be passed to funcIOXCB()
   when it is invoked for a callback. */

    sox = (SoxCallback *) malloc (sizeof (SoxCallback)) ;
    if (sox == NULL) {
        LGE "(func_IOX_ONIO) Error allocating SoxCallback structure.\nmalloc: ") ;
        return (sc->F) ;
    }

    sox->callback = NULL ;
    sox->sc = sc ;

/* Register the I/O source with the dispatcher.  When an I/O event of the
   specified type is detected on the source, the dispatcher will call
   funcIOXCB(), which, in turn, will call the Scheme function in the
   SoxCallback structure. */

    sox->callback = ioxOnIO (dispatcher, funcIOXCB, sox, reason, fd) ;
    if (sox->callback == NULL) {
        LGE "(func_IOX_ONIO) Error registering callback.\nioxOnIO: ") ;
        PUSH_ERRNO ;  free (sox) ;  POP_ERRNO ;
        return (sc->F) ;
    }

/* Save the function object and the user-supplied data so that they remain
   visible to the garbage collector and are, thus, not garbage collected. */

    sox->functionID = gc_protect (sc, function) ;
    sox->userDataID = gc_protect (sc, userData) ;

/* Return the callback to the caller. */

    return (mk_opaque (sc, (opaque) sox->callback)) ;

}

/*!*****************************************************************************

Procedure:

    func_IOX_WHENIDLE ()

    Register an Idle Task.


Purpose:

    Function func_IOX_WHENIDLE() registers an idle task with an I/O event
    dispatcher.

        (iox-whenidle <dispatcher> <function> <userData>)

        Register an idle task with dispatcher.  An opaque handle for the
        registered callback is returned to the caller and can be used to
        cancel the callback with IOX-CANCEL.  When the dispatcher is idle
        (i.e., no I/O or timers immediately pending), <function> is called
        with 3 arguments: the callback handle, the application-supplied
        <userData>, and the reason (IOX_IDLE) the callback is being invoked.
        After <function> completes, <dispatcher> re-queues the idle task for
        later processing.  The application is responsible for explicitly
        canceling the callback using IOX-CANCEL.


    Invocation:

        callback = func_IOX_WHENIDLE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the dispatcher, a function to be called
            when the dispatcher is idle, and a user-supplied value to pass to
            the callback function.  The user-supplied value can be an arbitrary
            Scheme value, so you can use CONS or LIST to pass multiple values
            to the callback.
        <callback>	- O
            returns a callback handle if the callback was successfully
            registered and #f if there was an error.

*******************************************************************************/


static  pointer  func_IOX_WHENIDLE (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    IoxDispatcher  dispatcher ;
    pointer  argument, function, userData ;
    SoxCallback  *sox ;




/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        dispatcher = (IoxDispatcher) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_IOX_WHENIDLE) Invalid dispatcher specification: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    function = argument ;

    args = cdr (args) ;
    argument = car (args) ;
    userData = argument ;

/* Allocate a structure to hold the Scheme callback function and the user
   parameter; a pointer to this structure will be passed to funcIOXCB()
   when it is invoked for a callback. */

    sox = (SoxCallback *) malloc (sizeof (SoxCallback)) ;
    if (sox == NULL) {
        LGE "(func_IOX_WHENIDLE) Error allocating SoxCallback structure.\nmalloc: ") ;
        return (sc->F) ;
    }

    sox->callback = NULL ;
    sox->sc = sc ;

/* Register the idle task with the dispatcher.  When the dispatcher is idle,
   it will call funcIOXCB(), which, in turn, will call the Scheme function in
   the SoxCallback structure. */

    sox->callback = ioxWhenIdle (dispatcher, funcIOXCB, sox) ;
    if (sox->callback == NULL) {
        LGE "(func_IOX_WHENIDLE) Error registering callback.\nioxWhenIdle: ") ;
        PUSH_ERRNO ;  free (sox) ;  POP_ERRNO ;
        return (sc->F) ;
    }

/* Save the function object and the user-supplied data so that they remain
   visible to the garbage collector and are, thus, not garbage collected. */

    sox->functionID = gc_protect (sc, function) ;
    sox->userDataID = gc_protect (sc, userData) ;

/* Return the callback to the caller. */

    return (mk_opaque (sc, (opaque) sox->callback)) ;

}

/*!*****************************************************************************

Procedure:

    funcIOXCB ()

    Handle an I/O Event Dispatcher Callback.


Purpose:

    Function funcIOXCB() is a generic IOX handler function assigned to IOX
    callbacks when a to-be-monitored event is registered using an IOX function.
    At the time the event was registered, a SoxCallback structure was created
    containing a Scheme function to be executed when the callback is invoked
    and the user data to be passed to the function.

    When the callback is invoked by the I/O event dispatcher, funcIOXCB()
    calls the Scheme function, passing it the callback handle, the user
    data, and the callback reason.


    Invocation:

        status = funcIOXCB (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by one of the IOX
            registration functions.
        <reason>	- I
            is the reason (e.g., IoxRead, IoxFire) the callback is being
            invoked.
        <userData>	- I
            is the address of the SoxCallback structure created when the
            callback was registered with the dispatcher.
        <status>	- O
            returns the status of handling the callback, zero if there were
            no errors and ERRNO otherwise.  The status value is ignored by the
            IOX dispatcher, but it may be useful if the application calls
            funcIOXCB() directly.

*******************************************************************************/


static  errno_t  funcIOXCB (

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
    pointer  args, function, userSupplied ;
    SoxCallback  *sox = (SoxCallback *) userData ;



/* If the callback is being cancelled, then deallocate the SoxCallback
   structure. */

    if (reason == IoxCancel) {
        gc_unprotect (sox->sc, sox->functionID) ;
        gc_unprotect (sox->sc, sox->userDataID) ;
        sox->callback = NULL ;
        sox->sc = NULL ;
        free (sox) ;
        return (0) ;
    }

/* Otherwise, call the Scheme function bound to the callback, passing it the
   callback handle, the callback reason, and the user-supplied argument(s). */

    function = gc_retrieve (sox->sc, sox->functionID) ;
    userSupplied = gc_retrieve (sox->sc, sox->userDataID) ;

				/* One or more user-supplied parameters. */
    args = cons (sox->sc, userSupplied, sox->sc->NIL) ;
				/* Reason for callback. */
    args = cons (sox->sc, mk_integer (sox->sc, (long) reason), args) ;
				/* Callback handle. */
    args = cons (sox->sc, mk_opaque (sox->sc, (void *) callback), args) ;

    scheme_call (sox->sc, function, args) ;

    return (0) ;

}
