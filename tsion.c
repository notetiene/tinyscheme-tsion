/* $Id: tsion.c,v 1.1 2009/08/27 00:16:15 alex Exp alex $ */
/*******************************************************************************

Program:

    tsion

    TinyScheme with I/O Event Dispatcher and Network Extensions.


Author:    Alex Measday


Purpose:

    TSION is TinyScheme with support for an I/O event dispatcher, TCP/IP
    networking, and other miscellaneous extensions.


    Invocation:

        % tsion [-debug] [-Debug] [-evaluate <code>] [<file(s)>] [-quit]

    where:

        "-debug"
        "-Debug"
            enables debug output (written to STDOUT).  Capital "-Debug"
            generates more voluminous debug.
        "-evaluate <code>"
            passes the argument string to the Scheme interpreter.
        "<file(s)>"
            are one or more Scheme files to load and execute.  The "-evaluate"
            option can be used to set arguments for use by the code in the
            file.  If multiple files are being loaded, a different "-evaluate"
            option can be specified before each one.
        "-quit"
            quits the program, thereby skipping the read-stdin loop.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <signal.h>			/* Signal definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "fnm_util.h"			/* Filename utilities. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */
#include  "gc_util.h"			/* Garbage collection utilities. */
#include  "plist_util.h"		/* TinyScheme property lists. */

/*******************************************************************************
    TSION's Main Program.
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
    bool  quit ;
    char  *argument, *fileName ;
    FILE  *file ;
    int  errflg, option ;
    OptContext  scan ;
    scheme  *sc ;
    TsionSpecific  ts ;

    const  char  *optionList[] = {	/* Command line options. */
        "{Debug}", "{debug}", "{evaluate:}", "{quit}", NULL
    } ;




#if HAVE_SIGNAL && defined(SIGPIPE)
    signal (SIGPIPE, SIG_IGN) ;
#endif
    aperror_print = 1 ;


/*******************************************************************************
    Initialize TSION and create a Scheme engine.
*******************************************************************************/

/* Create the Scheme engine. */

    sc = scheme_init_new ();
    if (sc == NULL) {
        LGE "[%s] Error initializing Scheme engine.\nscheme_init_new: ",
            argv[0]) ;
        exit (errno) ;
    }

    ts = (TsionSpecific) calloc (sizeof (_TsionSpecific), 1) ;
    if (ts == NULL) {
        LGE "[%s] Error allocating TSION-specific interpreter structure.\ncalloc: ",
            argv[0]) ;
        exit (errno) ;
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

/* Default I/O is from standard input and standard output. */

    scheme_set_input_port_file (sc, stdin) ;
    scheme_set_output_port_file (sc, stdout) ;

#if USE_DL
    scheme_define (sc,
                   sc->global_env,
                   mk_symbol (sc, "load-extension"),
                   mk_foreign_func (sc, scm_load_ext)) ;
#endif

/* Load the initialization file. */

    fileName = getenv ("TINYSCHEMEINIT") ;
    if (fileName == NULL)  fileName = "init.scm" ;
    file = fopen (fileName, "r") ;
    if (file == NULL) {
        LGE "[%s] Error opening initialization file, \"%s\".\nfopen: ",
            argv[0], fileName) ;
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

/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    quit = false ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-Debug" */
            tcp_util_debug = 1 ;
        case 2:			/* "-debug" */
            gc_util_debug = 1 ;
            plist_util_debug = 1 ;
            break ;
        case 3:			/* "-evaluate <code>" */
            scheme_load_string (sc, argument) ;
            break ;
        case 4:			/* "-quit" */
            quit = true ;
            break ;
        case NONOPT:		/* "<fileName>" */
            fileName = (char *) fnmBuild (FnmPath, argument, NULL) ;
            file = fopen (fileName, "r") ;
            if (file == NULL) {
                LGE "[%s] Error opening file, \"%s\".\nfopen: ",
                    argv[0], fileName) ;
                errflg++ ;
            } else {
                scheme_load_file (sc, file) ;
                fclose (file) ;
            }
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  tsion [-debug] [-Debug] [-evaluate <code>] [<fileName>] [-quit]\n") ;
        exit (EINVAL) ;
    }


/*******************************************************************************
    Read and execute commands from the user.
*******************************************************************************/

    if (!quit)  scheme_load_file (sc, stdin) ;

    scheme_deinit (sc) ;


    exit (0) ;

}
