#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

typedef struct Map
{
	double left;
	double top;
	double right;
	double bottom;
} Map;

typedef struct Player
{
	double x;
	double y;
	unsigned short size;
} Player;

typedef struct Node
{
	unsigned int nodeID;
	int x;
	int y;
	unsigned short size;
	unsigned char flags;
	unsigned char R,G,B;
	unsigned char* name;
} Node;

typedef struct NodeStack
{
	Node* node;
	struct NodeStack* next;
} NodeStack;

void NodeStack_push(NodeStack** list, Node* elem);
void NodeStack_clear(NodeStack** list);
Node* NodeStack_get(NodeStack* list, unsigned int id);
void NodeStack_remove(NodeStack** list, unsigned int id);
char NodeStack_find(NodeStack* list, unsigned int id);

Node* GetNearestFood(NodeStack* list, Player* p);

void printHex(char* data, size_t size);

#endif
