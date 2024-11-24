#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "adkpkg.h"

char *HOME;



void printHelp() {
    printf(
VERSION "\n"
"Package Manager by adk.\n\n"
"Usage: adkpkg [ -h|--help ] [ -v|--version ]\n\n"
"OPTIONS:\n"
"    new       Create new local repo.\n"
"    get       Get repo.\n"
"    remove    Remove local repo.\n"
    );
}






void logAdd(const char *textToAdd) {
    char *log_path = "/apps/c/adkpkg/logs";

    FILE *file = fopen(strcat(HOME, log_path), "a");
    if (file == NULL) {
        logerr("Unavariable to open the log file.");
        log("Aborting...");
        exit(1);
    }

    if (fputs("\n", file) == EOF) {
        logerr("Unavariable to write into log file.");
        log("Aborting...");
        fclose(file);
        exit(1);
    } else {
        fputs(textToAdd, file);
        fputs("\n", file);
    }

    fclose(file);
}

char* getPkgName(short type, short *len) {
    switch (type) {
        case 0:
            *len = 1;
            return "c";
    }

    logerr("This type of package in unavariable.");
    exit(1);
}


bool mkNew(char *name, short type) {
    short type_len = 0;
    char *type_str = getPkgName(type, &type_len);
    short name_len = strlen(name);

    pthread_t cp_load;
    pthread_create(&cp_load, 0, loadingTh, "Copying template to package");

    short cp_template_len = 95
        +name_len
        +(type_len*5)
        +(strlen(HOME)*5);

    char *cp_template = malloc(cp_template_len);
    snprintf(cp_template, cp_template_len, "cp -r %s/apps/c/adkpkg/%s-pkg/ %s/apps/%s/ > %s/apps/c/adkpkg/logs 2>&1 && mv %s/apps/%s/%s-pkg/ %s/apps/%s/%s/", HOME, type_str, HOME, type_str, HOME, HOME, type_str, type_str, HOME, type_str, name);

    logAdd(cp_template);
    int status = system(cp_template);
    if (status) {
        pthread_cancel(cp_load);
        clrLoading(false, "Copying template to package");

        logerr("Unavariable to copy template to package.");
        log("Aborting...");
        exit(1); // TODO: сделать очисту памяти по аборту/завершению
    }

    pthread_cancel(cp_load);
    clrLoading(true, "Copying template to package");
    return true;
}

bool delPkg(char *name, short type) {
    short type_len;
    char *type_str = getPkgName(type, &type_len);

    short rm_path_len = type_len
        +9
        +strlen(name);
    char *rm_path = malloc(rm_path_len); // 'rm -rf ~/apps/c/test'

    snprintf(rm_path, rm_path_len, "~/apps/%s/%s/", type_str, name);
    
    pthread_t rm_package;
    pthread_create(&rm_package, 0, loadingTh, "Removing package");

    short check_pkg_len = rm_path_len+20;
    char *check_pkg = malloc(check_pkg_len);
    snprintf(check_pkg, check_pkg_len, "ls %s > /dev/null 2>&1", rm_path);

    int status = system(check_pkg);

    if (status) {
        pthread_cancel(rm_package);
        clrLoading(false, "Removing package");
        logerr("Can't remove package that doesn't exists.");
        exit(1);
    }

    short rm_pkg_cmd_len = rm_path_len+7;
    char *rm_pkg_cmd = malloc(rm_pkg_cmd_len);
    snprintf(rm_pkg_cmd, rm_pkg_cmd_len, "rm -rf %s", rm_path);

    status = system(rm_pkg_cmd);
    if (status) {
        pthread_cancel(rm_package);
        clrLoading(false, "Removing package");
        logerr("Remove aborted.");
        printf("RM: Status %d\n", status);
        exit(1);
    }

    pthread_cancel(rm_package);
    clrLoading(true, "Removing package");
    return true;
}

void checkTFA(int argv, int min) {
    if (argv < min+1) {
        logerr("Too few arguments. See --help.");
        exit(1);
    }
}

int main(int argv, char **argc) {
    if (getuid() == 0) {
        logerr("Do not run as root.");
        exit(1);
    }

    checkTFA(argv, 1);
    system("touch ~/apps/c/adkpkg/logs");
    HOME = malloc(1+strlen(getenv("HOME")));
    HOME = getenv("HOME");


    for(int i=0; i<argv; ++i) {
        if (i == 0) continue;

        if (0==strcmp(argc[i], "--help") || 0==strcmp(argc[i], "-h")) {
            printHelp();
            exit(0);
        } else
        if (0==strcmp(argc[i], "--version") || 0==strcmp(argc[i], "-v")) {
            printf(VERSION "\n");
            exit(0);
        }
    }

    if (strcmp(argc[1], "new") == 0) {
        checkTFA(argv, 3);
        if (strcmp(argc[2], "c") == 0) {
            mkNew(argc[3], 0);
        }
    }

    else if (strcmp(argc[1], "remove") == 0) {
        checkTFA(argv, 3);
        if (strcmp(argc[2], "c") == 0) {
            delPkg(argc[3], 0);
        }
    }

    else {
        logerr("Incorrect args. See --help.");
        exit(1);
    }
    return 0;
}
