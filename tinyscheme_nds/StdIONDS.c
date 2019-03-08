/* $Id: StdIONDS.c,v 1.3 2011/04/05 16:51:30 alex Exp alex $ */
/*******************************************************************************

    StdIONDS.c - initializes various NDS subsystems, queries the user for
        command-line options, and calls the actual main program.

*******************************************************************************/


#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* Standard C string functions. */
#include  <nds.h>			/* NDS definitions. */
#include  <fat.h>			/* DS file system functions. */
#ifdef DSWIFI
#    include  <dswifi9.h>		/* DS WiFi functions. */
#endif


extern  int  PROGRAM (int argc,		/* The actual program to be called. */
                      char **argv) ;


#define  MAXARGS  32
#define  MAXINPUT  1024


/*******************************************************************************
    keyPressed() - echo the character that was typed.
*******************************************************************************/

static  void  keyPressed (int  c)
{
    if (c > 0)  iprintf ("%c", c) ;
}

/*******************************************************************************
    The main program.
*******************************************************************************/

int  main (void)

{    /* Local variables. */
    char  *argv[MAXARGS], *buffer, *commandLine, *keyword, *s ;
    FILE  *file ;
    int  argc ;
    Keyboard  *kb ;
    PrintConsole  bottomScreen, topScreen ;
    size_t  length ;



/* Initialize the screen for printing. */

#ifdef BOTTOM_SCREEN
    consoleDemoInit () ;
#else
    videoSetMode (MODE_0_2D) ;
    videoSetModeSub (MODE_0_2D) ;

    vramSetBankA (VRAM_A_MAIN_BG) ;
    vramSetBankC (VRAM_C_SUB_BG) ;

#    ifdef DEVKITPRO_R24
        topScreen = *consoleInit(0, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true);
        bottomScreen = *consoleInit(0, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false);
#    else
        consoleInit (&topScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0,
                     true, true) ;
        consoleInit (&bottomScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0,
                     false, true) ;
#    endif

    consoleSelect (&topScreen) ;
#endif
    iprintf ("Console initialized ...\n") ;

/* Initialize the keyboard for typing. */

#ifdef DEVKITPRO_R24
    kb = keyboardGetDefault () ;
    kb->OnKeyPressed = keyPressed ;
    keyboardInit (kb) ;
#else
    kb = keyboardDemoInit () ;
    kb->OnKeyPressed = keyPressed ;
#endif
    iprintf ("Keyboard initialized ...\n") ;

/* Initialize the file system utilities. */

    if (fatInitDefault ())
        iprintf ("FAT library initialized ...\n") ;
    else
        iprintf ("Error initializing the file system.\n") ;

/* If internet access is required, then initialize the DS WiFi utilities
   and connect to the wireless access point. */

#ifdef DSWIFI
    iprintf ("Connecting via WFC data ...\n") ;
    if (Wifi_InitDefault (WFC_CONNECT))
        iprintf ("... connected.\n") ;
    else
        iprintf ("... failed to connect.\n") ;
#endif

/* Open the program's optional configuration file. */

    commandLine = NULL ;
    buffer = malloc (MAXINPUT) ;
    sprintf (buffer, "/etc/%s.conf", SPROGRAM) ;
    file = fopen (buffer, "r") ;
    while ((file != NULL) && (fgets (buffer, MAXINPUT, file) != NULL)) {
        length = strlen (buffer) ;
        keyword = strtok (buffer, " \t\n\r") ;
        if ((keyword == NULL) || (*keyword == '#'))  continue ;
        if (strcmp (keyword, "putenv") == 0) {
            s = strtok (NULL, " \t\n\r") ;
            if (s != NULL)  putenv (s) ;
        } else if (strcmp (keyword, SPROGRAM) == 0) {
            s = keyword + strlen (keyword) ;
            if (s != (buffer + length))  s++ ;	/* Arguments after keyword? */
            commandLine = strdup (s) ;
        }
    }
    if (file != NULL)  fclose (file) ;
    free (buffer) ;

/* Ask the user to type in the command-line arguments on the keyboard. */

    if (commandLine == NULL) {
        commandLine = malloc (MAXINPUT) ;
        iprintf ("\nEnter the command-line arguments: \n") ;
        gets (commandLine) ;
    }

/* Parse the command-line arguments and add them to the argv[] array. */

    argc = 0 ;
    argv[argc++] = SPROGRAM ;
    s = strtok (commandLine, " \t\n\r") ;
    while ((s != NULL) && (argc < MAXARGS)) {
        argv[argc++] = s ;
        s = strtok (NULL, " \t\n\r") ;
    }

    swiWaitForVBlank () ;

/* Call the actual program. */

    iprintf ("Calling %s ...\n", SPROGRAM) ;
    return (PROGRAM (argc, argv)) ;

}
