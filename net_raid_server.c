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


void server_do_getattr(char *fullpath, int cfd){
    printf("GETATTR\n" );

    // memcpy ( &(send_data->st), &(receive_data->st), sizeof(struct stat) );

    struct getattr_data_server send_data;
    int res = lstat(fullpath, &(send_data.st));
    if(res < 0) {
        send_data.res = -errno;
    }else{
        send_data.res = res;
    }

    printf("send st_dev  %d\n",send_data.st.st_dev );
    printf("send st_ino  %d\n",send_data.st.st_ino);
    printf("send st_mode %d \n",send_data.st.st_mode );
    printf("send st_nlink %d\n",send_data.st.st_nlink );
    printf("send st_uid  %d\n",send_data.st.st_uid );
    printf("send st_gid  %d\n",send_data.st.st_gid );
    printf("send st_rdev %d \n",send_data.st.st_rdev );
    printf("send st_size %d \n",send_data.st.st_size );
    printf("send st_rdev %d \n",send_data.st.st_rdev );
    printf("send st_blksize %d \n",send_data.st.st_blksize );
    printf("send st_blocks %d \n",send_data.st.st_blocks );


    printf("server getattr res %d path: %s\n", send_data.res, fullpath );


    int n = send(cfd, &send_data, sizeof(struct getattr_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}

void server_do_opendir(char *fullpath, int cfd){
    printf("OPENDIR\n" );

    struct opendir_data_server send_data;

    DIR *dir =  opendir(fullpath);
    if(dir == NULL){
        printf("opendir res -1 %s\n", fullpath );
        // send_data.res = -1;
        send_data.res = -errno;
    }else{
        printf("opendir res 0 %s\n", fullpath );
        send_data.res = 0;
    }
    send_data.fi.fh = (intptr_t) dir;
    int n = send(cfd, &send_data, sizeof(struct opendir_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}

void server_do_readdir(char *fullpath, int cfd){
    printf("READDIR\n" );

    struct readdir_data_server send_data;

    //check whiles if etc
    DIR *dp =  opendir(fullpath);
    struct dirent *de = readdir(dp);
    if (de == 0) {
       // send_data.res = -1;
       send_data.res = -errno;
    }
    int i = 0;
    do {
        if (strlen(de->d_name) > MAX_FILES_NAME) {
            printf("filename too large %s\n", de->d_name );
            // receive_data.filler(send_data.readdir_buffer, de->d_name, NULL, 0) != 0

            // send_data.res = -1;
            send_data.res = -errno;
            break;
        }else{
            printf("AEEEEEEEEEEEEEE filename %s\n", de->d_name );
            strcpy(send_data.readdir_names[i], de->d_name);
        }
        i++;
    } while ((de = readdir(dp)) != NULL);

    send_data.dir_n_files = i;
    send_data.res = 0;

    int n = send(cfd, &send_data, sizeof(struct readdir_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}

void server_do_mkdir(char *fullpath, int cfd){
    printf("MKDIR\n" );

    struct mkdir_data_server send_data;

    mode_t mode;
    int n = recv(cfd, &mode, sizeof(mode_t), 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }

    int res = mkdir(fullpath, mode);
    if(res < 0)
        res = -errno;
    send_data.res = res;

    n = send(cfd, &send_data, sizeof(struct mkdir_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}

void server_do_releasedir(char *fullpath, int cfd){
    printf("RELEASEDIR\n" );

    struct releasedir_data_server send_data;

    struct fuse_file_info fi;
    int n = recv(cfd, &fi, sizeof(struct fuse_file_info), 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }


    int res = closedir((DIR *) (uintptr_t)fi.fh  );
    if(res < 0)
        res = -errno;
    send_data.res = res;

    n = send(cfd, &send_data, sizeof(struct releasedir_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}

void server_do_rmdir(char *fullpath, int cfd){
    printf("RMDIR\n" );

    struct rmdir_data_server send_data;
    int res = rmdir(fullpath);
    if(res < 0)
        res = -errno;
    send_data.res = res;

    int n = send(cfd, &send_data, sizeof(struct rmdir_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}

void server_do_rename(char *fullpath, int cfd){
    printf("RENAME\n" );

    struct rename_data_server send_data;

    char new_path[PATH_MAX];
    int n = recv(cfd, new_path, PATH_MAX, 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }

    char full_new_path[PATH_MAX];
    strcpy(full_new_path, server_storage_path);
    strcat(full_new_path, new_path);
    int res = rename(fullpath, full_new_path);
    if(res < 0)
        res = -errno;
    send_data.res = res;

    n = send(cfd, &send_data, sizeof(struct rename_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}

void server_do_mknod(char *fullpath, int cfd){
    printf("MKNOD\n" );
    struct mknod_data_server send_data;

    struct mknod_data_client receive_data;
    int n = recv(cfd, &receive_data, sizeof(struct mknod_data_client), 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }

    int res = mknod(fullpath, receive_data.mode, receive_data.dev);
    if(res < 0)
        res = -errno;
    send_data.res = res;

    n = send(cfd, &send_data, sizeof(struct mknod_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}


void server_do_open(char *fullpath, int cfd){
    printf("OPEN\n" );

    struct open_data_server send_data;

    struct fuse_file_info fi;
    int n = recv(cfd, &fi, sizeof(struct fuse_file_info), 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }

    int fd =  open(fullpath, fi.flags);
    int res = 0;
    if(fd < 0){
        fd = -errno;
        res = -errno;
    }

    send_data.open_fd = fd;
    send_data.res = res;

    n = send(cfd, &send_data, sizeof(struct open_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}

void server_do_read(char *fullpath, int cfd){
    printf("READ\n" );

    struct read_data_server send_data;

    struct read_data_client receive_data;
    int n = recv(cfd, &receive_data, sizeof(struct read_data_client), 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }

    char buffer[receive_data.size];
    int res =  pread(receive_data.fi.fh, buffer, receive_data.size, receive_data.offset);
    if(res < 0){
        res = -errno;
        printf("res uaryofitia da sendma sheidzleba aurios %d\n", res);
    }
    send_data.res = res;

    // check
    n = send(cfd, &send_data, sizeof(struct read_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa res  %d\n", n );

    if(res > 0) {
        n = send(cfd, buffer, res, 0);
        printf("BUFFERISTVIS GAAGZAVNA %d\n", n );
    }


}


void server_do_write(char *fullpath, int cfd){
    printf("WRITE\n" );

    struct write_data_server send_data;

    struct write_data_client receive_data;
    int n = recv(cfd, &receive_data, sizeof(struct write_data_client), 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }

    char buffer[receive_data.size];
    n = recv(cfd, buffer, receive_data.size, 0);
    printf("WRITESTVIS MIIGO %d %s\n", n, buffer );

    int res =  pwrite(receive_data.fi.fh, buffer, receive_data.size, receive_data.offset);

    printf("after pwrite\n" );
    if(res < 0){
        res = -errno;

    }

    send_data.res = res;

    n = send(cfd, &send_data, sizeof(struct write_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa res  %d\n", n );
    //araa sawiro?
    // memcpy(&(send_data->fi), &(receive_data->fi), sizeof(struct fuse_file_info));
    // int n = send(cfd, buffer, res, 0);

}


void server_do_truncate(char *fullpath, int cfd){
    printf("TRUNCATE\n" );

    struct truncate_data_server send_data;

    off_t new_size;
    int n = recv(cfd, &new_size, sizeof(off_t), 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }

    int res = truncate(fullpath, new_size);
    if(res < 0){
        res = -errno;
    }

    send_data.res = res;

    n = send(cfd, &send_data, sizeof(struct truncate_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa res  %d\n", n );
}

void server_do_release(char *fullpath, int cfd){
    printf("RELEASE\n" );

    struct release_data_server send_data;

    struct fuse_file_info fi;
    int n = recv(cfd, &fi, sizeof(struct fuse_file_info), 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }

    int res = close(fi.fh);
    if(res < 0){
        res = -errno;
    }
    send_data.res = res;
    n = send(cfd, &send_data, sizeof(struct release_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa res  %d\n", n );
}

void server_do_unlink(char *fullpath, int cfd){
    printf("UNLINK\n" );

    struct unlink_data_server send_data;

    int res = unlink(fullpath);
    if(res < 0){
        res = -errno;
    }
    send_data.res = res;

    int n = send(cfd, &send_data, sizeof(struct unlink_data_server), 0);
    printf("gagzavnaaaaaaaaaaaaaaaaaa res  %d\n", n );
}

void server_do_access(char *fullpath, int cfd){
    printf("ACCESS\n" );

    struct access_data_server send_data;
    int mode;
    int n = recv(cfd, &mode, sizeof(int), 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }

    int res = access(fullpath, mode);
    if(res < 0){
        res = -errno;
    }
    send_data.res = res;

    n = send(cfd, &send_data, sizeof(struct access_data_server), 0);
	printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}

void server_do_create(char *fullpath, int cfd){
    printf("CREATE\n" );

    struct create_data_server send_data;
    int mode;
    int n = recv(cfd, &mode, sizeof(int), 0);
    if(n == 0) {
        printf("lost connection with client. cant close cfd\n" );
        // return res to break;
    }

    int fd =  creat(fullpath, mode);
    int res = 0;
    if(fd < 0){
        // fd = -errno;  ????
        res = -errno;
    }

    send_data.fd = fd;
    send_data.res = res;

    n = send(cfd, &send_data, sizeof(struct access_data_server), 0);
	printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
}


void server_syscall_handler(struct syscall_data_client *receive_data,
                            int cfd){
    char fullpath[PATH_MAX];
    strcpy(fullpath, server_storage_path);
    strcat(fullpath, receive_data->path);

    if(receive_data->syscall == GETATTR) {
        server_do_getattr(fullpath, cfd);
    }else if(receive_data->syscall == OPENDIR){
        server_do_opendir(fullpath, cfd);
    }else if(receive_data->syscall == READDIR){
        server_do_readdir(fullpath, cfd);
    }else if(receive_data->syscall == MKDIR){
        server_do_mkdir(fullpath, cfd);
    }else if(receive_data->syscall == RELEASEDIR){
        server_do_releasedir(fullpath, cfd);
    }else if(receive_data->syscall == RMDIR){
        server_do_rmdir(fullpath, cfd);
    }else if(receive_data->syscall == RENAME){
        server_do_rename( fullpath, cfd);
    }else if(receive_data->syscall == MKNOD){
        server_do_mknod(fullpath, cfd);
    }else if(receive_data->syscall == OPEN){
        server_do_open(fullpath, cfd);
    }else if(receive_data->syscall == READ){
        server_do_read(fullpath, cfd);
    }else if(receive_data->syscall == WRITE){
        server_do_write(fullpath, cfd);
    }else if(receive_data->syscall == TRUNCATE){
        server_do_truncate(fullpath, cfd);
    }else if(receive_data->syscall == RELEASE){
        server_do_release(fullpath, cfd);
    }else if(receive_data->syscall == UNLINK){
        server_do_unlink(fullpath, cfd);
    }else if(receive_data->syscall == ACCESS){
        server_do_access(fullpath, cfd);
    }else if(receive_data->syscall == CREATE){
        server_do_create(fullpath, cfd);
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

        // struct syscall_data_server send_data;
        server_syscall_handler(&receive_data,  cfd);

        // n = send(cfd, &send_data, sizeof(struct syscall_data_server), 0);
        // printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );
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
