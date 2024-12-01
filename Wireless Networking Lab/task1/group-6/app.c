#include <stdio.h>
#include <stdlib.h>
#include "ztimer.h"
#include "shell.h"
#include "msg.h"
#include "board.h"
#include "periph/gpio.h"
#include "net/gnrc.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/gnrc/udp.h"
#include "net/ipv6/addr.h"
#include "net/netif.h"
#include "sht2x.h"
#include "sht2x_params.h"

#define ENABLE_DEBUG (0)
#define MAIN_QUEUE_SIZE 2
#define MAX_MSG_CHAR_SIZE 32
#define BROADCAST_ADDR "ff02::1"
#define NET_CODE "12"
#define PORT 1222

#define MAX_TEMP 2500 //2500 = 25.00 C

char rec_thread_stack[512];
char sen_thread_stack[THREAD_STACKSIZE_MAIN];
char sen_thread_broad_stack[THREAD_STACKSIZE_MAIN];
kernel_pid_t rec_thread_pid = KERNEL_PID_UNDEF;
kernel_pid_t sen_thread_pid = KERNEL_PID_UNDEF;
kernel_pid_t sen_thread_broad_pid = KERNEL_PID_UNDEF;

ipv6_addr_t broadcast_addr;
ipv6_addr_t my_addr[2]; // 2 interfaces, 2 addresses
char my_addr_str[IPV6_ADDR_MAX_STR_LEN]; // from the second interface

static volatile uint32_t count;

static sht2x_t sht21;

bool alarm_on = 0;

void main_loop(void);
void *rec_thread(void *arg);
void *sen_thread_broad(void *arg);
static void start_receiving(void);
static void start_sending(void);
static void send(ipv6_addr_t* addr, int port, char data[MAX_MSG_CHAR_SIZE]);
void parse_ipv6(char* addr_str, ipv6_addr_t* addr);
void parse_msg(char* msg);
void ledsBinary(int num);
void ledsAlarm(void);
void buttonCallback(void *arg);
void sendTemperature(void);
char* itoa(int val, int base);

int main(void)
{
    
    // Wait for pyterm
    ztimer_sleep(ZTIMER_MSEC, 1000);
    printf("Running.\n");
    
    // Print my IPv6 address
    // From the second interface
    netifs_get_ipv6(my_addr, 2);

    ipv6_addr_to_str(my_addr_str, &my_addr[1], IPV6_ADDR_MAX_STR_LEN);
    if(ENABLE_DEBUG){printf("My IP: %s\n", my_addr_str);}

    // Parse broadcast IPv6
    parse_ipv6(BROADCAST_ADDR, &broadcast_addr);

    // Create RECEIVE Thread
    start_receiving();

    // Create SEND Thread
    start_sending();

    //Button listener
    //GPIO_RISING for release activation
    //GPIO_FALLING for press activation
    gpio_init_int(BTN0_PIN, BTN0_MODE, GPIO_FALLING, buttonCallback, (void *)&count);

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

    //Alarm
    ledsAlarm();

    //Shell
    //char line_buf[SHELL_DEFAULT_BUFSIZE];
    //shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

}

void *rec_thread(void *arg)
{

    (void)arg;

    msg_t rec_msg_queue[MAIN_QUEUE_SIZE];
    msg_init_queue(rec_msg_queue, MAIN_QUEUE_SIZE);
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
                if(ENABLE_DEBUG){printf("REC: %s\n", (char*)(payload_snip->data));}

                //Parse and process
                parse_msg((char*)(payload_snip->data));

                gnrc_pktbuf_release(msg.content.ptr);
                break;
            }
            default:
            {
                /* msg.content.ptr points to upmost layer */
                /* -> search for payload snip (=GNRC_NETTYPE_UNDEF)*/
                gnrc_pktsnip_t* payload_snip = gnrc_pktsnip_search_type(msg.content.ptr, GNRC_NETTYPE_UNDEF);

                // Print received message, IP and Port
                if(ENABLE_DEBUG){printf("REC: %s\n", (char*)(payload_snip->data));}

                gnrc_pktbuf_release(msg.content.ptr);
                break;
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
        sendTemperature();
        ztimer_sleep(ZTIMER_MSEC, 5000); 
    }

    return NULL;
}

