#include "msp.h"

#include <stdio.h>
#include <stdlib.h>

#include "../udp/udp.h"
#include <ztimer.h>

#define IPV6_MULTICAST_ADDR "FF02::1"

#define MESSAGE_BUFFER_SIZE 64
#define HEADER_SIZE 3
#define PACKET_SIZE HEADER_SIZE + MESSAGE_BUFFER_SIZE + 1

#define THREAD_STACKSIZE 512

void (*pathSizeCallback)(int, int);
void (*pathCallback)(Commands*, int);
void (*handshakeCallback)(int);
void (*carsAnnouncementCallback)(int);
void (*goCallback)(void);
void (*idConflictCallback)(int, int);

uint8_t senderId = 0;
uint8_t messageId = 0;

uint8_t watchedMessageId = 0;

char packetBuffer[PACKET_SIZE];
char messageBuffer[MESSAGE_BUFFER_SIZE];

bool messageConfirmations[16] = {false};

uint8_t neededConfirmations = 0;

// Continuous Handhsake thread
bool continuousHandshakeActivated = false;
char handshakeStack[THREAD_STACKSIZE];

void convertMessageToPath(char* message, int messageSize, Commands* returnPath)
{
    for (int i = 0; i < messageSize; i++)
    {
        returnPath[i] = message[i] - '0';
    }
}

bool waitForConfirmation(int messageId, int numberOfConfirmations, int timeout)
{
    watchedMessageId = messageId;
    
    for (uint8_t i = 0; i < 16; i++)
    {
        messageConfirmations[i] = false;
    }

    for (uint8_t i = 0; i < 100; i++)
    {
        ztimer_sleep(ZTIMER_MSEC, timeout / 100);

        // count how many confirmations we have
        int confirmations = 0;
        for (uint8_t j = 0; j < 16; j++)
        {
            if (messageConfirmations[j])
            {
                confirmations++;
            }
        }
        // printf("Received %d confirmations so far\n", confirmations);

        if (confirmations >= numberOfConfirmations)
        {
            return true;
        }
    }

    return false;
}

void createPacket(char messageType, int msgSenderId, int msgId, int messageSize)
{
    // INITIAL BYTE:    0 | 1           | 3
    // CONTENT:         T | S_ID MSG_ID | MESSAGE

    // Set the message type
    packetBuffer[0] = messageType;

    // Set the message id
    packetBuffer[1] = msgSenderId + '0';
    packetBuffer[2] = msgId + '0';

    for (int i = 0; i < messageSize; i++)
    {
        packetBuffer[i + HEADER_SIZE] = messageBuffer[i];
    }

    packetBuffer[HEADER_SIZE + messageSize] = '\0'; // Null-terminate the string

#ifdef DEBUG
    printf("Created packet: ");
    for (int i = 0; i < PACKET_SIZE; i++)
    {
        if (packetBuffer[i] == '\0')
        {
            break;
        }

        printf("%c", packetBuffer[i]);
    }
    printf("\n");
#endif
    send_udp_message(packetBuffer, IPV6_MULTICAST_ADDR);
}

bool createNewPacket(char messageType, int messageSize)
{
    uint8_t newMessageId = ++messageId;

    bool pathSendingSuccess = false;
    for (uint8_t i = 0; i < 3; i++)
    {
        createPacket(messageType, senderId, newMessageId, messageSize);

        if (neededConfirmations == 0)
        {
            return true;
        }

        pathSendingSuccess = waitForConfirmation(newMessageId, neededConfirmations, 3000);
        
        if (pathSendingSuccess)
        {
            return true;
        }
    }

    printf("Packet failed to send\n");
    return false;
}

void decodePacket(char* packet, int packetSize, char* messageType, 
        uint8_t* msgSenderId, uint8_t* msgId, char** message, int* messageSize)
{
    *messageType = packet[0];
    *msgSenderId = packet[1] - '0';
    *msgId = packet[2] - '0';

    *messageSize = packetSize - HEADER_SIZE;
    *message = (char*)malloc(*messageSize + 1);
    for (int i = HEADER_SIZE; i < packetSize; i++)
    {
        (*message)[i - HEADER_SIZE] = packet[i];
    }
    (*message)[*messageSize] = '\0';

#ifdef DEBUG
    printf("Decoded packet: type %c | from %d | message #%d | content size: %d | content: %s\n", *messageType, *msgSenderId, *msgId, *messageSize, *message);
#endif
}

