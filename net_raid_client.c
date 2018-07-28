#include <stdio.h>
#include <unistd.h>
#include "config_parser.c"
#include <sys/wait.h>

int main(int argc, char *argv[]){
    printf("client main\n");
    parse_config_file("config_file");

    int i = 0;
    for(; i < storages_count; i++){
        struct storage_data stor = storages_data[i];
        if(stor.raid == 1){
            printf("stor.mountpoint %s\n", stor.mountpoint );

            pid_t pid = fork();
            if(pid == (pid_t) -1) perror("couldn fork");
            else if (pid == 0) { // child process
                char * argv[2];
                argv[1] = "-f";
                argv[2] = stor.mountpoint;

                int fuse_execv =execv("ssfs/ssfs", argv);
                if(fuse_execv == -1){
                    printf("couldnt execv\n");
                }
            }
            else{

                printf("in parent \n" );
                waitpid(-1, NULL, WUNTRACED);
            }



        }
    }

    return 0;

}