void *sen_thread_broad(void *arg)
{
    //Broadcast IP every few seconds

    (void)arg;

    while (1) 
    { 
        ztimer_sleep(ZTIMER_MSEC, 10000); 
    }

    return NULL;
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
    server.demux_ctx = (uint32_t)PORT;
    gnrc_netreg_register(GNRC_NETTYPE_UDP, &server);
}

static void start_sending(void)
{
    //Broadcast
    sen_thread_broad_pid = thread_create(sen_thread_broad_stack, sizeof(sen_thread_broad_stack), THREAD_PRIORITY_MAIN + 1, THREAD_CREATE_STACKTEST, sen_thread_broad, NULL, "sen_thread_broad");

    //
    sen_thread_pid = thread_create(sen_thread_stack, sizeof(sen_thread_stack), THREAD_PRIORITY_MAIN + 1, THREAD_CREATE_STACKTEST, sen_thread, NULL, "sen_thread");

}

static void send(ipv6_addr_t* addr, int port, char data_sen[MAX_MSG_CHAR_SIZE])
{
    //char data[MAX_MSG_CHAR_SIZE]
    //That looks kinda inneficient, but thats the price to pay
    
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
    if(ENABLE_DEBUG){printf("SENT: %s\n", data_sen);}
}

void parse_ipv6(char* addr_str, ipv6_addr_t* addr)
{
    if (ipv6_addr_from_str(addr, addr_str) == NULL) 
    {
       puts("Error: unable to parse destination address.");
    }
}

void parse_msg(char* msg)
{

    int msg_type = 0;

    //Should be processed?
    switch(msg[0]){
        case 'A':
        {
            msg_type = 1;
            break;
        }
        case 'B':
        {
            msg_type = 2;
            break; 
        }
        case 'C':
        {
            msg_type = 3;
            break; 
        }
        case 'D':
        {
            msg_type = 4;
            break; 
        }
        default:
        {
            if(ENABLE_DEBUG){printf("Msg ignored (OP_CODE)\n");}
            return;
        }
            
    }

    //Verify net_code
    //If it`s not the same, ignore
    char net_code[2] = "";
    net_code[0] = msg[2];
    net_code[1] = msg[3];
    if(strcmp(net_code, NET_CODE) != 0){
        if(ENABLE_DEBUG){printf("Msg ignored (NET_CODE)\n");}
        return;
    }

    //Broadcast case
    if(msg_type == 1){

    }

    //Click case (LedsBin)
    if(msg_type == 2){
        
        char num[2] = {msg[5], msg[6]};
        printf("Changing leds to %s\n", num);

        //Convert num to int
        int num_int = atoi(num);

        //Change leds
        ledsBinary(num_int);

    }

    //Temperature case
    if(msg_type == 3){

        char temp[5] = {msg[5], msg[6], msg[7], msg[8], msg[9]};

        //Convert to int
        int temp_int = atoi(temp);

        //Print
        printf("Temperature received:  %02d.%02d °C\n", temp_int / 100, temp_int % 100);

        //Warning
        if(temp_int > MAX_TEMP){
            printf("\n-------------------------");
            printf("\nWARNING - TEMPERATURE RECEIVED TOO HIGH!\n");
            printf("-------------------------\n\n");

            alarm_on = 1;
        }
        
    }

    //Click case (Alarm)
    if(msg_type == 4){
        
        alarm_on = 0;

    }

}

void ledsBinary(int num)
{

    //Alarm is ON > Return
    if(alarm_on){return;}

    //Max of 4 bits (0 to 15)
    if((num > 15) || (num < 0)){
        num = 0;
    }

    //Int to binary
    int binaryNum[4] = {0,0,0,0};
    int i = 0;
    while (num > 0) {
        binaryNum[i] = num % 2;
        num = num / 2;
        i++;
    }

    // LEDs initialized in /board/ibr-node/board.c
    // LED0 = D1, LED1 = D2, LED2 = D3, LED3 = D4,
    if(binaryNum[0]){LED0_ON;}else{LED0_OFF;}
    if(binaryNum[1]){LED1_ON;}else{LED1_OFF;}
    if(binaryNum[2]){LED2_ON;}else{LED2_OFF;}
    if(binaryNum[3]){LED3_ON;}else{LED3_OFF;}

    //Should be 2, but 4 is nicer! :)
  
}

