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

#include "Utils.h"
#include "WS.h"
#include "UI.h"

const char* BotName;

void IARecv(unsigned char* payload, int* exit);
void IAUpdate(struct lws *wsi);
void IAInit(const char* name);
char* getName();

NodeStack* nodes;
NodeStack* playerNodes;
Node* player;

unsigned int playerID;
unsigned int player_length;
unsigned int playerTotalSize;

int split_timer;

#endif