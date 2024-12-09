#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

void print_permissions(mode_t mode) 
{
    char perms[11] = "----------";
    
    // type
    if (S_ISDIR(mode)) 
        perms[0] = 'd';
    else if (S_ISLNK(mode)) 
        perms[0] = 'l';
    
    // user
    if (mode & S_IRUSR) 
        perms[1] = 'r';
    if (mode & S_IWUSR) 
        perms[2] = 'w';
    if (mode & S_IXUSR) 
        perms[3] = 'x';
    
    // group
    if (mode & S_IRGRP) 
        perms[4] = 'r';
    if (mode & S_IWGRP) 
        perms[5] = 'w';
    if (mode & S_IXGRP) 
        perms[6] = 'x';
    
    // others
    if (mode & S_IROTH) 
        perms[7] = 'r';
    if (mode & S_IWOTH) 
        perms[8] = 'w';
    if (mode & S_IXOTH) 
        perms[9] = 'x';
    
    printf("%s ", perms);
}


void printdir(char *dir, int depth) 
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char path[1024];

    // open dir
    if ((dp = opendir(dir)) == NULL) 
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    // traverse
    while ((entry = readdir(dp)) != NULL) 
    {
        // ignore . & ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // full path
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        // status
        if (lstat(path, &statbuf) == -1) 
        {
            fprintf(stderr, "Error getting stat for %s\n", path);
            continue;
        }

        // indent
        for (int i = 0; i < depth; i++)
            printf("    ");

        // permissions
        print_permissions(statbuf.st_mode);

        // hard link
        printf("%3ld ", statbuf.st_nlink);

        // owner & group
        struct passwd *pwd = getpwuid(statbuf.st_uid);
        struct group *grp = getgrgid(statbuf.st_gid);
        printf("%s %s ", pwd ? pwd->pw_name : "unknown", grp ? grp->gr_name : "unknown");

        // size
        printf("%8ld ", statbuf.st_size);

        // latest update time
        char timebuf[80];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&statbuf.st_mtime));
        printf("%s ", timebuf);

        // name
        printf("%s", entry->d_name);

        // recurse
        if (S_ISDIR(statbuf.st_mode)) 
        {
            printf("/\n");
            printdir(path, depth + 1);
        } 
        else
            printf("\n");
    }

    // close
    closedir(dp);
}

int main(int argc, char *argv[]) 
{
    char *dir;
    char resolved_path[1024];
    
    // specific or .
    if (argc == 1) 
        dir = ".";
    else
        dir = argv[1];

    // abs path
    if (realpath(dir, resolved_path) == NULL) 
    {
        perror("Error resolving path");
        return 1;
    }

    printf("%s:\n", resolved_path);
    printdir(resolved_path, 0);

    return 0;
}
