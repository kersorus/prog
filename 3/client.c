#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "integral.h"
#include "config.h"
#include "debug.h"

int main(int argc, char *argv[])
{
    if (argc != 2)         
    {
        fprintf(stderr, "Usage: %s <number-of-threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    errno = 0;
    char *endptr = NULL;
    size_t nthreads  = strtoull(argv[1], &endptr, 10);
    if (errno != 0 || *endptr)
    {
        fprintf(stderr, "Usage: %s <number-of-threads>\n", argv[0]);
        exit(EXIT_FAILURE);  
    }

    while(1)
    {
        int broadcast_socket = socket(PF_INET, SOCK_DGRAM, 0);
        CHECKR(broadcast_socket == -1, "socket");

        int opt = 1;
        int ret = setsockopt(broadcast_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        CHECKR(ret == -1, "setsockopt");

        struct sockaddr_in broadcast_addr = 
        {
            .sin_family = AF_INET,
            .sin_port = htons(CLIENT_PORT),
            .sin_addr.s_addr = INADDR_ANY,
        };

        ret = bind(broadcast_socket, (struct sockaddr *) &broadcast_addr, sizeof(broadcast_addr));
        CHECKR(ret == -1, "bind");

        printf("Waiting for broadcast from server...\n");

        char broadcast_msg[sizeof(BROADCAST_MSG)] = {};
        struct sockaddr_in server_addr = {};
        socklen_t server_addr_size = sizeof(server_addr);
        ret = recvfrom(broadcast_socket, broadcast_msg, sizeof(BROADCAST_MSG), 0,
                       (struct sockaddr *) &server_addr, &server_addr_size);
        CHECKR(ret == -1, "recvfrom");

        close(broadcast_socket);

        ret = strncmp(BROADCAST_MSG, broadcast_msg, sizeof(BROADCAST_MSG));
        if (ret != 0)
        {
            fprintf(stderr, "Bad broadcast message: %s\n", broadcast_msg);
            exit(EXIT_FAILURE);
        }

        int connected_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        CHECKR(connected_socket == -1, "socket");

        server_addr.sin_port = htons(SERVER_PORT);
        ret = connect(connected_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));
        CHECKR(ret == -1, "connect");

        printf("Successfully connected to server\n");

        char msg[MSGSIZE * 2];
        snprintf(msg, sizeof(msg), "%lu", nthreads);
        ret = send(connected_socket, msg, sizeof(msg), MSG_NOSIGNAL);
        CHECKR(ret != sizeof(msg), "send");

        ret = recv(connected_socket, msg, sizeof(msg), 0);
        CHECKR(ret != sizeof(msg), "recv");

        errno = 0;
        double range_start = strtod(msg, NULL);
        double range_end  = strtod(msg + 0x10, NULL);
        CHECKR(errno, "strtod");
        printf("Range is received: [%lg, %lg]\n", range_start, range_end);

        double result = 0.0;
        compute_integral(range_start, range_end, nthreads, &result);
        printf("Result is computed: %lg\n", result);

        snprintf(msg, sizeof(msg), "%lg", result);
        ret = send(connected_socket, msg, sizeof(msg), MSG_NOSIGNAL);
        CHECKR(ret != sizeof(msg), "send");

        close(connected_socket);
    }
}
