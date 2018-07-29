#define FUSE_USE_VERSION 30

#include <fuse.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include <arpa/inet.h>

#include "structures.h"
#include "constants.h"
#include "config_parser.c"


int sfds[64];

// unda gaarkvio romeli mountpointidan gamoidzaxeba da shesabamisad
// romeli serveris path unda sheesabamebodes?
static int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] %s\n", path );
	printf( "\tAttributes of %s requested\n", path );

	// dasaweria
	int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = GETATTR;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	struct syscall_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct syscall_data_server), 0);
	printf("miigo <3  %d\n", n );

	printf("received st.uid %d  \n",receive_data.st.st_uid );

	// st = &receive_data.st;
	// printf("set uid %d  Current %d\n",st->st_uid, getuid() );
	// return receive_data.res;

	memcpy ( st, &(receive_data.st), sizeof(struct stat) );
	printf("getattr res %d\n", receive_data.res);
	// if(receive_data.res < 0) {
	// 	return -errno;
	// }

	return receive_data.res;
	// return 0;
}

static int do_opendir(const char *path, struct fuse_file_info *fi){
	// dasaweria
	int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[opendir]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = OPENDIR;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	struct syscall_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct syscall_data_server), 0);
	printf("miigo <3  %d\n", n );

	// copy unda???? ara ls ar mushaobs mere
	fi = &receive_data.fi;


	return receive_data.res;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{

	// dasaweria
	int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[readdir]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = READDIR;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	struct syscall_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct syscall_data_server), 0);
	printf("miigo <3  %d\n", n );

	if(receive_data.res < 0){
		return -errno;
	}

	int i = 0;

	for(;i < receive_data.dir_n_files; i++){
		if(filler(buffer, receive_data.readdir_names[i], NULL, 0) != 0){
			printf("filler error %s \n", receive_data.readdir_names[i] );
			return -errno;
		}

	}

	return 0;
}

static int do_mkdir(const char * path, mode_t mode){
	// dasaweria
	int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[mkdir]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = MKDIR;
	send_data.mode = mode;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	struct syscall_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct syscall_data_server), 0);
	printf("miigo <3  %d\n", n );

	// copy unda????
	if(receive_data.res < 0){
		return -errno;
	}
	return receive_data.res;

}

static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
    printf( "--> Reading file %s\n", path );
    char file54Text[] = "Hello World From File54!";
    char file349Text[] = "Hello World From File349!";
    char *selectedText = NULL;

    if ( strcmp( path, "/file54" ) == 0 )
		selectedText = file54Text;
	else if ( strcmp( path, "/file349" ) == 0 )
		selectedText = file349Text;
	else
		return -1;

    memcpy( buffer, selectedText + offset, size );
	return strlen( selectedText ) - offset;
}


static struct fuse_operations operations = {
    .getattr	= do_getattr,
	.opendir	= do_opendir,
    .readdir	= do_readdir,
	.mkdir 		= do_mkdir,
    .read		= do_read,
};


int connect_to_servers(int index){
	struct storage_data stor = storages_data[index];
	int i = 0;
	for(; i < stor.servers_num; i++){
		char * server_ip = stor.servers[i].ip;
		int port = stor.servers[i].port;

		int sfd;
		struct sockaddr_in addr;
		int ip;
		char buf[1024];
		sfd = socket(AF_INET, SOCK_STREAM, 0);
		inet_pton(AF_INET, server_ip, &ip);

		printf("ip : %d\n", ip  );
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = ip;

		int connection_status = connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));

		// sfd ebi unda sheinaxo
		// daforkilia da sfds tavisi aqvs da mixedavs??
		sfds[i] = sfd;


		if(connection_status == -1){
			printf("error with the connection\n" );
			return -1;
		}else   printf("connected\n" );

		// write(sfd, "qwe", 3);
		// read(sfd, &buf, 3);
		// printf("read from server: %s\n", buf);
		// sleep(600);

		//mount via fuse

		// close(sfd);
	}

}

int main(int argc, char *argv[]){
    printf("client main\n");
    printf("config %s\n", argv[1] );

    if(parse_config_file(argv[1]) == -1){
        return -1;
    }

    int i = 0;
    for(; i < storages_count; i++){
        struct storage_data stor = storages_data[i];
        if(stor.raid == 1){
            printf("stor.mountpoint %s\n", stor.mountpoint );
            pid_t pid = fork();
            if(pid == (pid_t) -1) perror("couldn fork");

            else if (pid == 0) { // child process
                printf("in child\n" );

                connect_to_servers(i);

				//run fuse_main

                // run in background
                // strcpy(argv[1], stor.mountpoint);
                // return fuse_main( argc, argv, &operations, NULL );

                // run in foreground
                int argcc = 3;
                char * argvv[argcc];
                argvv[0] = malloc(256);
                argvv[1] = malloc(256);
                argvv[2] = malloc(256);
                strcpy(argvv[0], argv[0]);
                argvv[1] = "-f";
                strcpy(argvv[2], stor.mountpoint);

                return fuse_main( argcc, argvv, &operations, NULL );

            }
            else{

                printf("in parent \n" );
                waitpid(pid, NULL, WUNTRACED);
            }



        }
    }

    return 0;

}
