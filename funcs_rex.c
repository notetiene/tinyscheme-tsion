/* $Id: funcs_rex.c,v 1.1 2009/09/01 11:31:46 alex Exp $ */
/*******************************************************************************

File:

    funcs_rex.c

    Regular Expression Functions.


Author:    Alex Measday


Purpose:

    The FUNCS_REX package defines functions for text matching and substitution
    using regular expressions.

        (rex-create "<regexp>")			=> <pattern>|#f
        (rex-debug <value>)
        (rex-destroy <pattern>)			=> <status>   (#t|#f)
        (rex-error)				=> "<error text>"

        (rex-match <pattern> "<text>")		=> <match(es)>|#f

        (rex-replace <pattern> "<text>"
                     "<replacement>" global?)	=> <result>|#f
            where <result> is a pair (newText. numSubstitutions)

        (rex-wild "<wildcard>")			=> "<regexp>"


Public Procedures:

    addFuncsREX() - registers the functions with the Scheme intepreter.

Private Procedures:

    func_REX_CREATE() - implements the REX-CREATE function.
    func_REX_DEBUG() - implements the REX-DEBUG function.
    func_REX_DESTROY() - implements the REX-DESTROY function.
    func_REX_ERROR() - implements the REX-ERROR function.
    func_REX_MATCH() - implements the REX-MATCH function.
    func_REX_REPLACE() - implements the REX-REPLACE function.
    func_REX_WILD() - implements the REX-WILD function.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "rex_util.h"			/* Regular expression utilities. */
#include  "tsion.h"			/* TinyScheme I/O Network functions. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  pointer  func_REX_CREATE P_((scheme *sc, pointer args)) ;
static  pointer  func_REX_DEBUG P_((scheme *sc, pointer args)) ;
static  pointer  func_REX_DESTROY P_((scheme *sc, pointer args)) ;
static  pointer  func_REX_ERROR P_((scheme *sc, pointer args)) ;
static  pointer  func_REX_MATCH P_((scheme *sc, pointer args)) ;
static  pointer  func_REX_REPLACE P_((scheme *sc, pointer args)) ;
static  pointer  func_REX_WILD P_((scheme *sc, pointer args)) ;

/*!*****************************************************************************

Procedure:

    addFuncsREX ()

    Register the REX Functions with the Scheme Interpreter.


Purpose:

    Function addFuncsREX() registers the REX functions as foreign functions
    with the Scheme interpreter.


    Invocation:

        addFuncsREX (sc) ;

    where

        <sc>	- I
            is the Scheme interpreter.

*******************************************************************************/


void  addFuncsREX (

#    if PROTOTYPES
        scheme  *sc)
#    else
        sc)

        scheme  *sc ;
#    endif

{

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "rex-create"),
                   mk_foreign_func (sc, func_REX_CREATE)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "rex-debug"),
                   mk_foreign_func (sc, func_REX_DEBUG)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "rex-destroy"),
                   mk_foreign_func (sc, func_REX_DESTROY)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "rex-error"),
                   mk_foreign_func (sc, func_REX_ERROR)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "rex-match"),
                   mk_foreign_func (sc, func_REX_MATCH)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "rex-replace"),
                   mk_foreign_func (sc, func_REX_REPLACE)) ;

    scheme_define (sc, sc->global_env,
                   mk_symbol (sc, "rex-wild"),
                   mk_foreign_func (sc, func_REX_WILD)) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    func_REX_CREATE ()

    Compile a Regular Expression.


Purpose:

    Function func_REX_CREATE() compiles a regular expression into a form
    suitable for matching with REX-MATCH and REX-REPLACE.

        (rex-create "<regexp>")

        Compile regular expression <regexp>.  An opaque handle is returned
        for the compiled pattern; #f is returned in the event of an error.

        See the prolog in LIBGPL's "rex_util.c" source file for the syntax
        of regular expressions recognized by REX-CREATE.


    Invocation:

        pattern = func_REX_CREATE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a string containing the regular
            expression to be compiled.
        <pattern>	- O
            returns the compiled regular expression as an opaque handle.

*******************************************************************************/


static  pointer  func_REX_CREATE (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *regexp ;
    CompiledRE  pattern ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_string (argument)) {
        regexp = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_REX_CREATE) Argument is not a string: ") ;
        return (sc->F) ;
    }

/* Compile the regular expression. */

    if (rex_compile (regexp, &pattern)) {
        LGE "(func_REX_CREATE) Error compiling RE: \"%s\"\nrex_compile: ",
            regexp) ;
        return (sc->F) ;
    }

/* Return the compiled RE to the caller. */

    return (mk_opaque (sc, (opaque) pattern)) ;

}

/*!*****************************************************************************

Procedure:

    func_REX_DEBUG ()

    Enable/Disable Regular Expression Debug Output.


Purpose:

    Function func_REX_DEBUG() enables or disables regular expression debug.

        (rex-debug <value>)

        Set the regular expression debug flag to <value>, an integer number.
        A value of 0 disables debug; a non-zero value enables debug.  Debug
        is written to standard output.


    Invocation:

        func_REX_DEBUG (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: an integer indicating the desired
            debug level.

*******************************************************************************/


