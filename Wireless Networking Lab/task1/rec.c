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
#define MAX_MSG_CHAR_SIZE 32
#define BROADCAST_ADDR "ff02::1"
#define NET_CODE 22
#define PORT 1222

char udp_thread_stack[2048];
char sen_thread_stack[THREAD_STACKSIZE_MAIN];
kernel_pid_t udp_loop_pid = KERNEL_PID_UNDEF;
kernel_pid_t sen_thread_pid = KERNEL_PID_UNDEF;

ipv6_addr_t broadcast_addr;
ipv6_addr_t my_addr[2]; // 2 interfaces, 2 addresses
char my_addr_str[IPV6_ADDR_MAX_STR_LEN]; // from the second interface

void main_loop(void);
void *rec_thread(void *arg);
static void start_receiving(void);
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

    // Create RECEIVE Thread
    start_receiving();

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

void *_udp_event_loop(void *arg)
{

    //ztimer_sleep(ZTIMER_MSEC, 3000);

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
                if(ENABLE_DEBUG)
                {
                    printf("REC: %s\n", *((char* *)payload_snip->data));
                    gnrc_pktbuf_release(msg.content.ptr);
                }
                break;
            }
            default:
            {
                /* msg.content.ptr points to upmost layer */
                /* -> search for payload snip (=GNRC_NETTYPE_UNDEF)*/
                gnrc_pktsnip_t* payload_snip = gnrc_pktsnip_search_type(msg.content.ptr, GNRC_NETTYPE_UNDEF);

                // Print received message, IP and Port
                if(ENABLE_DEBUG)
                {
                    printf("REC (other): %s\n", *((char* *)payload_snip->data));
                    gnrc_pktbuf_release(msg.content.ptr);
                }
            }
        }
        
    }

    return NULL;
}

static void start_receiving(void)
{
    /* initialize netregister entry struct*/
    gnrc_netreg_entry_t server = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL, KERNEL_PID_UNDEF);

    /* init receive thread */
    udp_loop_pid = thread_create(udp_thread_stack, sizeof(udp_thread_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, _udp_event_loop, NULL, "udp_loop");
    ztimer_sleep(ZTIMER_MSEC, 3000);

    /* start server (which means registering eventloop for the chosen port) */
    server.target.pid = udp_loop_pid;
    
    /* port = z.B. 8000*/
    server.demux_ctx = (uint32_t)PORT;
    gnrc_netreg_register(GNRC_NETTYPE_UDP, &server);
}

void parse_ipv6(char* addr_str, ipv6_addr_t* addr)
{
    if (ipv6_addr_from_str(addr, addr_str) == NULL) 
    {
       puts("Error: unable to parse destination address.");
    }
}

