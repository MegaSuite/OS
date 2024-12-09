#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <stack>
#include <string>
#include <vector>
#include <algorithm>

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

void print_entry(const char* path, const char* name, const struct stat& statbuf, int depth) 
{
    // indent
    for (int i = 0; i < depth; i++)
        printf("    ");

    // permissions
    print_permissions(statbuf.st_mode);

    //hardlink
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
    printf("%s", name);
    if (S_ISDIR(statbuf.st_mode))
        printf("/");
    printf("\n");
}

void printdir(char *dir) 
{
    std::stack<std::pair<std::string, int>> dir_stack;
    dir_stack.push({dir, 0});

    while (!dir_stack.empty()) 
    {
        std::string current_path = dir_stack.top().first;
        int depth = dir_stack.top().second;
        dir_stack.pop();

        DIR *dp;
        struct dirent *entry;
        struct stat statbuf;
        
        // output path
        if (depth > 0)
            printf("\n%s:\n", current_path.c_str());

        // open dir
        if ((dp = opendir(current_path.c_str())) == NULL) 
        {
            fprintf(stderr, "Cannot open directory: %s\n", current_path.c_str());
            continue;
        }

        // store all subs
        std::vector<std::pair<std::string, struct stat>> entries;
        
        while ((entry = readdir(dp)) != NULL) 
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", current_path.c_str(), entry->d_name);
            
            if (lstat(path, &statbuf) == -1) 
            {
                fprintf(stderr, "Error getting stat for %s\n", path);
                continue;
            }
            
            entries.push_back({std::string(entry->d_name), statbuf});
        }
        closedir(dp);

        // sort
        std::sort(entries.begin(), entries.end(), 
            [](const auto& a, const auto& b) {
                return a.first < b.first;
            }
        );

        // output
        for (const auto& entry : entries) 
        {
            const std::string& name = entry.first;
            const struct stat& sb = entry.second;
            
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_path.c_str(), name.c_str());
            
            print_entry(full_path, name.c_str(), sb, depth);
            
            if (S_ISDIR(sb.st_mode)) 
            {
                dir_stack.push({full_path, depth + 1});
            }
        }
    }
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
    printdir(resolved_path);

    return 0;
}

