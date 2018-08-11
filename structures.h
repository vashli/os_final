#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fuse.h>
#include <dirent.h>

#include "constants.h"
struct server{
    char ip[32];
    int port;
};

struct basic_data {
    char errorlog[PATH_MAX];
    int cache_size;
    char cache_replacment[CACHE_REPL_ALG_SIZE];
    int timeout;
};


struct storage_data{
    char diskname[MAX_FILES_NAME];
    char mountpoint[PATH_MAX];
    int raid;
    int servers_num;
    struct server servers[MAX_SERVERS_ON_STORAGE];
    struct server hotswap;
};




#pragma pack(1)   // this helps to pack the struct to 5-bytes
// struct syscall_data_client{
//     int syscall;
//     char path[PATH_MAX];
//     struct fuse_file_info fi;
//     char new_path[PATH_MAX];
//     mode_t mode;
//     dev_t dev;
//     size_t size;
//     off_t offset;
//     int new_size;
//     struct stat st;
// };
struct syscall_data_client{
    int syscall;
    char path[PATH_MAX];
};

// //readdir_names size???? 128 file * (16) saxelis_zoma
// struct syscall_data_server{
//     struct stat st;
//     struct fuse_file_info fi;
//     int res;
//     int dir_n_files;
//     char readdir_names[MAX_FILES_NUM][MAX_FILES_NAME];
//     int open_fd;
// };


struct getattr_data_server{
    struct stat st;
    int res;
};

struct opendir_data_server{
    struct fuse_file_info fi;
    int res;
};

struct readdir_data_server{
    int dir_n_files;
    char readdir_names[MAX_FILES_NUM][MAX_FILES_NAME];
    int res;
};

struct mkdir_data_server{
    int res;
};


struct releasedir_data_server{
    int res;
};

struct rmdir_data_server{
    int res;
};

struct rename_data_server{
    int res;
};

struct mknod_data_server{
    int res;
};

struct mknod_data_client{
    mode_t mode;
    dev_t dev;
};


struct open_data_server{
    int open_fd;
    int res;
};

struct read_data_server{
    int res;
};

struct read_data_client{
    struct fuse_file_info fi;
    size_t size;
    off_t offset;
};


struct write_data_server{
    int res;
};

struct write_data_client{
    struct fuse_file_info fi;
    size_t size;
    off_t offset;
};

struct truncate_data_server{
    int res;
};

struct release_data_server{
    int res;
};

struct unlink_data_server{
    int res;
};


struct access_data_server{
    int res;
};

struct create_data_server{
    int fd;
    int res;
};

#pragma pack(0)   // turn packing off


#endif
