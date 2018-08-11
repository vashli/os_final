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

int mountpoint_index;
int sfds[MAX_SERVERS_ON_STORAGE];

// unda gaarkvio romeli mountpointidan gamoidzaxeba da shesabamisad
// romeli serveris path unda sheesabamebodes?
static int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] %s\n", path );
	printf( "\tAttributes of %s requested\n", path );

	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = GETATTR;
	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );



	struct getattr_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct getattr_data_server), 0);
	// printf("miigo <3  %d\n", n );

	memcpy ( st, &(receive_data.st), sizeof(struct stat) );

	//
	// dev_t     st_dev;         /* ID of device containing file */
	// ino_t     st_ino;         /* Inode number */
	// mode_t    st_mode;        /* File type and mode */
	// nlink_t   st_nlink;       /* Number of hard links */
	// uid_t     st_uid;         /* User ID of owner */
	// gid_t     st_gid;         /* Group ID of owner */
	// dev_t     st_rdev;        /* Device ID (if special file) */
	// off_t     st_size;        /* Total size, in bytes */
	// blksize_t st_blksize;     /* Block size for filesystem I/O */
	// blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */

	// printf("received st_dev  %d\n",st->st_dev );
	// printf("received st_ino  %d\n",st->st_ino);
	// printf("received st_mode %d \n",st->st_mode );
	// printf("received st_nlink %d\n",st->st_nlink );
	// printf("received st_uid  %d\n",st->st_uid );
	// printf("received st_gid  %d\n",st->st_gid );
	// printf("received st_rdev %d \n",st->st_rdev );
	// printf("received st_size %d \n",st->st_size );
	// printf("received st_rdev %d \n",st->st_rdev );
	// printf("received st_blksize %d \n",st->st_blksize );
	// printf("received st_blocks %d \n",st->st_blocks );


	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!getattr] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;
}

static int do_opendir(const char *path, struct fuse_file_info *fi){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[opendir]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = OPENDIR;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );



	struct opendir_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct opendir_data_server), 0);
	// printf("miigo <3  %d\n", n );

	// copy unda???? ara ls ar mushaobs mere
	fi = &receive_data.fi;

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!opendir] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{

	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	// printf( "[readdir]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = READDIR;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );


	struct readdir_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct readdir_data_server), 0);
	// printf("miigo <3  %d\n", n );


	int i = 0;
	for(;i < receive_data.dir_n_files; i++){
		if(filler(buffer, receive_data.readdir_names[i], NULL, 0) != 0){
			printf("filler error %s \n", receive_data.readdir_names[i] );

		}

	}
	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!readdir] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;
}

static int do_mkdir(const char * path, mode_t mode){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[mkdir]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = MKDIR;


	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	//send mode
	n = send(sfd, &mode, sizeof(mode_t), 0);
	// printf("sent mode %d\n", n );


	struct mkdir_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct mkdir_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!mkdir] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;

}

static int do_releasedir(const char *path, struct fuse_file_info *fi){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[releasedir]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = RELEASEDIR;



	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	//send fi
	n = send(sfd, fi, sizeof(struct fuse_file_info), 0);
	// printf("sent fi %d\n", n );



	struct releasedir_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct releasedir_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!release] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;

}
static int do_rmdir(const char *path){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[rmdir]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = RMDIR;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );


	struct rmdir_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct rmdir_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!rmdir] %s, %d\n", path , receive_data.res);
	}


	return receive_data.res;

}

static int do_rename(const char *path, const char *new_path){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[rename]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = RENAME;


	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	//send newpath //check PATH_MAX

	n = send(sfd, new_path, PATH_MAX, 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );



	struct rename_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct rename_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!rename] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;

}

static int do_mknod(const char *path, mode_t mode, dev_t dev){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[mknod]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = MKNOD;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	//send mode dev
	struct mknod_data_client send_data2;
	send_data2.mode = mode;
	send_data2.dev = dev;
	n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("sent mode dev  %d\n", n );


	struct mknod_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct mknod_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!mknod] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;
}

static int do_open(const char *path, struct fuse_file_info *fi){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[open]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = OPEN;


	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	//send fi
	n = send(sfd, fi, sizeof(struct fuse_file_info), 0);
	// printf("sent fi  %d\n", n );


	struct open_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct open_data_server), 0);
	// printf("miigo <3  %d\n", n );


	fi->fh = receive_data.open_fd;
	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!oprn] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;
}

static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[read]  %s\n", path );
	printf("size %d, offset %d, buffer_size % d\n",
	 (int)size, (int)offset, (int)sizeof(*buffer) );


	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = READ;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	// send read_data_client
	struct read_data_client send_data2;
	memcpy(&send_data2.fi, fi, sizeof(struct fuse_file_info));
	send_data2.size = size;
	send_data2.offset = offset;
	n = send(sfd, &send_data2, sizeof(struct read_data_client), 0);
	// printf("sent read_data_client %d\n", n );



	struct read_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct read_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if(n > 0 && receive_data.res > 0) {
		// n = recv(sfd, buffer, size, 0);
		n = recv(sfd, buffer, receive_data.res, 0);
		printf("BUFFERSHI CHAWERA  %d\n", n );
	}


	// printf("size %d, offset %d, buffer_size % d\n",
	//  (int)size, (int)offset, (int)sizeof(*buffer) );


	 if( receive_data.res < 0) {
		 printf( "[!!!!!!!!!read] %s, %d\n", path , receive_data.res);
	 }

	return receive_data.res;
}

