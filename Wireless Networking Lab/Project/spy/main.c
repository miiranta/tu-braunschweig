#include <stdio.h>
#include "net/sock/udp.h"
#include "net/ipv6/addr.h"

#define SERVER_BUFFER_SIZE      (32)
#define SERVER_MSG_QUEUE_SIZE   (8)

static char server_buffer[SERVER_BUFFER_SIZE];
static msg_t server_msg_queue[SERVER_MSG_QUEUE_SIZE];

const int port = 4242;

int main(void)
{
    sock_udp_t sock;
    sock_udp_ep_t server = { .port = port, .family = AF_INET6 };
    msg_init_queue(server_msg_queue, SERVER_MSG_QUEUE_SIZE);

    if (sock_udp_create(&sock, &server, NULL, 0) < 0)
    {
        return -1;
    }

    printf("Success! Started UDP server on port %u\n", server.port);

    while (1) 
    {
        int res = sock_udp_recv(&sock, server_buffer, sizeof(server_buffer) - 1, SOCK_NO_TIMEOUT, NULL);

        if (res > 0)
        {
            server_buffer[res] = '\0';
            printf("RECEIVED PACKET: %s\n", server_buffer);
        }
    }

    return 0;
}
