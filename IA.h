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

void IARecv(unsigned char* payload);
unsigned char* IAUpdate();

NodeStack* nodes;
Node* player;

#endif