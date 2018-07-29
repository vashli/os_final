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
struct syscall_data_client{
    int syscall;
    char path[PATH_MAX];
    mode_t mode;
    struct fuse_file_info fi;
    char new_path[PATH_MAX];
};

//readdir_names size???? 128 file * (16) saxelis_zoma
struct syscall_data_server{
    struct stat st;
    struct fuse_file_info fi;
    int res;
    int dir_n_files;
    char readdir_names[MAX_FILES_NUM][MAX_FILES_NAME];
};

#pragma pack(0)   // turn packing off


#endif
