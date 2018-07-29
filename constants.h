#ifndef CONSTANTS_H
#define CONSTANTS_H

#define GETATTR     1
#define OPENDIR     2
#define READDIR     3
#define MKDIR       4
#define RELEASEDIR  5
#define RMDIR       6
#define RENAME      7
#define MKNOD       8
#define OPEN        9
#define READ        10
#define WRITE       11
#define TRUNCATE    12
#define ACCESS      13
#define UTIME       14


#define MAX_FILES_NUM 128
#define MAX_FILES_NAME 64 // #define MAX_FILES_NAME 256 linuxshi
#define MAX_SERVERS_ON_STORAGE 64
#define MAX_MOUNTPOINTS_NUM 64

#define CACHE_REPL_ALG_SIZE 4

#endif
