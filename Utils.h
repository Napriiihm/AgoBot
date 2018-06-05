#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

typedef struct Vec2
{
	unsigned int x;
	unsigned int y;
} Vec2;

typedef enum NODE_TYPE
{
	FOOD,
	VIRUS,
	PLAYER
} NODE_TYPE;

typedef struct Map
{
	double left;
	double top;
	double right;
	double bottom;
} Map;

typedef struct Node
{
	unsigned int nodeID;
	unsigned int x;
	unsigned int y;
	unsigned short size;
	unsigned char flags;
	unsigned char R,G,B;
	NODE_TYPE type;
	unsigned char* name;
} Node;

typedef struct NodeStack
{
	Node* node;
	struct NodeStack* next;
} NodeStack;

typedef struct Packet
{
	unsigned char* data;
	size_t len;
} Packet;

void NodeStack_push(NodeStack** list, Node* elem);
void NodeStack_clear(NodeStack** list);
Node* NodeStack_get(NodeStack* list, unsigned int id);
NodeStack* NodeStack_remove(NodeStack* list, unsigned int id);
char NodeStack_find(NodeStack* list, unsigned int id);
size_t NodeStack_length(NodeStack* list);
void NodeStack_update(NodeStack** list, Node* elem);

double getDistance(Node* n1, Node* n2);
double getDist(Vec2 a, Vec2 b);

Vec2 normalize(Vec2 vec);

void printHex(char* data, size_t size);

int max(int a, int b);

#endif
