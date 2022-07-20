#include "myftpc.h"

int main(int *argc, char *argv[])
{
    while(1) {
        exec_command();
    }
}

int exec_command()
{
    char lbuf[BUF_LEN], *av[NARGS];
    int ac;
    struct command_table_t* p;
    fprintf(stderr, "\x1b[34mmyFTP%%\x1b[0m ");
    if(fgets(lbuf, sizeof lbuf, stdin) == NULL) {
        fprintf(stderr, "\n");
        return 0;
    }
    lbuf[strlen(lbuf) - 1] = '\0';
    getargs(&ac, av, lbuf);
    if(ac < 1) return 0;

    for(p = cmd_tbl; p->cmd; p++) {
        if(strcmp(av[0], p->cmd) == 0) {
            (p->func)(&ac, av);
            break;
        }
    }
    if(p->cmd == NULL) {
        fprintf(stderr, "%s: Unknown command. Please Try help.\r\n", av[0]);
        return -1;
    }
}

int getargs(int *argc, char *argv[], char *lbuf)
{
    *argc = 0;
    argv[0] = NULL;

    while(1) {
        while(isblank(*lbuf)) lbuf++;
        if(*lbuf == '\0') return 0;
        argv[(*argc)++] = lbuf;

        if (*argc > NARGS) return -1;
        while(*lbuf && !isblank(*lbuf)) lbuf++;
        if(*lbuf == '\0') {
            argv[(*argc)] = NULL;
            return 0;
        }
        *lbuf++ = '\0';
    }
}



static inline void show_descr(struct command_table_t *p) { printf("\x1b[1m%s\n\x1b[0m%s", p->cmd, p->descr); }
int help(int *argc, char *argv[])
{
    struct command_table_t* p;
    if(*argc == 1) {
        for(p = cmd_tbl; p->cmd; p++) {
            show_descr(p);
        }
    } else if(*argc == 2) { 
        for(p = cmd_tbl; p->cmd; p++) {
            if(strcmp(argv[1], p->cmd) == 0) {
                show_descr(p);
                return 0;
            }
        }
        if(p->cmd == NULL) {
            fprintf(stderr, "%s: Unknown command.\r\n", argv[1]);
        }
    } else {
        fprintf(stderr, "Too many args for help.\r\n");
    }
    return -1;
}

int quit(int *argc, char *argv[]) { exit(0); }

int pwd(int *argc, char *argv[])
{}
int cd(int *argc, char *argv[])
{}
int dir(int *argc, char *argv[])
{}

int lpwd(int *argc, char *argv[])
{
    char fullpath[PATH_MAX];
    if (getcwd(fullpath, sizeof(fullpath)) < 0) {
        perror("getcwd");
        return -1;
    }
    fprintf(stderr, "%s\r\n", fullpath);
    return 0;
}

int lcd(int *argc, char *argv[])
{
    char relpath[PATH_MAX];
    char *fullpath;
    if (*argc != 2) {
        fprintf(stderr, "lcd requires exact 1 argument\r\n");
        return -1;
    }

    if ((fullpath = normalize_path(argv[1])) != NULL) {
        if (chdir(fullpath) < 0) {
            perror("chdir");
        }
    }
    free(fullpath);
    return 0;
}

void show_file(struct stat *st, char *name)
{
    char timestr[BUF_LEN];
    struct tm *tm_ptr;
    tm_ptr = localtime(&st->st_ctime);
    strftime(timestr, BUF_LEN, "%Y %b %d %H:%M", tm_ptr);
    fprintf(stderr, "%12zd %s %s\n", st->st_size, timestr, name);
};

int ldir(int *argc, char *argv[])
{
    DIR *dir;
    struct dirent *entry;
    struct stat st;

    if (*argc > 2) {
        fprintf(stderr, "Too many args given\n");
        return -1;
    }
    if (*argc == 1) {
        argv[1] = ".";
        argv[2] = NULL;
    }

    if (stat(argv[1], &st) < 0) {
        perror("stat");
        return -1;
    }
    if (S_ISDIR(st.st_mode)) {
        if ((dir = opendir(argv[1])) == NULL) {
            perror("opendir");
            return -1;
        }
        while ((entry = readdir(dir)) != NULL) {
            if (stat(entry->d_name, &st) < 0) {
                perror("stat");
                return -1;
            }
            show_file(&st, entry->d_name);
        }
        closedir(dir);
    } else {
        show_file(&st, argv[1]);
    }
    return 0;
}

int ftpget(int *argc, char *argv[])
{}
int ftpput(int *argc, char *argv[])
{}