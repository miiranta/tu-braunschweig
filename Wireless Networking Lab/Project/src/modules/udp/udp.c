#include "udp.h"

#include <stdio.h>

#include "net/sock/udp.h"
#include "net/ipv6/addr.h"

#define SERVER_STACK_SIZE       (2048)
#define SERVER_BUFFER_SIZE      (32)
#define SERVER_MSG_QUEUE_SIZE   (8)

static char server_stack[SERVER_STACK_SIZE];
static char server_buffer[SERVER_BUFFER_SIZE];
static msg_t server_msg_queue[SERVER_MSG_QUEUE_SIZE];

const int port = 4242;

typedef void (*udp_cb)(void*, int);
udp_cb udp_message_callback;

void *_udp_server(void* args)
{
    (void)args;
    
    sock_udp_t sock;
    sock_udp_ep_t server = { .port = port, .family = AF_INET6 };
    msg_init_queue(server_msg_queue, SERVER_MSG_QUEUE_SIZE);

    if (sock_udp_create(&sock, &server, NULL, 0) < 0)
    {
        return NULL;
    }

    printf("Success! Started UDP server on port %u\n", server.port);

    while (1) 
    {
        int res = sock_udp_recv(&sock, server_buffer, sizeof(server_buffer) - 1, SOCK_NO_TIMEOUT, NULL);

        if (res < 0) 
        {
            puts("Error while receiving");
        } 
        else if (res == 0) 
        {
            puts("No data received");
        }
        else
        {
            server_buffer[res] = '\0';
            udp_message_callback((void*)server_buffer, res);
        }
    }

    return NULL;
}

void start_udp_server(void)
{
    thread_create(
        server_stack, 
        sizeof(server_stack),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        _udp_server,
        NULL,
        "UDP Server"
    );
}

void register_received_message_callback(void (*msg_cb)(void*, int))
{
    udp_message_callback = msg_cb;
}

void send_udp_message(char* message, char* addr)
{
    int res;
    sock_udp_ep_t remote = { .family = AF_INET6 };

    ipv6_addr_from_str((ipv6_addr_t *)&remote.addr, addr);

    if (ipv6_addr_is_link_local((ipv6_addr_t *)&remote.addr))
    {
        gnrc_netif_t *netif = gnrc_netif_iter(NULL);
        remote.netif = (uint16_t)netif->pid;
    }

    remote.port = port;

    res = sock_udp_send(NULL, message, strlen(message), &remote);
    if (res < 0)
    {
        puts("could not send");
    }
}
