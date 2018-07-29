#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "structures.h"

struct basic_data bas_data;
struct storage_data storages_data[128];
int storages_count = -1;

int check_parsing();
int str_to_int(char* str);
int parse_cache_size(char* last_token);
int parse_config_file(char* file_name);
void parsing_error();


void parsing_error() {
    printf("STOP!! couldn't parse config file\n");
}


int str_to_int(char* str){
    int i = 0;
    for(; i < strlen(str); i++){
        if(str[i] > '9' || str[i] < '0') return - 1;
    }
    return atoi(str);
}

int parse_cache_size(char* last_token){
    int len = strlen(last_token);
    int i = 0;
    int power = -1;
    if(strcmp(last_token + len - 1, "K")==0){
        power = 0;
    }else if(strcmp(last_token + len - 1, "M")==0){
        power = 1;
    }else if(strcmp(last_token + len - 1, "G")==0){
        power = 2;
    }else{
        return -1;
    }

    char num[len];
    memcpy(num, last_token, len-1);
    num[len-1] = '\0';
    int amount = str_to_int(num);

    if(amount < 0) {
        return -1;
    }else{
        int i = 0;
        for(; i < power; i++){
            amount *= 1024;
        }
    }
    bas_data.cache_size = amount;

    return amount;
}


int check_parsing(){
    printf("basic_data:\n");
    printf("errorlog %s\n", bas_data.errorlog);
    printf("cache_size %d\n", bas_data.cache_size);
    printf("cache_replacment %s\n", bas_data.cache_replacment);
    printf("timeout %d\n", bas_data.timeout);

    printf("storages_count %d\n", storages_count  );
    if(storages_count <= 0) {
        return -1;
    }
    int i = 0;
    for(; i < storages_count; i++){
        struct storage_data stor = storages_data[i];
        printf("storage %d:\n", i);

        printf("diskname %s\n", stor.diskname);
        printf("mountpoint %s\n", stor.mountpoint);
        printf("raid %d\n", stor.raid);
        printf("servers_num %d\n", stor.servers_num);

        int j = 0;
        for(; j < stor.servers_num; j++){
            struct server serv = stor.servers[j];
            printf("server %d, ip = %s, port = %d\n",j, serv.ip, serv.port );
        }
        printf("hotswap, ip = %s, port = %d\n", stor.hotswap.ip, stor.hotswap.port );

    }
}

int parse_config_file(char* file_name){
    const char *delimiter_characters = "\n ,";
    int buffer_size = 1024;
    char buffer[buffer_size];
    char *last_token;
    FILE * file = fopen( file_name , "r");



    int token = 0;
    int token_in_storage = 0;
    int server_index = -1;
    int ip = 1;
    int hotswap_index = -1;
    if (file) {
        while( fgets(buffer, buffer_size, file) != NULL ){
            last_token = (char*) strtok( buffer, delimiter_characters );
            while( last_token != NULL ){
                // printf( "%s\n", last_token );

                if(token == 2){
                    strcpy(bas_data.errorlog, last_token);
                }else if(token == 5){
                    if (parse_cache_size(last_token) == -1){
                        parsing_error();
                        return -1;
                    }
                }else if(token == 8){
                    if(strlen(last_token) !=3) {
                        parsing_error();
                        return -1;
                    }else{
                        strcpy(bas_data.cache_replacment, last_token);
                    }
                }else if(token == 11){
                    int timeout = str_to_int(last_token);
                    if(timeout < 0){
                        parsing_error();
                        return -1;
                    }else{
                        bas_data.timeout = timeout;
                    }
                }

                if(strcmp(last_token, "diskname") == 0){
                    // printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaadisk %s\n", last_token );
                    storages_count++;
                    token_in_storage = 0;
                }else if(storages_count >= 0){
                    if(token_in_storage == 2){
                        strcpy(storages_data[storages_count].diskname, last_token);
                    }
                    else if(token_in_storage == 5){
                        strcpy(storages_data[storages_count].mountpoint, last_token);
                    }
                    else if(token_in_storage == 8){
                        int raid = str_to_int(last_token);
                        if(raid != 1 && raid != 5){
                            parsing_error();
                            return -1;
                        }else{
                            storages_data[storages_count].raid = raid;
                        }
                    }
                    else if(strcmp(last_token, "servers") == 0){
                        server_index = 0;
                    }else if( strcmp(last_token, "hotswap") == 0){
                        server_index = -1;
                        hotswap_index = 0;
                        // printf("server index 000000000000000000\n" );
                    }else if(token_in_storage > 10 && (server_index >= 0 || hotswap_index != -1)
                            && strcmp(last_token, "=") != 0){

                        char ip[32];
                        char port[11];

                        int index = 0;
                        while(last_token[index] != ':'){
                            ip[index] = last_token[index];
                            index++;
                        }
                        ip[index] = '\0';
                        index++;
                        int i = index;
                        for(; i < strlen(last_token); i++){
                            port[i - index] = last_token[i];
                        }
                        port[i - index] = '\0';

                        int por = str_to_int(port);
                        if(por < 0){
                            parsing_error();
                            return -1;
                        }else{
                            if(server_index >= 0){
                                storages_data[storages_count].servers_num = server_index+1;
                                strcpy(storages_data[storages_count].servers[server_index].ip, ip);
                                storages_data[storages_count].servers[server_index].port = por;
                                server_index++;
                            }else{
                                strcpy(storages_data[storages_count].hotswap.ip, ip);
                                storages_data[storages_count].hotswap.port = por;
                                hotswap_index = -1;
                            }
                        }

                    }
                }
                token_in_storage++;
                token++;
                last_token =(char*) strtok( NULL, delimiter_characters );
            }

        }
        fclose(file );
        storages_count++;
        if(check_parsing()== -1){
            parsing_error();
            return -1;
        }

        return 0;
    }else{
        printf("wasn't able to open config file\n" );
        return -1;
    }
}
