#include <json-c/json.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include "parser.h"
#include "request_tree.h"
#include "json_tree.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("invalid number of argument\n need 2, but you have %d\n", argc);
        for (int i = 0; i < argc; i++) {
            printf("%s\n", argv[i]);
        }
        return 100;
    }
    printf("I am Client\n");
    char *str;
    int fd = 0;
    struct sockaddr_in demoserverAddr;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        printf("Error : Could not create socket\n");
        return 1;
    } else {
        demoserverAddr.sin_family = AF_INET;
        demoserverAddr.sin_port = htons(atoi(argv[1]));
        demoserverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        memset(demoserverAddr.sin_zero, '\0', sizeof(demoserverAddr.sin_zero));
    }

    if (connect(fd, (const struct sockaddr *) &demoserverAddr, sizeof(demoserverAddr)) < 0) {
        printf("ERROR connecting to server\n");
        return 1;
    }

        int yydebug = 1;
        yyparse();
        request_tree tree = get_request_tree();
        if (tree.type == REQUEST_CLOSE) {
            return 0;
        }
        json_object *jobj = tree_to_json(&tree);

        char temp_buff[MAX_SIZE] = {0};

        if (strcpy(temp_buff, json_object_to_json_string(jobj)) == NULL) {
            perror("strcpy");
            return EXIT_FAILURE;
        }

        if (write(fd, temp_buff, strlen(temp_buff)) == -1) {
            perror("write");
            return EXIT_FAILURE;
        }
        ssize_t r;
        char buff[MAX_SIZE] = {0};
        r = read(fd, buff, MAX_SIZE);

        if (r == -1) {
            perror("read");
            return EXIT_FAILURE;
        }

        json_object *response = json_tokener_parse(buff);
        response = json_object_object_get(response, "Response");
        if (json_object_get_type(response) == json_type_array) {
            for (int i = 0; i < json_object_array_length(response); i++) {
                json_object *val = json_object_array_get_idx(response, i);
                printf("READ %d: %s\n", i, json_object_to_json_string(val));
            }
        } else {
            printf("READ: %s\n", json_object_to_json_string(response));
        }
    return 0;
}
