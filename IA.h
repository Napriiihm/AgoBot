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

void IARecv(unsigned char* payload, int* exit);
void IAUpdate(struct lws *wsi);

NodeStack* nodes;
Node* player;

unsigned int playerID;
unsigned int player_length;

int split_timer;

#endif