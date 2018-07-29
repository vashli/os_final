#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include <stdlib.h>
#define BACKLOG 10

#include "config_parser.c"

void client_handler(int cfd) {
    char buf[1024];
    int data_size;
    while (1) {
        data_size = read (cfd, &buf, 1024);
        if (data_size <= 0)
            break;

        printf("read from client: %s\n", buf);
        write (cfd, &buf, data_size);
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
    char* server_storage_path = argv[3];

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
