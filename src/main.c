/*****************
 * Headers       *
 *****************/

#include "adkpkg.h"

/*
 * ADKPKG (adk's pkg) - MIT LICENSE (2024-2025) (c) Adisteyf (adk.) / Shawarmateam (by adisteyf).
 * 
 * ADKPKG is a package manager for UNIX systems that connects various UNIX package managers and 
 * installation utilities (e.g., git, wget, etc.).
 *
 * You can check the version using `adkpkg --version` and get help using `adkpkg --help`.
 *
 *
 * This project is licensed under the MIT License and is owned by Adisteyf (adk).
 *
 * ## Dependencies:
 * 
 * - glibc (GNU C Library)
 * - UNIX/Linux headers of your OS & API (for dirent.h)
 *
 * ## Recommended Compiler:
 * 
 * - GCC
 *
 * ## Links:
 * 
 * - Repository:      https://github.com/adisteyf/adkpkg.git
 * - Main Repository: https://github.com/shawarmateam/adkpkg.git (for pull requests)
 * - Bug Reports:     https://github.com/shawarmateam/adkpkg/issues
 */








/******************************************************
 * Function for loading bar in console. Need to start *
 * at new thread.                                     *
 ******************************************************/

void *loadingTh(void *args)
{
    const char *loading[] = {"|", "/", "-", "\\"};
    int i;

    while (1) {
        printf(LAST_STR BOLD_YELLOW "=> " RESET "%s... %s", (char*)args, loading[i % 4]);
        fflush(stdout);
        usleep(200000);

        i++;
        if (i == 4) i=0;
    }
}


/******************************************************
 * Function to clear loading bar in console.          *
 ******************************************************/

void clrLoading(bool isCorrect, char *str)
{
    if (isCorrect) {
        printf(LAST_STR BOLD_YELLOW "=> " RESET "%s... " BOLD_GREEN "Done!" RESET);
    }

    else {
        printf(LAST_STR BOLD_YELLOW "=> " RESET "%s... " BOLD_RED "Error!" RESET "\n");
    }

    putchar('\n');
    fflush(stdout);
}



EnvVar* parseEnv(const char *envString, int *count)
{
    int lines = 0;
    const char *ptr = envString;
    while (*ptr) {
        if (*ptr == '\n') lines++;
        ptr++;
    }

    if (*(ptr - 1) != '\n') lines++;

    EnvVar *envVarsArray = malloc(lines * sizeof(EnvVar));
    if (!envVarsArray) {
        return NULL;
    }

    char *envStringCopy = strdup(envString);
    char *line = strtok(envStringCopy, "\n");
    int index = 0;

    while (line != NULL) {
        char *equalSign = strchr(line, '=');
        if (equalSign) {
            *equalSign = 0;
            envVarsArray[index].key = strdup(line);
            envVarsArray[index].value = strdup(equalSign + 1);
            index++;
        }
        line = strtok(NULL, "\n");
    }

    free(envStringCopy);
    *count = lines;
    return envVarsArray;
}

void freeEnvVars(EnvVar *envVars, int count)
{
    for (int i = 0; i < count; i++) {
        free(envVars[i].key);
        free(envVars[i].value);
    }

    free(envVars);
}



/**********************************************
 * Check if folder contains any files/folders *
 * (excluding dotfiles).                      *
 **********************************************/

int checkFolderContent(char * directory)
{
    struct dirent *entry;
    DIR *dp = opendir(directory);

    if (dp == NULL) {
        printf("directory equals '%s'\n", directory);
        logerr("Failed to open directory");
        exit(3);
    }

    int has_files = 0;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] != '.') {
            has_files = 1;
            break;
        }
    }

    closedir(dp);

    return has_files;
}


char* readFile(const char* filename)
{
    FILE *file;
    char line[256];
    char *content = NULL;
    size_t total_length = 0;

    file = fopen(filename, "r");
    if (file == NULL) {
        logerr("Can't open file.");
        printf("File: '%s'\n", filename);
        //log("Aborting...");
        //exit(EXIT_FAILURE);
        isADKCFG = false;
        return "unknown";
    }

    while (fgets(line, sizeof(line), file)) {
        size_t line_length = strlen(line);

        char *new_content = realloc(content, total_length + line_length + 1);
        if (new_content == NULL) {
            logerr("Can't allocate memory.");
            log("Aborting...");

            free(content);
            fclose(file);
            exit(EXIT_FAILURE);
        }
        content = new_content;

        strcpy(content + total_length, line);
        total_length += line_length;
    }

    fclose(file);
    return content;
}


