#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

// 将文件权限转换为可读字符串
void print_permissions(mode_t mode) {
    char perms[11] = "----------";
    
    // 文件类型
    if (S_ISDIR(mode)) perms[0] = 'd';
    else if (S_ISLNK(mode)) perms[0] = 'l';
    
    // 用户权限
    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = 'x';
    
    // 组权限
    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = 'x';
    
    // 其他用户权限
    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = 'x';
    
    printf("%s ", perms);
}

void printdir(char *dir, int depth) {
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char path[1024];

    // 打开目录
    if ((dp = opendir(dir)) == NULL) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    // 改变当前工作目录
    chdir(dir);

    // 遍历目录项
    while ((entry = readdir(dp)) != NULL) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // 获取文件状态
        if (lstat(entry->d_name, &statbuf) == -1) {
            fprintf(stderr, "Error getting stat for %s\n", entry->d_name);
            continue;
        }

        // 打印缩进
        for (int i = 0; i < depth; i++)
            printf("    ");

        // 打印文件信息
        print_permissions(statbuf.st_mode);

        // 打印硬链接数
        printf("%3ld ", statbuf.st_nlink);

        // 打印所有者和组
        struct passwd *pwd = getpwuid(statbuf.st_uid);
        struct group *grp = getgrgid(statbuf.st_gid);
        printf("%s %s ", pwd ? pwd->pw_name : "unknown", 
               grp ? grp->gr_name : "unknown");

        // 打印文件大小
        printf("%8ld ", statbuf.st_size);

        // 打印最后修改时间
        char timebuf[80];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&statbuf.st_mtime));
        printf("%s ", timebuf);

        // 打印文件名
        printf("%s", entry->d_name);

        // 如果是目录，递归查询
        if (S_ISDIR(statbuf.st_mode)) {
            printf("/");
            printf("\n");
            
            // 构建完整路径
            snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
            printdir(path, depth + 4);
        } else {
            printf("\n");
        }
    }

    // 返回上级目录
    chdir("..");

    // 关闭目录
    closedir(dp);
}

int main(int argc, char *argv[]) {
    char *dir;
    
    // 如果没有指定目录，使用当前目录
    if (argc == 1)
        dir = ".";
    else
        dir = argv[1];

    printf("Directory scan of %s:\n", dir);
    printdir(dir, 0);

    return 0;
}
