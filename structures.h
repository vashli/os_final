#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fuse.h>
struct server{
    char ip[32];
    int port;
};

struct basic_data {
    char errorlog[256];
    int cache_size;
    char cache_replacment[3];
    int timeout;
};


struct storage_data{
    char diskname[256];
    char mountpoint[256];
    int raid;
    int servers_num;
    struct server servers[64];
    struct server hotswap;
};




#pragma pack(1)   // this helps to pack the struct to 5-bytes
struct syscall_data_client{
    int syscall;
    char path[256];
    mode_t mode;

};

//readdir_names size???? 128 file * (16) saxelis_zoma
struct syscall_data_server{
    struct stat st;
    struct fuse_file_info fi;
    int res;
    int dir_n_files;
    char readdir_names[128][16];
};

// struct getattr_client{
//     char path[256];
// };
//
// struct getattr_sever{
//     struct stat st;
//     int res;
// };
#endif

#pragma pack(0)   // turn packing off