void ledsAlarm(void)
{

    //Alarm is OFF > Return
    if(!(alarm_on)){return;}

    LED0_ON;
    LED1_ON;
    LED2_ON;
    LED3_ON;

    ztimer_sleep(ZTIMER_MSEC, 500);

    LED0_OFF;
    LED1_OFF;
    LED2_OFF;
    LED3_OFF;

    ztimer_sleep(ZTIMER_MSEC, 500);

}

void buttonCallback(void *arg)
{

    //ALARM
    if(alarm_on){

        //Send button press
        char data[MAX_MSG_CHAR_SIZE] = "";
        strcpy(data, "D"); //Opcode
        strcat(data, " ");
        strcat(data, NET_CODE); //Netcode
        send(&broadcast_addr, PORT, data);

        //Disable alarm
        alarm_on = 0;

        return;
    }

    //BINARY LEDS
    ++(*((uint32_t*) arg));
    int comp_var = (int) *((uint32_t*) arg); //Needs to be an int so you can compare with 0

    //Max of 4 bits (0 to 15)
    if((comp_var > 15) || (comp_var < 0)){
        *((uint32_t*) arg) = 0;
        printf("Counter reset! (max is 15)\n");
    }

    printf("Button pressed %ld times\n", *((uint32_t*) arg));

    //Convert to string and make it 2 digits always
    char str[2];
    strcpy(str, itoa((int) *((uint32_t*) arg), 10));
    if(strlen(str) == 0){
        str[0] = '0';
        str[1] = '0';
    }
    if(strlen(str) == 1){
        str[1] = str[0];
        str[0] = '0';
    }

    //Send button press
    char data[MAX_MSG_CHAR_SIZE] = "";
    strcpy(data, "B"); //Opcode
    strcat(data, " ");
    strcat(data, NET_CODE); //Netcode
    strcat(data, " ");
    strcat(data, str);
    send(&broadcast_addr, PORT, data);

}

void sendTemperature(void)
{

    //Read temperature
    SHT21_ON;

    ztimer_sleep(ZTIMER_MSEC, 10); 
    if (sht2x_init(&sht21, sht2x_params) != 0) {
        printf("Could not initialize SHT21\n");
        return;
    }

    int16_t temp = INT16_MIN;

    temp = sht2x_read_temperature(&sht21);
    if (temp == INT16_MIN) {
        printf("Read from SHT21 failed\n");
        return;
    }

    SHT21_OFF;

    //Print
    printf("My temperature:  %02d.%02d °C\n", temp / 100, temp % 100);

    //Alarm On?
    if(temp > MAX_TEMP){
        alarm_on = 1;
    }

    //Parse temp (00.00)
    char str[5];
    strcpy(str, itoa((int) temp, 10));
    if(strlen(str) == 0){
        str[0] = '0';
        str[1] = '0';
        str[2] = '0';
        str[3] = '0';
    }
    if(strlen(str) == 1){
        str[3] = str[0];
        str[2] = '0';
        str[1] = '0';
        str[0] = '0';
    }
    if(strlen(str) == 2){
        str[3] = str[1];
        str[2] = str[0];
        str[1] = '0';
        str[0] = '0';
    }
    if(strlen(str) == 3){
        str[3] = str[2];
        str[2] = str[1];
        str[1] = str[0];
        str[0] = '0';
    }

    //Send temperature
    char data[MAX_MSG_CHAR_SIZE] = "";
    strcpy(data, "C"); //Opcode
    strcat(data, " ");
    strcat(data, NET_CODE); //Netcode
    strcat(data, " ");
    strcat(data, str);
    send(&broadcast_addr, PORT, data);

}

// By Robert Jan Schaper (http://www.strudel.org.uk/itoa/)
char* itoa(int val, int base)
{
	
	static char buf[32] = {0};
	
	int i = 30;
	
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
}