/******************************************************
 * Tokenizing a string ny '\n' and writes it into     *
 * string array.                                      *
 ******************************************************/

char** splitString(const char *input, int *count)
{
    char *input_copy = strdup(input);
    if (!input_copy) {
        logerr("Can't copy string for list. (splitString).");
        log("Aborting...");
        exit(1);
    }

    char **args = NULL;
    *count = 0;

    char *token = strtok(input_copy, " ");
    while (token != NULL) {
        args = (char **)realloc(args, (*count + 1) * sizeof(char *));
        if (!args) {
            free(input_copy);
            return NULL;
        }

        args[*count] = strdup(token);
        if (!args[*count]) {
            free(input_copy);
            for (int i = 0; i < *count; i++) {
                free(args[i]);
            }
            free(args);
            return NULL;
        }

        (*count)++;
        token = strtok(NULL, " ");
    }

    free(input_copy);
    return args;
}



/******************************************************
 * Add some text to log file in                       *
 * $(adkpkg -p adkpkg)/logs                           *
 ******************************************************/

void logAdd(const char *textToAdd)
{
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


/******************************************************
 * Creates a custom package based on template.        *
 ******************************************************/

bool mkNew(char *name, char *type_str)
{
    short type_len = strlen(type_str);
    short name_len = strlen(name);

    pthread_t cp_load;
    pthread_create(&cp_load, 0, loadingTh, "Copying template to package");

    short cp_template_len = 95
        +name_len
        +(type_len*5)
        +(HOME_LEN*5);

    char *cp_template = malloc(cp_template_len);
    snprintf       /* copy template into apps/lang dir */
    (
            cp_template,
            cp_template_len,
            "cp -r %s/apps/c/adkpkg/%s-pkg/ %s/apps/%s/ > %s/apps/c/adkpkg/logs 2>&1 && mv %s/apps/%s/%s-pkg/ %s/apps/%s/%s/",
            HOME, type_str, HOME, type_str, HOME, HOME, type_str, type_str, HOME, type_str, name
    );

    logAdd(cp_template);
    int status = system(cp_template);
    if (status) {
        pthread_cancel(cp_load);
        clrLoading(false, "Copying template to package");

        logerr("Unavariable to copy template to package.");
        log("Aborting...");
        exit(1);
    }

    pthread_cancel(cp_load);
    clrLoading(true, "Copying template to package");
    return true;
}



/******************************************************
 * Deletes a package.                                 *
 ******************************************************/

bool delPkg(char *name, char *type_str)
{
    if (askDownload) {
        putchar('\n');
        puts("Remove this package? [y/N]:");
        logarr;

        char * answer=0x00;
        size_t len_answer = 0;

        if (getline(&answer, &len_answer, stdin)<0) {
            logerr("Can't read the answer");
            log("Aborting...");
            exit(1);
        }

        for (int i=0; i<len_answer; ++i) {
            answer[i]=tolower(answer[i]);
            if (answer[i]=='\n') { answer[i]=0; }
        }

        if (0==strcmp(answer, "n") || 0==strcmp(answer, "no") || answer[0]==0) { exit(0); }
    }

    short type_len = strlen(type_str);
    short rm_path_len = type_len
        +9
        +strlen(name);
    char *rm_path = malloc(rm_path_len);

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

void freeStrArr(char ** str_arr, int len)
{
    for (int i=0; i<len; ++i) { free(str_arr[i]); }
    free(str_arr);
}


/******************************************************
 * Download a package.                                *
 ******************************************************/

bool getPkg(char *name)
{
    log("Getting mirror list...");
    short len_list = 32
        +HOME_LEN;

    char *list_cmd = malloc(len_list);
    snprintf(list_cmd, len_list, "%s/apps/c/adkpkg/lists/%s.txt", HOME, mirrorName);

    char *list = readFile(list_cmd);
    free(list_cmd);

    bool isFounded = false;
    bool isLink = false;
    char *package_repo;
    
    char *token = strtok(list, "\n");
    while (token != 0) {
        if (isFounded) {
            package_repo = strdup(token);
            break;
        }

        if (!isLink && 0==strcmp(token, name)) {
            isFounded = true;
        }

        token = strtok(0, "\n");
        isLink = !isLink;
    }

    if (!isFounded) {
        logerr("Package is unavariable.");
        exit(1);
    }

    log("Command for download:");
    printf("'%s'\n", package_repo);

    if (askDownload) {
        putchar('\n');
        puts("Download this package? [Y/n]:");
        logarr;

        char * answer=0x00;
        size_t len_answer = 0;

        if (getline(&answer, &len_answer, stdin)<0) {
            logerr("Can't read the answer");
            log("Aborting...");
            exit(1);
        }

        for (int i=0; i<len_answer; ++i) {
            answer[i]=tolower(answer[i]);
            if (answer[i]=='\n') { answer[i]=0; }
        }

        if (0==strcmp(answer, "n") || 0==strcmp(answer, "no")) { exit(0); }
    }

    log("Preparing to download...");

    int status = system("mkdir -p /tmp/adkpkg");
    if (status) {
        logerr("Can't create temp dir for adkpkg.");
        log("Aborting...");
        exit(1);
    }

    status = system(package_repo);
    free(package_repo);
    if (status && !forceDownload) {
        printf("status equals %d\n", status);
        logerr("Unable to download package into temp dir (/tmp/adkpkg).");
        logerr("Use -f for force.");
        log("Aborting...");
        exit(1);
    }

    int adkcfg_path_len = 20
        +strlen(name);

    char *adkcfg_path = malloc(adkcfg_path_len);

    snprintf(adkcfg_path, adkcfg_path_len, "/tmp/adkpkg/%s/ADKCFG", name);
    char *ADKCFG = readFile(adkcfg_path);
    free(adkcfg_path);

    char *type = 0x00;
    if (isADKCFG) {
        int cfg_len = 0;
        EnvVar *ADKCFG_VARS = parseEnv(ADKCFG, &cfg_len);
        printf("ADKCFG: '%s'\n", ADKCFG);
        free(ADKCFG);

        for (int i=0; i<cfg_len; ++i) {
            printf("current key: %s\n", ADKCFG_VARS[i].key);
            if (0==strcmp(ADKCFG_VARS[i].key, "TYPE")) // TODO: добавить больше параметров для adkcfg
                type = ADKCFG_VARS[i].value;
        }

        if (!type) {
            logerr("Type is missing in package.");
            printf("Type: '%s'\n", type);
            log("Aborting...");
            exit(1);
        }
    }

    else {
        putchar('\n');
        printf("No ADKCFG file (write the type): ");
        logarr;

        size_t len_type = 0;

        if (getline(&type, &len_type, stdin)<0) {
            logerr("Can't read the answer");
            log("Aborting...");
            exit(1);
        }

        for (int i=0; i<len_type; ++i) {
            type[i]=tolower(type[i]);
            if (type[i]=='\n') { type[i]=0; }
        }
    }

    pthread_t cp_load;
    pthread_create(&cp_load, 0, loadingTh, "Copying package into ~/apps");

    int mk_typedir_len = 17
        +HOME_LEN
        +strlen(type);

    char *mk_typedir = malloc(mk_typedir_len);
    snprintf(mk_typedir, mk_typedir_len, "mkdir -p %s/apps/%s", HOME, type);
    system(mk_typedir);

    free(mk_typedir);

    int cp_to_apps_len = 24
        +strlen(type)
        +HOME_LEN
        +(strlen(name))*2;

    char *cp_to_apps = malloc(cp_to_apps_len);
    snprintf(cp_to_apps, cp_to_apps_len, "mv /tmp/adkpkg/%s %s/apps/%s/%s", name, HOME, type, name);
    status = system(cp_to_apps);

    pthread_cancel(cp_load);
    if (status) {
        clrLoading(false, "Copying package into ~/apps");
        logerr("Unable to download package into ~/apps");
        printf("%s\n", status);
        log("Aborting...");
        free(cp_to_apps);
        exit(1);
    }

    clrLoading(true, "Copying package into ~/apps");
    log("Setting package into PKGS_INFO file...");

    short p_info_path_len = 25+HOME_LEN;
    char *p_info_path = malloc(p_info_path_len);
    snprintf(p_info_path, p_info_path_len, "%s/apps/c/adkpkg/PKGS_INFO", HOME);

    short str_pkg_len = 2
        +(strlen(name)*2)
        +HOME_LEN
        +strlen(type);

    char *str_pkg = malloc(str_pkg_len);
    snprintf(str_pkg, str_pkg_len, "%s=%s/apps/%s/%s", name, HOME, type, name);

    FILE *pkgs_info = fopen(p_info_path, "a");
    free(p_info_path);
    if (!pkgs_info) {
        logerr("Can't open PKGS_INFO file. Package was not added to package list.");
    } else

    if (fputs(str_pkg, pkgs_info) == EOF) {
        logerr("Can't write to PKGS_INFO file. Package was not added to package list.");
    }

    free(str_pkg);
    fclose(pkgs_info);

    log("Package was downloaded successfully.");

    return true;
}

/* check for Too Few Argunents (TFA) */
void checkTFA(int argv, int min)
{
    if (argv < min+1) {
        logerr("Too few arguments. See --help.");
        exit(1);
    }
}

int main(int argv, char ** argc)
{
    if (getuid() == 0) {
        logerr("Do not run as root.");
        exit(1);
    }

    checkTFA(argv, 1);
    system("touch ~/apps/c/adkpkg/logs ~/apps/c/adkpkg/PKGS_INFO");
    HOME = malloc(1+strlen(getenv("HOME")));
    HOME = getenv("HOME");
    HOME_LEN = strlen(HOME);


    for(int i=0; i<argv; ++i) {
        if (i == 0) continue;

        if (0==strcmp(argc[i], "--path") || 0==strcmp(argc[i], "-p")) {
            checkTFA(argv, 2);

            short p_info_path_len = 25+HOME_LEN;
            char *p_info_path = malloc(p_info_path_len);
            snprintf(p_info_path, p_info_path_len, "%s/apps/c/adkpkg/PKGS_INFO", HOME);

            char *env_str = readFile(p_info_path);
            free(p_info_path);
            int pkgs_len = 0;
            EnvVar *pkgs = parseEnv(env_str, &pkgs_len);

            for (int j=0; j<pkgs_len; ++j) {
                if (0==strcmp(pkgs[j].key, argc[i+1])) {
                    puts(pkgs[j].value);
                    break;
                }
            }
            exit(0);
        }
        if (0==strcmp(argc[i], "--help") || 0==strcmp(argc[i], "-h")) {
            puts(INFO "\n");
            exit(0);
        }
        if (0==strcmp(argc[i], "--version") || 0==strcmp(argc[i], "-v")) {
            puts(VERSION "\n");
            exit(0);
        }
    }

    if (strcmp(argc[1], "new") == 0) {
        checkTFA(argv, 3);
        mkNew(argc[3], argc[2]);
    }

    else if (strcmp(argc[1], "remove") == 0) {
        checkTFA(argv, 3);

        short i=0;
        if (0==strcmp(argc[2], "-a") || 0==strcmp(argc[2], "--dont-ask")) {
            i++;
            checkTFA(argv, 3+i);
            askDownload = false;
        }

        delPkg(argc[3+i], argc[2+i]);
    }

    else if (strcmp(argc[1], "get") == 0) {
        checkTFA(argv, 3);

        short i=0;
        if (0==strcmp(argc[2], "-a") || 0==strcmp(argc[2], "--dont-ask")) {
            i++;
            checkTFA(argv, 3+i);
            askDownload = false;
        }

        if (0==strcmp(argc[2+i], "-f") || 0==strcmp(argc[2+i], "--force")) {
            i++;
            checkTFA(argv, 3+i);
            forceDownload = true;
        }

        mirrorName = strdup(argc[2+i]);
        getPkg(argc[3+i]);
    }

    else {
        logerr("Incorrect args. See --help.");
        exit(1);
    }

    return 0;
}
