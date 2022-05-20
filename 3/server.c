#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "debug.h"

int main(int argc, char *argv[])
{
    int listening_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    CHECKR(listening_socket == -1, "socket");

    int opt = 1;
    int ret = setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    CHECKR(ret == -1, "setsockopt");

    struct timeval accept_timeout = 
    {
        .tv_sec = ACCEPT_TIMEOUT_SEC,
        .tv_usec = ACCEPT_TIMEOUT_USEC,
    };
       
    ret = setsockopt(listening_socket, SOL_SOCKET, SO_RCVTIMEO, &accept_timeout, sizeof(accept_timeout));
    CHECKR(ret == -1, "setsockopt");

    struct sockaddr_in listening_addr =
    {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };

    ret = bind(listening_socket, (struct sockaddr *) &listening_addr, sizeof(listening_addr));
    CHECKR(ret == -1, "bind");

    ret = listen(listening_socket, BACK_LOG);
    CHECKR(ret == -1, "listen");

    int broadcast_socket = socket(PF_INET, SOCK_DGRAM, 0);
    CHECKR(broadcast_socket== -1, "");

    opt = 1;
    ret = setsockopt(broadcast_socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    CHECKR(ret == -1, "setsockopt");

    struct sockaddr_in broadcast_addr = 
    {
        .sin_family = AF_INET,
        .sin_port = htons(CLIENT_PORT),
        .sin_addr.s_addr = INADDR_BROADCAST,
    };

    printf("Sending broadcast...\n");

    ret = sendto(broadcast_socket, BROADCAST_MSG, sizeof(BROADCAST_MSG), 0,
                     (struct sockaddr *) &broadcast_addr, sizeof(broadcast_addr));
    CHECKR(ret == -1, "sendto");

    close(broadcast_socket);

    printf("Accepting clients...\n");

    int nclients = 0;
    struct Client {
        int socket;
        size_t nthreads;
    } client_list[NCLIENTS_MAX];
    struct timeval no_timeout = 
    {
        .tv_sec = 0,
        .tv_usec = 0,
    };

    for (; nclients < NCLIENTS_MAX; nclients++) 
    {
        client_list[nclients].socket = accept(listening_socket, NULL, NULL);
        if (client_list[nclients].socket != -1) 
        {
            ret = setsockopt(client_list[nclients].socket, SOL_SOCKET, SO_RCVTIMEO,
                             &no_timeout, sizeof(no_timeout));
            CHECKR(ret == -1, "setsockopt");

            opt = 1;
            ret = setsockopt(client_list[nclients].socket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
            CHECKR(ret == -1, "setsockopt");

            int ka_probs = TCP_KEEP_ALIVE_PROBES;
            ret = setsockopt(client_list[nclients].socket, IPPROTO_TCP, TCP_KEEPCNT, &ka_probs, sizeof(ka_probs));
            CHECKR(ret == -1, "setsockopt");

            int ka_time = TCP_KEEP_ALIVE_TIME;
            ret = setsockopt(client_list[nclients].socket, IPPROTO_TCP, TCP_KEEPIDLE, &ka_time, sizeof(ka_time));
            CHECKR(ret == -1, "setsockopt");

            int ka_intvl = TCP_KEEP_ALIVE_INTVL;
            ret = setsockopt(client_list[nclients].socket, IPPROTO_TCP, TCP_KEEPINTVL, &ka_intvl, sizeof(ka_intvl));
            CHECKR(ret == -1, "setsockopt");
        }
        else
        {
            if (errno == EAGAIN)
                break;

            CHECKR(1, "accept");
        }
    }

    if (nclients == 0)
    {
        printf("No response to broadcast\n");
        exit(EXIT_FAILURE);
    }
    else if (nclients == NCLIENTS_MAX)
    {
        printf("To many answers\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("%d answers are received\n", nclients);

    char msg[2 * MSGSIZE];
    size_t total_nthreads = 0;
    for (int i = 0; i < nclients; ++i) 
    {
        ret = recv(client_list[i].socket, msg, sizeof(msg), 0);
        CHECKR(ret != sizeof(msg), "recv");
        sscanf(msg, "%lu\n", &client_list[i].nthreads);
        printf("Client %d has %lu threads\n", i, client_list[i].nthreads);
        total_nthreads += client_list[i].nthreads;
    }

    printf("Total number of threads is %lu\n", total_nthreads);

    double step = (RANGE_END - RANGE_START) / total_nthreads;
    double start = RANGE_START;
    for (int i = 0; i < nclients; i++)
    {
        double end = start + step * client_list[i].nthreads;
        snprintf(msg, MSGSIZE, "%f", start);
        snprintf(msg + MSGSIZE, MSGSIZE, "%f", end);

        ret = send(client_list[i].socket, msg, sizeof(msg), MSG_NOSIGNAL);
        CHECKR(ret != sizeof(msg), "send");

        start = end;
    }
      
    double result = 0.0;
    for (int i = 0; i < nclients; i++)
    {
        ret = recv(client_list[i].socket, msg, sizeof(msg), 0);
        CHECKR(ret != sizeof(msg), "recv");

        errno = 0;
        result += strtod(msg, NULL);
        CHECKR(errno, "strtod");
    }
      
    printf("Computing is completed: %lg\n", result);

    for (int i = 0; i < nclients; i++)
        close(client_list[i].socket);

    close(listening_socket);
    
    exit(EXIT_SUCCESS);
}
