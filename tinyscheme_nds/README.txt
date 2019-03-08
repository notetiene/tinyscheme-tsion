
                  Building TinyScheme for the Nintendo DS
                  ---------------------------------------

These are the files you need to build the TinyScheme library and the TinyScheme
stand-alone intepreter for the Nintendo DS.

(1) Download the TinyScheme distribution and extract it to your target
    directory.

(2) Be sure that environmant variables DEVKITPRO and DEVKITARM are defined
    for your devkitPro toolchain.

(3) Drop "Makefile.nds" and "StdIONDS.c" into your TinyScheme directory.
    (NOTE that this "StdIONDS.c" is a simplified version of the file used
    by the TSION executables.)

(4) Run "make -f Makefile.nds".  This will build the TinyScheme library,
    "libtinyscheme.a", and the TinyScheme executable, "tinyscheme.nds".

You're done!  On your DS, you'll need to install "tinyscheme.nds" and "init.scm"
in the main ("/") directory.  When you run "tinyscheme.nds", it will look for a
configuration file, "/etc/tinyscheme.conf".  See TinyScheme's "Manual.txt" file
for allowable command-line options.

(The Scheme files and "tinyscheme.nds" should be in the same directory.)
If there is no configuration file, a keyboard will pop up and you will be
prompted for the command-line arguments.  Don't enter any and you will be
taken to TinyScheme's command-line prompt, "> ".  Go crazy typing in Scheme
commands!
