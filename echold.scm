; $Id: echold.scm,v 1.1 2009/09/18 14:59:32 alex Exp $
;*******************************************************************************
;
;    ECHOLD - is a simple echo server that can handle multiple clients
;        simultaneously.  Clients connect to the server on port 10234.
;        Any data a client sends is echoed back to the client.  If a
;        client drops its connection, ECHOLD drops it too.
;
;        Unlike ECHOD, ECHOLD uses the LF-terminated networking (LFN)
;        functions to communicate on a line-by-line basis with clients.
;
;        ECHOLD revolves around an I/O event dispatcher.  In the MAIN LOOP
;        section at the bottom of this program, the dispatcher is created,
;        the TCP/IP listening port is created, and the listening port is
;        registered with the dispatcher as an input source.  Control is
;        then turned over to the dispatcher to monitor (i) the listening
;        port for incoming connection requests and (ii) any future client
;        connections.
;
;        Whenever a client tries to connect to the server on port 10234,
;        the dispatcher detects the pending connection request and invokes
;        the NEW-CLIENT callback function.  NEW-CLIENT accepts the connection
;        request, the operating system creates a new data connection for the
;        client, and the new data connection is registered with the dispatcher
;        as another input source.  (Thus, at any given time, ECHOLD will have
;        the listening port and the data connections of all connected clients
;        registered with the dispatcher.)
;
;        When a client sends an arbitrary chunk of data to the server,
;        the dispatcher detects the pending input on the client's data
;        connection and invokes the ECHO-CLIENT callback function,
;        passing it the client's LF-terminated networking stream (supplied
;        when the client was registered with the dispatcher in NEW-CLIENT).
;
;        While the client's stream has pending input, ECHO-CLIENT reads
;        CR- and/or LF-terminated lines of input from the client and writes
;        each line back to the client, terminated with both a CR and a LF.
;        If the network connection goes down, ECHO-CLIENT closes the
;        connection and removes its registration with the dispatcher.
;
;        (Unlike ECHOD with its raw TCP/IP I/O, ECHOLD's LFN functions buffer
;        input, so ECHOLD can't simply read a single chunk of data and return
;        control to the dispatcher.  Instead, ECHOLD must continue reading
;        input into its buffer until the line-termination character is
;        received.  At that point, ECHO-CLIENT can echo the input line back
;        to the client, but ECHOLD may now have part of the following line
;        of input in its buffer, so it must loop and wait for the remainder
;        of the next line.  Consequently, a single invocation of ECHO-CLIENT
;        may read and echo multiple lines of input.)
;
;*******************************************************************************


(iox-debug 1)
(tcp-debug 1)
(lfn-debug 1)


;*******************************************************************************
;    echo-client - is a callback function invoked when input from a client is
;        detected by the I/O event dispatcher.  Pending input from the client
;        is read line-by-line and echoed back to the client until there is
;        no more input.  If an error occurs (i.e., the client closes the
;        connection), echo-client unregisters the client as an input source
;        and closes the client's LF-terminated network stream.
;*******************************************************************************

(define (echo-client callback reason client)

; While the client stream is readable, read and echo the next input line.

    (do ((buffer #f))
        ((not (lfn-readable? client)))
        (begin
            (set! buffer (lfn-getline client))
            (if buffer (lfn-putline client buffer 3))
        )
    )

; No more input - check if the client is still up.  If not, unregister the
; client as an input source and close its network connection.

    (if (not (lfn-up? client))
        (begin
            (iox-cancel callback)
            (lfn-destroy client)
        )
    )

)


;*******************************************************************************
;    new-client - is a callback function invoked when a new client requests
;        a network connection.  The connection is accepted, a LF-terminated
;        network stream is created for the client, and the client's socket
;        is registered as an input source with the I/O event dispatcher.
;        When input is received from the client, the dispatcher will invoke
;        echo-client to read and process the input.
;*******************************************************************************

(define (new-client callback reason . args)
    (let*  ((server (list-ref args 0))
            (client (lfn-create (tcp-answer server))))
        (iox-onio (iox-dispatcher callback)
            echo-client client IOX_READ (lfn-fd client)
        )
    )
)


;*******************************************************************************
;    Main Loop - creates the network listening port at which the program
;        will listen for client connection requests, creates the I/O event
;        dispatcher, registers the listening port as an input source with
;        the dispatcher, and then turns control over to the dispatcher to
;        monitor the listening port and any future network connections for
;        activity.
;*******************************************************************************

(define listener (tcp-listen 10234))

(define dispatcher (iox-create))

(iox-onio dispatcher new-client listener IOX_READ (tcp-fd listener))

(display "Monitoring ...\n")

(iox-monitor dispatcher)
