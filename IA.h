#ifndef IA_H
#define IA_H

#include "WS.h"
#include "UI.h"

#define AVOID_VIRUS_DISTANCE 5
#define TARGET_MARGE 800
#define ENEMIE_SECURE_DISTANCE 400
#define WALL_ESCAPE_DISTANCE 300
#define WALL_FORCE 1000
#define ESCAPE_VIRUS_ANGLE 100
#define SPLIT_DISTANCE_COEF 2
#define ESCAPE_THREAT_ANGLE 90
#define AVOID_VIRUS_DISTANCE_NEAR_WALL 100
#define TARGET_WALL_FACTOR 3

typedef enum ESCAPE_STATE
{
	IDLE,
	ESCARGOT, //Léo
	ESCAPE
} ESCAPE_STATE;

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

Node* nearestFood;

ESCAPE_STATE state;

unsigned int newPlayerNodeId;

unsigned int playerID;
unsigned int player_length;
unsigned int playerTotalSize;

#endif