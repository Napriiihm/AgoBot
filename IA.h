#ifndef IA_H
#define IA_H

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>

#include "Angle.h"
#include "WS.h"
#include "UI.h"

#define SPLIT_DISTANCE 710
#define DANGER_DISTANCE 50
#define FOOD_VALUE 10

#define WALL_VALUE 100
#define WALL_ESCAPE_DISTANCE 200
#define DEFAULT_MARGE 200
#define AVOID_VIRUS_DISTANCE 15

const char* BotName;

void IARecv(unsigned char* payload, int* exit);
void IAUpdate(struct lws *wsi);
void IAV2(struct lws* wsi);
void IAV3(struct lws* wsi);
void IAInit(const char* name);
char* getName();

NodeStack* nodes;
NodeStack* playerNodes;
Node* player;

unsigned int newPlayerNodeId;

unsigned int playerID;
unsigned int player_length;
unsigned int playerTotalSize;

int split_timer;

#endif