void handleReceivedMessage(void* pkt, int pktSize)
{
    char* packet = (char*)pkt;
#ifdef DEBUG
    printf("Received packet: ");
    for (int i = 0; i < pktSize; i++)
    {
        printf("%c", packet[i]);
    }
    printf("\n");
#endif

    char messageType;
    uint8_t rcvdMsgSenderId;
    uint8_t rcvdMsgId;
    char *message = NULL;
    int messageSize;

    decodePacket(packet, pktSize, &messageType, &rcvdMsgSenderId, 
        &rcvdMsgId, &message, &messageSize);

    if (messageType == 'P' || messageType == 'S')
    {
        createPacket('K', rcvdMsgSenderId, rcvdMsgId, 0);
    }

    switch (messageType)
    {
    case 'S':
    {
        pathSizeCallback(message[0] - '0', rcvdMsgSenderId);
        break;
    }
    case 'P':
    {
        Commands receivedPath[messageSize]; 
        convertMessageToPath(message, messageSize, receivedPath);
        pathCallback(receivedPath, messageSize);
        break;
    }
    case 'H':
    {
        handshakeCallback(message[0] - '0');
        break;
    }
    case 'R':
    {
        carsAnnouncementCallback(message[0] - '0');
        break;
    }
    case 'G':
    {
        goCallback();
        break;
    }
    case 'C':
    {
        idConflictCallback(message[0] - '0', message[1] - '0');
        break;
    }
    case 'K':
    {
        if (rcvdMsgId == watchedMessageId)
        {
            messageConfirmations[rcvdMsgSenderId] = true;
        }
        break;
    }
    default:
    {
        break;
    }
    }

    free(message);
}

/****************************** PUBLIC FUNCTIONS ******************************/

void start_msp_server(void)
{
    start_udp_server();
    register_received_message_callback(handleReceivedMessage);
}

void setAssureSending(uint8_t desiredConfirmations)
{
    neededConfirmations = desiredConfirmations;
}

bool sendPathSize(int size)
{
    messageBuffer[0] = size + '0';

    return createNewPacket('S', 1);
}

bool sendPath(uint8_t* path, int pathSize)
{
    for (uint8_t i = 0; i < pathSize; i++)
    {
        messageBuffer[i] = path[i] + '0';
    }

    return createNewPacket('P', pathSize);
}

bool announceCars(int numCars)
{
    messageBuffer[0] = numCars + '0';

    return createNewPacket('R', 1);
}

bool logCommand(Commands command)
{
    messageBuffer[0] = 'C';
    messageBuffer[1] = command + '0';

    return createNewPacket('L', 2);
}

bool logPath(uint8_t *path, int pathSize)
{
    messageBuffer[0] = 'P';
    for (uint8_t i = 0; i < pathSize; i++)
    {
        messageBuffer[i + 1] = path[i] + '0';
    }

    return createNewPacket('L', pathSize + 1);
}

bool handshake(int id)
{
    senderId = id;

    messageBuffer[0] = id + '0';

    return createNewPacket('H', 1);
}

bool solveIdConflict(int oldId, int newId)
{
    senderId = newId;

    messageBuffer[0] = oldId + '0';
    messageBuffer[1] = newId + '0'; 

    return createNewPacket('C', 2);
}

bool sendGo(void)
{
    createNewPacket('G', 0);
    createNewPacket('G', 0);
    createNewPacket('G', 0);
    createNewPacket('G', 0);
    createNewPacket('G', 0);

    return true;
}

void register_receive_path_size_callback(void (*path_size_cb)(int, int))
{
    pathSizeCallback = path_size_cb;
}

void register_receive_path_callback(void (*path_cb)(Commands*, int))
{
    pathCallback = path_cb;
}

void register_handshake_callback(void (*handshake_cb)(int))
{
    handshakeCallback = handshake_cb;
}

void register_cars_announcement_callback(void (*carsAnnouncement_cb)(int))
{
    carsAnnouncementCallback = carsAnnouncement_cb;
}

void register_go_callback(void (*go_cb)(void))
{
    goCallback = go_cb;
}

void register_id_conflict_callback(void (*id_conflict_cb)(int, int))
{
    idConflictCallback = id_conflict_cb;
}
