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
#define MAIN_QUEUE_SIZE 1
#define MAX_MSG_CHAR_SIZE 2
#define BROADCAST_ADDR "ff02::1"
#define NET_CODE 22
#define PORT 1222

char rec_thread_stack[1024];
char sen_thread_stack[THREAD_STACKSIZE_MAIN];
kernel_pid_t rec_thread_pid = KERNEL_PID_UNDEF;
kernel_pid_t sen_thread_pid = KERNEL_PID_UNDEF;

kernel_pid_t lowpan_pid;

ipv6_addr_t broadcast_addr;
ipv6_addr_t my_addr[2]; // 2 interfaces, 2 addresses
char my_addr_str[IPV6_ADDR_MAX_STR_LEN]; // from the second interface

void main_loop(void);
void *sen_thread(void *arg);
static void start_sending(void);
static void send(ipv6_addr_t* addr, int port, char data[MAX_MSG_CHAR_SIZE]);
void parse_ipv6(char* addr_str, ipv6_addr_t* addr);

int main(void)
{
    
    // Wait for pyterm
    ztimer_sleep(ZTIMER_MSEC, 1000);
    if(ENABLE_DEBUG){printf("Running.\n");}
    
    // Print my IPv6 address
    // From the second interface
    netifs_get_ipv6(my_addr, 2);

    ipv6_addr_to_str(my_addr_str, &my_addr[1], IPV6_ADDR_MAX_STR_LEN);
    if(ENABLE_DEBUG){printf("My IP: %s\n", my_addr_str);}

    // Parse broadcast IPv6
    parse_ipv6(BROADCAST_ADDR, &broadcast_addr);

    // Create SEND Thread
    start_sending();

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

void *sen_thread(void *arg)
{
    (void)arg;
    char data[MAX_MSG_CHAR_SIZE] = "";

    while (1) 
    { 
        //Empty data
        strcpy(data, "b");
        //strcat(data, my_addr_str);
        
        send(&broadcast_addr, PORT, data);
        ztimer_sleep(ZTIMER_MSEC, 2000); 
    }

    return NULL;
}

static void start_sending(void)
{
    sen_thread_pid = thread_create(sen_thread_stack, sizeof(sen_thread_stack), THREAD_PRIORITY_MAIN + 1, THREAD_CREATE_STACKTEST, sen_thread, NULL, "sen_thread");
}

static void send(ipv6_addr_t* addr, int port, char data_sen[MAX_MSG_CHAR_SIZE])
{
    //char data[MAX_MSG_CHAR_SIZE]
    //That looks kinda inneficient, but thats the price to pay
    //strcat(data, my_addr_str);
    
    //uint16_t data = 1020; /* example data */

    /* gnrc_pktsnip_t: each snip is a different layer */
    gnrc_pktsnip_t *payload, *udp, *ip;

    /* allocate payload, udp-header, ip-header*/
    payload = gnrc_pktbuf_add(NULL, data_sen, sizeof(char) * MAX_MSG_CHAR_SIZE, GNRC_NETTYPE_UNDEF);
    udp = gnrc_udp_hdr_build(payload, port, port);
    ip = gnrc_ipv6_hdr_build(udp, NULL, addr);

    /* add netif header, set process-id of network interface*/
    gnrc_pktsnip_t *netif = gnrc_netif_hdr_build(NULL, 0, NULL, 0);
    ((gnrc_netif_hdr_t *)netif->data)->if_pid = (kernel_pid_t)gnrc_netif_iter(NULL)->pid;

    /* prepend ip packet with netif header */
    LL_PREPEND(ip, netif);

    /* dispatch packet through network stack = send */
    gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip);

    // Print
    if(ENABLE_DEBUG)
    {
        // addr to string
        char addr_str[IPV6_ADDR_MAX_STR_LEN];
        ipv6_addr_to_str(addr_str, addr, IPV6_ADDR_MAX_STR_LEN);

        printf("SENT: %s to %s:%d\n", data_sen, addr_str, port);
    }
}

void parse_ipv6(char* addr_str, ipv6_addr_t* addr)
{
    if (ipv6_addr_from_str(addr, addr_str) == NULL) 
    {
       puts("Error: unable to parse destination address.");
    }
}

