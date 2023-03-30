#include <arpa/inet.h>
#include <unistd.h>
#include "../../include/json_inc.h"
#include "../../include/serverClient_inc.h"

#define MAXMSGLEN		1024

struct ListClientThread *ll = NULL;


int main (int argc, char *argv[]){
    int sock, sockfd, sockCli;
    struct sockaddr_in sockServer, sockClient;
    pthread_t threadClient;
    char buffer[MAXMSGLEN];
    socklen_t peer_addr_size;
    ll = (struct ListClientThread *)malloc(sizeof(struct ListClientThread));
    listInit(ll);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        handle_error("socket");
}