static  pointer  func_REX_DEBUG (

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
        LGE "(func_REX_DEBUG) Argument is not a number: ") ;
        return (sc->F) ;
    }

/* Set the debug level. */

    rex_util_debug = is_real (argument) ? (int) rvalue (argument)
                                        : (int) ivalue (argument) ;

    return (sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_REX_DESTROY ()

    Destroy a Compiled Regular Expression.


Purpose:

    Function func_REX_DESTROY() destroys a compiled regular expression.

        (rex-destroy <pattern>)

        Destroy compiled regular expression <pattern>.


    Invocation:

        status = func_REX_DESTROY (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the compiled regular expression
            to be destroyed.
        <status>	- O
            returns true (#t) if the pattern was destroyed successfully
            and false (#f) otherwise.

*******************************************************************************/


static  pointer  func_REX_DESTROY (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    CompiledRE  pattern ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        pattern = (CompiledRE) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_REX_DESTROY) Argument is not a pattern: ") ;
        return (sc->F) ;
    }

/* Destroy the compiled regular expression. */

    return (rex_delete (pattern) ? sc->F : sc->T) ;

}

/*!*****************************************************************************

Procedure:

    func_REX_ERROR ()

    Get Error Text After Compilation Error.


Purpose:

    Function func_REX_ERROR() returns the error text after REX-CREATE fails
    to compile a regular expression.

        (rex-error)

        Get more error information after REX-CREATE fails to compile a regular
        expression.  The information is returned as a string; #f is returned
        if there is no error information.


    Invocation:

        text = func_REX_ERROR (sc, args) ;

    where

        <sc>	- I
            is the Scheme interpreter.
        <args>	- I
            is a list of the arguments, which is ignored.
        <text>	- O
            returns the error text following a failed attempt to compile a
            regular expression.

*******************************************************************************/


static  pointer  func_REX_ERROR (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{

    return ((rex_error_text == NULL) ? sc->F : mk_string (sc, rex_error_text)) ;

}

/*!*****************************************************************************

Procedure:

    func_REX_MATCH ()

    Match a Regular Expression in a Target String.


Purpose:

    Function func_REX_MATCH() attempts to match a regular expression in a
    target string.

        (rex-match <pattern> "<text>")

        Attempt to match compiled regular expression <pattern> in target
        string <text>.  If the match is successful, a Scheme list is returned
        whose first element is the entire string of text matched by the regular
        expression and whose remaining elements are the strings matched by the
        subexpresions ("$n"), if any, in the regular expression:

            ("<full match>" "<subexp0>" "<subexp1>" ...))

        If no match is found, #f is returned.


    Invocation:

        list = func_REX_MATCH (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the compiled regular expression and
            the target string.
        <list>		- O
            returns a Scheme list whose first element is the fully matched
            text and whose remaining elements are the strings matched by the
            subexpressions in the regular expression.  If no match was found,
            #f is returned.

*******************************************************************************/


static  pointer  func_REX_MATCH (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *matchStart, *subexpStart[10], *target ;
    CompiledRE  pattern ;
    int  matchLength, numSubExps, subexpLength[10] ;
    pointer  argument, pair ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        pattern = (CompiledRE) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_REX_MATCH) Argument is not a pattern: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (is_string (argument)) {
        target = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_REX_MATCH) Argument is not a string: ") ;
        return (sc->F) ;
    }

/* Attempt to match the regular expression against the target string. */

    numSubExps = rex_nosex (pattern) ;

    if (!rex_match ((const char *) target, pattern,
                     &matchStart, &matchLength,
                     numSubExps,
                     &subexpStart[0], &subexpLength[0],
                     &subexpStart[1], &subexpLength[1],
                     &subexpStart[2], &subexpLength[2],
                     &subexpStart[3], &subexpLength[3],
                     &subexpStart[4], &subexpLength[4],
                     &subexpStart[5], &subexpLength[5],
                     &subexpStart[6], &subexpLength[6],
                     &subexpStart[7], &subexpLength[7],
                     &subexpStart[8], &subexpLength[8],
                     &subexpStart[9], &subexpLength[9])) {

        return (sc->F) ;	/* No match? */

    }

/* The match was successful.  Construct a Scheme list whose car is a string
   containing the fully matched text and whose cdr is a list of the matched
   subexpressions. */

    pair = cons (sc, mk_bstring (sc, matchStart, matchLength), sc->NIL) ;

    while (numSubExps-- > 0) {		/* Build list in reverse. */
        cdr (pair) = cons (sc, mk_bstring (sc, subexpStart[numSubExps],
                                               subexpLength[numSubExps]),
                               cdr (pair)) ;
    }

    return (pair) ;

}

/*!*****************************************************************************

Procedure:

    func_REX_REPLACE ()

    Find and Replace Text in a String.


Purpose:

    Function func_REX_REPLACE() performs a search-and-replace operation on a
    string of text.

        (rex-replace <pattern> "<text>" "<replacement>" global?)

        Perform a search-and-replace operation on string <text>.  The search
        string (specified by previously-compiled regular expression <pattern>)
        is located in the source string and replaced by the substitution text,
        <replacement>.  This process may be done once or, if <global?> is #t,
        repeatedly throughout the whole string.  REX-REPLACE is intended to
        perform EX(1)-style subsitutions on a line of text:

            s/<regexp>/<replacement>/[g]

        REX-REPLACE returns a Scheme pair whose car is the new text after
        substitutions have been made and whose cdr is the number of
        substitutions that were made:

            (<newText> <numSubstitutions>)

        If no substitutions were made, REX-REPLACE returns the old text and
        zero for the number of substitutions.

        See the prolog in LIBGPL's "rex_util.c" source file for the special
        character sequences used to control the replacement process; e.g.,
        inserting matched subexpressions, etc.


    Invocation:

        pair = func_REX_REPLACE (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: the compiled regular expression, the
            text to be modified, the replacement text, and the once-or-globally
            flag.
        <pair>		- O
            returns a Scheme pair whose car is the modified text and whose cdr
            is the number of substitutions that were performed.

*******************************************************************************/


static  pointer  func_REX_REPLACE (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    bool  global ;
    char  *replacement, *result, *source ;
    CompiledRE  pattern ;
    int  numSubstitutions ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_opaque (argument)) {
        pattern = (CompiledRE) opaque_value (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_REX_REPLACE) Argument is not a pattern: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (is_string (argument)) {
        source = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_REX_REPLACE) Argument is not a string: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    if (is_string (argument)) {
        replacement = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_REX_REPLACE) Argument is not a string: ") ;
        return (sc->F) ;
    }

    args = cdr (args) ;
    argument = car (args) ;
    global = !(argument == sc->F) ;

/* Perform the search-and-replace operation on the source string. */

    if (rex_replace ((const char *) source, pattern,
                     (const char *) replacement, global ? -1 : 1,
                     &result, &numSubstitutions)) {
        LGE "(func_REX_REPLACE) Error performing search-and-replace.\nrex_replace: ") ;
        return (sc->F) ;
    }

