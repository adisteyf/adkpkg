#ifndef ADKPKG_H
#define ADKPKG_H

/*******************************************
 * All includs of adkcfg.h (includs part). *
 *******************************************/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>


/*******************************************
 * All defines of adkcfg.h (defines part). *
 *******************************************/

/* information about program */
#define VERSION "adkpkg v1.0 (release)"
#define INFO VERSION "\n" \
"Package Manager by Adisteyf (adk.) under the MIT License.\n\n" \
"Usage: adkpkg [ -h|--help || -v|--version || -p|--path ] *OPTION* ...\n\n" \
"OPTIONS:\n" \
"    new       Create new local repo. TYPE NAME\n" \
"    get       Get repo from a mirror (arch, github, aur, etc.). [ -a|--dont-ask || -f|--force ] MIRROR_NAME PACKAGE_NAME\n" \
"    remove    Remove local repo. [ -a|--dont-ask ] MIRROR_NAME PACKAGE_NAME"



/* better than stdbool.h */
#define   bool  char
#define   true  255
#define   false 0

/* main defines */
#define BOLD_RED "\033[1;31m"
#define BOLD_YELLOW "\033[1;33m"
#define BOLD_GREEN "\033[1;32m"
#define RESET "\033[0m"
#define LAST_STR "\r"

/* logs */
#define logerr(msg) fputs(BOLD_RED "ERROR: " RESET msg "\n\n", stderr)
#define log(msg) fputs(BOLD_YELLOW "=> " RESET msg "\n", stdout)
#define logarr printf(BOLD_YELLOW "=> " RESET)



/******************************************************
 * EnvVar is a structure of env variable for .env     *
 * files. Only two parameters: key (name of variable) *
 * and value (what variable contains).                *
 ******************************************************/

typedef struct {
    char *key;
    char *value;
} EnvVar;

/* global variables */
char  *HOME;
short  HOME_LEN;
char  *mirrorName;
bool   isOverSystem;
bool   askDownload   = true;
bool   forceDownload = false;
bool   isADKCFG      = false;


/*************************************************************
 * Functions                                                 *
 * INFO: Documentation about functions you can see in main.c *
 *************************************************************/

/* loading bar */
void   * loadingTh(void *args);
void     clrLoading(bool isCorrect, char *str);

/* parse functions */
EnvVar * parseEnv(const char *envString, int *count);
char   * readFile(const char* filename);
char  ** splitString(const char *input, int *count);

/* basic functions */
int      main(int argv, char **argc);
void     logAdd(const char *textToAdd);
void     checkTFA(int argv, int min);

/* functions for free memory */
void     freeEnvVars(EnvVar *envVars, int count);
void     freeStrArr(char **str_arr, int len);


/* manipulates with packages */
bool     mkNew(char *name, char *type_str);
bool     delPkg(char *name, char *type_str);
bool     getPkg(char *name);


#endif /* ADKPKG_H */
