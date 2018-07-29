#define FUSE_USE_VERSION 30

#include <fuse.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include <arpa/inet.h>




#include "config_parser.c"


static int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );

	// GNU's definitions of the attributes (http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html):
	// 		st_uid: 	The user ID of the file’s owner.
	//		st_gid: 	The group ID of the file.
	//		st_atime: 	This is the last access time for the file.
	//		st_mtime: 	This is the time of the last modification to the contents of the file.
	//		st_mode: 	Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
	//		st_nlink: 	The number of hard links to the file. This count keeps track of how many directories have entries for this file. If the count is ever decremented to zero, then the file itself is discarded as soon
	//						as no process still holds it open. Symbolic links are not counted in the total.
	//		st_size:	This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.

	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now

	if ( strcmp( path, "/" ) == 0 )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
	else
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}

	return 0;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	printf( "--> Getting The List of Files of %s\n", path );

	filler( buffer, ".", NULL, 0 ); // Current Directory
	filler( buffer, "..", NULL, 0 ); // Parent Directory

	if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the root directory show the following
	{
		filler( buffer, "file54", NULL, 0 );
		filler( buffer, "file349", NULL, 0 );
	}

	return 0;
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
    .readdir	= do_readdir,
    .read	= do_read,
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
		if(connection_status == -1){
			printf("error with the connection\n" );
			return -1;
		}else   printf("connected\n" );

		write(sfd, "qwe", 3);
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
