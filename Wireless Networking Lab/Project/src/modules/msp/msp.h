#ifndef MSP_H
#define MSP_H

#include <stdbool.h>
#include <stdint.h>

#include "../enums/commands.h"

// MSP: Maze Solver Protocol

void start_msp_server(void);
void setAssureSending(uint8_t desiredConfirmations);

bool sendPathSize(int size);
bool sendPath(uint8_t* path, int pathSize);
bool announceCars(int numCars);
bool sendGo(void);
bool handshake(int id);
bool solveIdConflict(int oldId, int newId);

bool logCommand(Commands command);
bool logPath(uint8_t* path, int pathSize);

void register_receive_path_size_callback(void (*path_size_cb)(int, int));
void register_receive_path_callback(void (*path_cb)(Commands*, int));
void register_handshake_callback(void (*handshake_cb)(int));
void register_cars_announcement_callback(void (*carsAnnouncement_cb)(int));
void register_go_callback(void (*go_cb)(void));
void register_id_conflict_callback(void (*id_conflict_cb)(int, int));

#endif /* MSP_H */