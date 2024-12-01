#include <stdio.h>
#include "ztimer.h"
#include "shell.h"
#include "msg.h"
#include "net/gnrc.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/gnrc/udp.h"
#include "net/ipv6/addr.h"
#include "net/netif.h"

#define ENABLE_DEBUG (1)
#define DEST_ADDR "fe80::a4bd:cdd8:54bb:e875"
#define OWN_PORT 1222
#define DEST_PORT 1222
#define MAIN_QUEUE_SIZE 4

char rec_thread_stack[1024];
char sen_thread_stack[THREAD_STACKSIZE_MAIN];
kernel_pid_t rec_thread_pid = KERNEL_PID_UNDEF;
kernel_pid_t sen_thread_pid = KERNEL_PID_UNDEF;

kernel_pid_t lowpan_pid;

ipv6_addr_t dest_addr;

void main_loop(void);
void *rec_thread(void *arg);
void *sen_thread(void *arg);
static void send(ipv6_addr_t* addr, int port);
static void start_receiving(void);

int main(void)
{
    
    // Wait for pyterm
    ztimer_sleep(ZTIMER_MSEC, 1000);
    if(ENABLE_DEBUG){printf("Running.\n");}
    
    /* print all IPv6 addresses */
    printf("{\"My IPv6 addresses are\": [\"");
    netifs_print_ipv6("\", \"");
    puts("\"]}");

    // Parse IPv6
    if (ipv6_addr_from_str(&dest_addr, DEST_ADDR) == NULL) 
    {
        puts("Error: unable to parse destination address.");
    }

    // Create RECEIVE Thread
    start_receiving();

    // Create SEND Thread
    sen_thread_pid = thread_create(sen_thread_stack, sizeof(sen_thread_stack), THREAD_PRIORITY_MAIN + 1, THREAD_CREATE_STACKTEST, sen_thread, NULL, "sen_thread");

    // Main loop
    while (1) 
    { 
        main_loop();
        ztimer_sleep(ZTIMER_MSEC, 1000); 
    }

    return 0;

}

void main_loop(void)
{

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

}

void *rec_thread(void *arg)
{

    (void)arg;

    msg_t _udp_msg_queue[MAIN_QUEUE_SIZE];
    msg_init_queue(_udp_msg_queue, MAIN_QUEUE_SIZE);
    msg_t msg;

    while (1)
    { 
        
        msg_receive(&msg);
        
        switch(msg.type)
        {
            /* if message was received through netapi */
            case GNRC_NETAPI_MSG_TYPE_RCV:
            {
                /* msg.content.ptr points to upmost layer */
                /* -> search for payload snip (=GNRC_NETTYPE_UNDEF)*/
                gnrc_pktsnip_t* payload_snip = gnrc_pktsnip_search_type(msg.content.ptr, GNRC_NETTYPE_UNDEF);
                
                // Print received message, IP and Port
                printf("udp_msg_recv: %s\n", *((char* *)payload_snip->data));
                gnrc_pktbuf_release(msg.content.ptr);
                break;
            }
            default:
            {
                gnrc_pktsnip_t* payload_snip = gnrc_pktsnip_search_type(msg.content.ptr, GNRC_NETTYPE_UNDEF);

                // Print received message, IP and Port
                printf("other packet: %s\n", *((char* *)payload_snip->data));
                gnrc_pktbuf_release(msg.content.ptr);
            }
        }
        
    }

    return NULL;
}

void *sen_thread(void *arg)
{
    (void)arg;
    while (1) 
    { 
        send(&dest_addr, DEST_PORT);
        ztimer_sleep(ZTIMER_MSEC, 2000); 
    }

    return NULL;
}

static void send(ipv6_addr_t* addr, int port)
{
    //My second IP address
    ipv6_addr_t my_ip[2];
    netifs_get_ipv6(my_ip, 2);

    char* my_ip_str = (char*)malloc(IPV6_ADDR_MAX_STR_LEN);
    ipv6_addr_to_str(my_ip_str, &my_ip[1], IPV6_ADDR_MAX_STR_LEN);
    if(ENABLE_DEBUG){printf("My IP: %s\n", my_ip_str);}


    //That looks kinda inneficient, but thats the price to pay
    char* data = strcat(my_ip_str, " says hello!");

    //uint16_t data = 1000; /* example data */

    /* gnrc_pktsnip_t: each snip is a different layer */
    gnrc_pktsnip_t *payload, *udp, *ip;

    /* allocate payload, udp-header, ip-header*/
    payload = gnrc_pktbuf_add(NULL, &data, sizeof(data), GNRC_NETTYPE_UNDEF);
    udp = gnrc_udp_hdr_build(payload, port, port);
    ip = gnrc_ipv6_hdr_build(udp, NULL, addr);

    /* add netif header, set process-id of network interface*/
    gnrc_pktsnip_t *netif = gnrc_netif_hdr_build(NULL, 0, NULL, 0);
    ((gnrc_netif_hdr_t *)netif->data)->if_pid = (kernel_pid_t)gnrc_netif_iter(NULL)->pid;

    /* prepend ip packet with netif header */
    LL_PREPEND(ip, netif);

    /* dispatch packet through network stack = send */
    gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip);

    // Print sent message, IP and Port
    printf("udp_msg_sent: %s to %s:%d\n", data, DEST_ADDR, DEST_PORT);
}

static void start_receiving(void)
{
    /* initialize netregister entry struct*/
    gnrc_netreg_entry_t server = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL, KERNEL_PID_UNDEF);

    /* init receive thread */
    rec_thread_pid = thread_create(rec_thread_stack, sizeof(rec_thread_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, rec_thread, NULL, "rec_thread");

    /* start server (which means registering eventloop for the chosen port) */
    server.target.pid = rec_thread_pid;
    
    /* port = z.B. 8000*/
    server.demux_ctx = (uint32_t)OWN_PORT;
    gnrc_netreg_register(GNRC_NETTYPE_UDP, &server);
}



