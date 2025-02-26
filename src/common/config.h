//#define TEST_MODE // Add DM equipment to all characters and DM can build houses

/*
 * File: config.h
 * Purpose: Angband specific configuration stuff
 */

#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

/*
 * Some really important things you ought to change
 */

/*
 * Defines the default paths to the Angband directories, for ports that use
 * the main.c file.
 *
 * "config path" is for per-installation configurable data, like the game's
 * edit files and system-wide preferences.
 *
 * "lib path" is for static data, like sounds, graphics and fonts.
 *
 * "data path" is for variable data, like save files and scores. On single-
 * user systems, this also includes user preferences and dumps (on multi-
 * user systems these go under the user's home directory).
 *
 * Using the value "./lib/" below tells Angband that, by default,
 * the user will run "angband" from the same directory that contains
 * the "lib" directory.  This is a reasonable (but imperfect) default.
 *
 * If at all possible, you should change this value to refer to the
 * actual location of the folders, for example, "/etc/angband/"
 * or "/usr/share/angband/", or "/var/games/angband/". In fact, if at all
 * possible you should use a packaging system which does this for you.
 */
#ifndef DEFAULT_CONFIG_PATH
    #ifndef BUILDINGWithVS
        # define DEFAULT_CONFIG_PATH "." PATH_SEP "lib" PATH_SEP
    #else//Get Absolute path instead
        # define DEFAULT_CONFIG_PATH "lib" PATH_SEP
    #endif
#endif

#ifndef DEFAULT_LIB_PATH
    #ifndef BUILDINGWithVS
        # define DEFAULT_LIB_PATH "." PATH_SEP "lib" PATH_SEP
    #else
        # define DEFAULT_LIB_PATH "lib" PATH_SEP
    #endif
#endif

#ifndef DEFAULT_DATA_PATH
    #ifndef BUILDINGWithVS
        # define DEFAULT_DATA_PATH "." PATH_SEP "lib" PATH_SEP
    #else
        # define DEFAULT_DATA_PATH "lib" PATH_SEP
    #endif
#endif

/*
 * Other defines
 */

/* Compile in support for debug commands
#define DEBUG_MODE */

/*
 * OPTION: Do not switch to manual targeting if there are no
 * targets in the vicinity of player (un-Angband), display a prompt instead
 */
#define NOTARGET_PROMPT

/* Use private paths: /save and /scores subdirectories are put in /user if defined */
/* PWMAngband: since we only work under Windows, we define this here -- do not modify! */
#ifdef WINDOWS
#if !defined(USE_PRIVATE_PATHS)//Prevent macro redefinition
# define USE_PRIVATE_PATHS
#endif
#endif

#if defined(ON_ANDROID)

#if !defined(USE_PRIVATE_PATHS)
# define USE_PRIVATE_PATHS 1
#endif

#else

/**
 * OPTION: Create and use a hidden directory in the users home directory
 * for storing pref files and character dumps.
 */
#if defined(UNIX) && !defined(MACH_O_CARBON) && !defined(PRIVATE_USER_PATH)
# define PRIVATE_USER_PATH "~/.pwmangband"
#endif

#endif /* ON_ANDROID */

#endif /* INCLUDED_CONFIG_H */