static int do_write( const char *path, const char *buffer, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{

	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[write]  %s\n", path );
	printf("size %d, offset %d\n", (int)size, (int)offset );


	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = WRITE;


	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );


	struct write_data_client send_data2;
	memcpy(&send_data2.fi, fi, sizeof(struct fuse_file_info));
	send_data2.size = size;
	send_data2.offset = offset;
	n = send(sfd, &send_data2, sizeof(struct write_data_client), 0);
	// printf("sent write_data_client %d\n", n );




	n = send(sfd, buffer, size, 0);
	printf("WRITESTVIS GAIGZAVNA %d %s\n", n , buffer);



	struct write_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct write_data_server), 0);
	// printf("miigo <3  %d\n", n );

	// fis ar vabruneb saertod ar unda mgoni
	// memcpy(fi, &(receive_data.fi), sizeof(struct fuse_file_info));

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!write] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;
}


static int do_truncate(const char *path, off_t newsize){


	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[truncate]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = TRUNCATE;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	//send newsize
	n = send(sfd, &newsize, sizeof(off_t), 0);
	// printf("sent newsize%d\n", n );

	struct truncate_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct truncate_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!truncate] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;

}

static int do_release(const char *path, struct fuse_file_info *fi){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[release]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = RELEASE;

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	//send fi
	n = send(sfd, fi, sizeof(struct fuse_file_info), 0);
	// printf("sent fi  %d\n", n );


	struct release_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct release_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!release] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;


}


static int do_unlink(const char *path){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[unlink]  %s\n", path );

	struct syscall_data_client send_data;
	send_data.syscall = UNLINK;
	strcpy(send_data.path, path);

	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	struct unlink_data_server receive_data;

	n = recv(sfd, &receive_data, sizeof(struct unlink_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!unlink] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;
}


static int do_access(const char *path, int mode){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[access]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = ACCESS;


	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	//send mode
	n = send(sfd, &mode, sizeof(int), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	struct access_data_server receive_data;

	n = recv(sfd, &receive_data, sizeof(struct access_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!access] %s, %d\n", path , receive_data.res);
	}

	return receive_data.res;
}



static int do_create(const char *path, mode_t mode, struct fuse_file_info *fi){
	// dasaweria
	// int mountpoint_index = 0;
	int server_index = 0;
	int sfd = sfds[server_index];


	printf( "[create]  %s\n", path );

	struct syscall_data_client send_data;
	strcpy(send_data.path, path);
	send_data.syscall = CREATE;


	int n = send(sfd, &send_data, sizeof(struct syscall_data_client), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );

	//send mode
	n = send(sfd, &mode, sizeof(int), 0);
	// printf("gagzavnaaaaaaaaaaaaaaaaaa <3  %d\n", n );


	struct create_data_server receive_data;
	n = recv(sfd, &receive_data, sizeof(struct create_data_server), 0);
	// printf("miigo <3  %d\n", n );

	if( receive_data.res < 0) {
		printf( "[!!!!!!!!!create] %s, %d\n", path , receive_data.res);
	}

	fi->fh = receive_data.fd;
	return receive_data.res;
}



static struct fuse_operations operations = {
    .getattr	= do_getattr,
	.opendir	= do_opendir,
    .readdir	= do_readdir,
	.mkdir 		= do_mkdir,
	.releasedir = do_releasedir,
	.rmdir 		= do_rmdir,
	.rename 	= do_rename,
	.mknod 		= do_mknod,
	.open		= do_open,
	.read		= do_read,
	.write 		= do_write,
	.truncate 	= do_truncate,
	.release 	= do_release, //something doesnt work
	.unlink 	= do_unlink,
	.access 	= do_access,
	.create 	= do_create,
	// .utime 		= do_utime,
};


int connect_to_servers(int index){
	struct storage_data stor = storages_data[index];
	int i = 0;
	// tmp to test several mountpoints
	// for(; i < stor.servers_num; i++){
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
	// }

}

int main(int argc, char *argv[]){
    printf("client main\n");
    printf("config %s\n", argv[1] );

    if(parse_config_file(argv[1]) == -1){
        return -1;
    }

    int i = 0;
    for(; i < storages_count; i++){
		mountpoint_index = i;
        struct storage_data stor = storages_data[i];
        if(stor.raid == 1){
            printf("stor.mountpoint %s\n", stor.mountpoint );
            pid_t pid = fork();
            if(pid == (pid_t) -1) perror("couldn fork");

            else if (pid == 0) { // child process
                printf("in child\n" );

                connect_to_servers(i);

				//run fuse_main


                // strcpy(argv[1], stor.mountpoint);
                // return fuse_main( argc, argv, &operations, NULL );

                // run in foreground
                int argcc = 4;
                char * argvv[argcc];
                argvv[0] = malloc(256);
                argvv[1] = malloc(256);
                argvv[2] = malloc(256);
				argvv[3] = malloc(256);
                strcpy(argvv[0], argv[0]);
                argvv[1] = "-f";
				argvv[2] = "-s";
                strcpy(argvv[3], stor.mountpoint);



				    // run in background
				//
				// int argcc = 2;
				// char * argvv[argcc];
				// argvv[0] = malloc(256);
				// argvv[1] = malloc(256);
				// // argvv[2] = malloc(256);
				// strcpy(argvv[0], argv[0]);
				// // argvv[1] = "-f";
				// strcpy(argvv[1], stor.mountpoint);

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
