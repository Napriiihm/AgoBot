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

#include <libwebsockets.h>

#include "Utils.h"

void IAInit();
char* IAStep(unsigned char* payload);

NodeStack* nodes;
Player* player;
Map* map;

#endif