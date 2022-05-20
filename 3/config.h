#ifndef CONFIG_H_INCUDED
#define CONFIG_H_INCUDED

#define SERVER_PORT 4789
#define CLIENT_PORT 4790

#define TCP_KEEP_ALIVE_PROBES 2
#define TCP_KEEP_ALIVE_TIME   10
#define TCP_KEEP_ALIVE_INTVL  1

#define ACCEPT_TIMEOUT_SEC 2
#define ACCEPT_TIMEOUT_USEC 0

#define NCLIENTS_MAX 256

#define BACK_LOG 255

#define BROADCAST_MSG "MAGIC"

#define MSGSIZE 0x10

#include <math.h>

#define FUNC(x) ((x) * (x))
#define DX 1e-6
#define RANGE_START 0
#define RANGE_END 100.0

#endif // CONFIG_H_INCUDED
