#include <stdio.h>
#include "config_parser.c"
int main(int argc, char *argv[]){
    printf("client main\n");
    parse_config_file("config_file");
    return 0;

}
