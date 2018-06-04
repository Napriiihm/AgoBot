#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

typedef struct Vec2
{
	int x;
	int y;
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
	int x;
	int y;
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

void NodeStack_push(NodeStack** list, Node* elem);
void NodeStack_clear(NodeStack** list);
Node* NodeStack_get(NodeStack* list, unsigned int id);
NodeStack* NodeStack_remove(NodeStack* list, unsigned int id);
char NodeStack_find(NodeStack* list, unsigned int id);
size_t NodeStack_length(NodeStack* list);

double getDistance(Node* n1, Node* n2);
double getDist(Vec2 a, Vec2 b);

Vec2 normalize(Vec2 vec);

void printHex(char* data, size_t size);

#endif
