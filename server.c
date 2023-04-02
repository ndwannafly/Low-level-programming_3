#include <stdio.h>
#include <netinet/in.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "json_execute.h"

int main(int argc, char * argv[]) {
    if(argc != 3){
        printf("invalid number of argument\n need 2, but you have %d\n", argc);
        for(int i=0; i<argc; i++){
            printf("%s\n", argv[i]);
        }
        return 100;
    }
    FILE *f = open_database(argv[2]);

    printf("I am server\n");
    int listenfd = 0, connfd = 0;    //related with the server
    struct sockaddr_in serv_addr;
    int port = atoi(argv[1]);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(port);

    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    printf("binding\n");

    listen(listenfd, 5);
    while (true) {
        printf("listening\n");
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);

        printf("Reading from client\n");

        ssize_t r;

        char buff[MAX_SIZE] = {0};

        r = read(connfd, buff, MAX_SIZE);

        if (r == -1) {
            perror("read");
            return EXIT_FAILURE;
        }

        json_object *obj = json_tokener_parse(buff);

        printf("READ: %s\n", json_object_to_json_string(obj));

        obj = json_execute(obj, f);

        char temp_buff[MAX_SIZE] = {0};

        if (strcpy(temp_buff, json_object_to_json_string(obj)) == NULL)//json_object_to_json_string(jobj)
        {
            perror("strcpy");
        }
        if (write(connfd, temp_buff, strlen(temp_buff)) == -1) {
            perror("write");
        }
    }
}
