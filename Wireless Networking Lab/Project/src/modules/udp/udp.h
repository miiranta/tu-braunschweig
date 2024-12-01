#ifndef UDP_H
#define UDP_H

void start_udp_server(void);
void register_received_message_callback(void (*msg_cb)(void*, int));
void send_udp_message(char* message, char* dest);

#endif /* UDP_H */
