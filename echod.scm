; $Id: echod.scm,v 1.1 2009/09/18 14:59:32 alex Exp $
;*******************************************************************************
;
;    ECHO - is a simple echo server that can handle multiple clients
;        simultaneously.  Clients connect to the server on port 10234.
;        Any data a client sends is echoed back to the client.  If a
;        client drops its connection, ECHOD drops it too.
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
;        as another input source.  (Thus, at any given time, ECHO will have
;        the listening port and the data connections of all connected clients
;        registered with the dispatcher.)
;
;        When a client sends an arbitrary chunk of data to the server, the
;        dispatcher detects the pending input on the client's data connection
;        and invokes the ECHO-CLIENT callback function, passing it the client's
;        TCP/IP endpoint (supplied when the client was registered with the
;        dispatcher in NEW-CLIENT).
;
;        ECHO-CLIENT reads a single chunk of input from the client and writes
;        it back as is to the client.  If the network connection goes down,
;        ECHO-CLIENT closes the connection and removes its registration with
;        the dispatcher.
;
;*******************************************************************************


(iox-debug 1)
(tcp-debug 1)


;*******************************************************************************
;    echo-client - is a callback function invoked when input from a client
;        is detected by the I/O event dispatcher.  A single chunk of input
;        (up to 1024 bytes in length) is read and echoed back to the client.
;        If an error occurs (i.e., the client closes the connection),
;        echo-client unregisters the client as an input source and closes
;        its network connection.
;*******************************************************************************

(define (echo-client callback reason client)
    (let  ((buffer (tcp-read client -1024)))
        (if buffer
            (tcp-write client buffer)
            (begin
                (iox-cancel callback)
                (tcp-destroy client)
            )
        )
    )
)


;*******************************************************************************
;    new-client - is a callback function invoked when a new client requests
;        a network connection.  The connection is accepted and the client's
;        socket is registered as an input source with the I/O event dispatcher.
;        When input is received from the client, the dispatcher will invoke
;        echo-client to read and process the input.
;*******************************************************************************

(define (new-client callback reason . args)
    (let*  ((server (list-ref args 0))
            (client (tcp-answer server)))
        (iox-onio (iox-dispatcher callback)
            echo-client client IOX_READ (tcp-fd client)
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
