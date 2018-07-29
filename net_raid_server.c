#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include <stdlib.h>
#define BACKLOG 10


#include "structures.h"
#include "constants.h"
#include "config_parser.c"


char* server_storage_path;  // yvela servers tavisi eqneba? :( :ddd


void server_do_getattr( struct syscall_data_client *receive_data,
                        struct syscall_data_server *send_data,
                        char *fullpath){
    printf("GETATTR\n" );
    int res = stat(fullpath, &(send_data->st));
    if(res < 0) {
        send_data->res = -errno;
    }else{
        send_data->res = res;
    }

    printf("server getattr res %d path: %s\n", send_data->res, receive_data->path );
}

void server_do_opendir( struct syscall_data_client *receive_data,
                        struct syscall_data_server *send_data,
                        char *fullpath){
    printf("OPENDIR\n" );
    DIR *dir =  opendir(fullpath);
    if(dir == NULL){
        printf("opendir res -1 %s\n", fullpath );
        // send_data.res = -1;
        send_data->res = -errno;
    }else{
        printf("opendir res 0 %s\n", fullpath );
        send_data->res = 0;
    }
    send_data->fi.fh = (intptr_t) dir;
}

void server_do_readdir( struct syscall_data_client *receive_data,
                        struct syscall_data_server *send_data,
                        char *fullpath){
    printf("READDIR\n" );

    //check whiles if etc
    DIR *dp =  opendir(fullpath);
    struct dirent *de = readdir(dp);
    if (de == 0) {
       // send_data.res = -1;
       send_data->res = -errno;
    }
    int i = 0;
    do {
        if (strlen(de->d_name) > 16) {
            printf("filename too large %s\n", de->d_name );
            // receive_data.filler(send_data.readdir_buffer, de->d_name, NULL, 0) != 0

            // send_data.res = -1;
            send_data->res = -errno;
            break;
        }else{
            printf("AEEEEEEEEEEEEEE filename %s\n", de->d_name );
            strcpy(send_data->readdir_names[i], de->d_name);
        }
        i++;
    } while ((de = readdir(dp)) != NULL);

    send_data->dir_n_files = i;
    send_data->res = 0;

}

void server_do_mkdir(struct syscall_data_client *receive_data,
                    struct syscall_data_server *send_data,
                    char *fullpath){
    printf("MKDIR\n" );
    int res = mkdir(fullpath, receive_data->mode);
    if(res < 0)
        res = -errno;
    send_data->res = res;
}

void server_do_releasedir(struct syscall_data_client *receive_data,
                        struct syscall_data_server *send_data,
                        char *fullpath){
    printf("RELEASEDIR\n" );
    int res = closedir((DIR *) (uintptr_t) receive_data->fi.fh  );
    if(res < 0)
        res = -errno;
    send_data->res = res;
}

void server_do_rmdir(struct syscall_data_client *receive_data,
                    struct syscall_data_server *send_data,
                    char *fullpath){
    printf("RMDIR\n" );
    int res = rmdir(fullpath);
    if(res < 0)
        res = -errno;
    send_data->res = res;
}

void server_syscall_handler(struct syscall_data_client *receive_data,
                            struct syscall_data_server *send_data){
    char fullpath[512];
    strcpy(fullpath, server_storage_path);
    strcat(fullpath, receive_data->path);
    if(receive_data->syscall == GETATTR) {
        server_do_getattr(receive_data, send_data, fullpath);
    }else if(receive_data->syscall == OPENDIR){
        server_do_opendir(receive_data, send_data, fullpath);
    }else if(receive_data->syscall == READDIR){
        server_do_readdir(receive_data, send_data, fullpath);
    }else if(receive_data->syscall == MKDIR){
        server_do_mkdir(receive_data, send_data, fullpath);
    }else if(receive_data->syscall == RELEASEDIR){
        server_do_releasedir(receive_data, send_data, fullpath);
    }else if(receive_data->syscall == RMDIR){
        server_do_rmdir(receive_data, send_data, fullpath);
    }else{
        printf("unknown syscall %d\n", receive_data->syscall);
    }

}


void client_handler(int cfd) {
    int data_size;
    while (1) {
        struct syscall_data_client receive_data;
        int n = recv(cfd, &receive_data, sizeof(struct syscall_data_client), 0);

        if(n == 0) {
            printf("lost connection with client. closing cfd\n" );
            break;
        }
        printf("miigo <3  %d , %s\n", n, receive_data.path );

        struct syscall_data_server send_data;
        server_syscall_handler(&receive_data, &send_data);

        n = send(cfd, &send_data, sizeof(struct syscall_data_server), 0);
        printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
    }
    close(cfd);
}


int main(int argc, char* argv[])
{

    // ./net_raid_server 127.0.0.1 10001 /home/mari/Desktop/final_project_desktop/storage_dir1
    // ./net_raid_server 127.0.0.1 10002 /home/mari/Desktop/final_project_desktop/storage_dir2
    char* ip = argv[1];
    int port = str_to_int(argv[2]);
    if(port == -1) {
        printf("invalid port %s\n", argv[2] );
        return -1;
    }
    server_storage_path = argv[3];

    DIR *dir;
    if ((dir = opendir (server_storage_path)) == NULL) {
        printf("invalid path to storage %s\n", argv[3] );
        closedir(dir);
        return -1;

    }
    closedir(dir);

    printf("ip = %s\n", ip);
    printf("port = %d\n", port);
    printf("server_storage_path = %s\n", server_storage_path);

    int sfd, cfd;
    struct sockaddr_in addr;
    struct sockaddr_in peer_addr;

    sfd = socket(AF_INET, SOCK_STREAM, 0);

    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    listen(sfd, BACKLOG);

    while (1)
    {
        printf("waiting for client: \n" );
        int peer_addr_size = sizeof(struct sockaddr_in);
        cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);

        switch(fork()) {
            case -1:
                exit(100);
            case 0:
                close(sfd);
                client_handler(cfd);
                exit(0);
            default:
                close(cfd);
        }
    }
    close(sfd);
}