/* The operation was successfully completed.  Return a Scheme pair whose car
   is the newly modified text and whose cdr is the number of substitutions
   that were made. */

    return (cons (sc, mk_string (sc, result),
                      mk_integer (sc, numSubstitutions))) ;

}

/*!*****************************************************************************

Procedure:

    func_REX_WILD ()

    Convert a Shell-Style Wildcard Expression into a Regular Expression.


Purpose:

    Function func_REX_CREATE() converts a UNIX CSH(1)-style wildcard
    expression to the corresponding regular expression (RE).

        (rex-wild "<wildcard>")

        Convert UNIX shell-style wildcard expression <wildcard> to the
        corresponding regular expression, which is returned as a string
        to the caller.

        The conversion handles the following cases:

        "^", RE's start-of-string symbol, is prepended to the RE to anchor the
            pattern match at the beginning of the target string.

        "*" in the wildcard expression is converted to ".*" in the RE.  "." is
            a special RE symbol that matches any character and "*" indicates
            zero or more occurrences of the preceding character, so ".*" matches
            zero or more characters in the target string.

        "?" in the wildcard expression is converted to "." in the RE.  "." is
            a special RE symbol that matches any single character.

        "." in the wildcard expression is converted to "\." (escaped dot) in
            the RE to disambiguate it from RE's special symbol.

        All other characters are passed through as is (meaning they must occur
            in the target file name).  NOTE that a shell character list/range
            enclosed in square brackets ("[...]") is passed through as is; the
            shell character set specification is compatible with the regular
            expression character set specification.

        "$", RE's end-of-string symbol, is appended to the RE so that execution
            of the RE must consume the entire target string.

        For example, the following wildcard expression:

            "*.c.0*"

        (which matches strings like "fsearch.c.007") will be converted into:

            "^.*\.c\.0.*$"


    Invocation:

        regexp = func_REX_WILD (sc, args) ;

    where

        <sc>		- I
            is the Scheme interpreter.
        <args>		- I
            is a list of the arguments: a string containing the shell-style
            wildcard specification to be converted.
        <regexo>	- O
            returns the converted regular expression as a string.

*******************************************************************************/


static  pointer  func_REX_WILD (

#    if PROTOTYPES
        scheme  *sc,
        pointer  args)
#    else
        sc, args)

        scheme  *sc ;
        pointer  args ;
#    endif

{    /* Local variables. */
    char  *regexp, *wildcard ;
    pointer  argument ;



/* Get the argument(s). */

    argument = car (args) ;
    if (is_string (argument)) {
        wildcard = strvalue (argument) ;
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(func_REX_WILD) Argument is not a string: ") ;
        return (sc->F) ;
    }

/* Conver the wildcard specification into a regular expression. */

    regexp = (char *) rex_wild (wildcard) ;

/* Return the converted regular expression to the caller. */

    return (mk_string (sc, regexp)) ;

}
