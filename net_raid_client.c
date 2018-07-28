#include <stdio.h>
#include "config_parser.c"
int main(int argc, char *argv[]){
    printf("client main\n");
    parse_config_file("config_file");

    for(int i = 0; i< 10; i++){

    }
    return 0;
}
