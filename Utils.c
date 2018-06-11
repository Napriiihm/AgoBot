#include "Utils.h"

#include "IA.h"

double max(double a, double b) { return (a > b) ? a : b; }
double min(double a, double b) { return (a > b) ? b : a; }

double Vec2_length(Vec2 vec)
{
	return sqrt(pow(vec.x, 2) + pow(vec.y, 2));
}

double splitDistance(Node* node)
{
	if(node == NULL)
		return 0;

	unsigned short size = node->size;
	double pipi = M_PI * M_PI;
	double modif = 3 + log(1 + size) / 10;
	double splitSpeed = 35 * min(pow(size, -M_PI / pipi / 10) * modif, 150);
	double ret = max(splitSpeed * 12.8, size * 2);

	return ret;
}

Node* NodeStack_getLargest(NodeStack* list)
{
	unsigned int max = 0;
	Node* ret;
	NodeStack* tmp = list;
	while(tmp != NULL)
	{
		if(tmp->node != NULL && tmp->node->size > max)
		{
			ret = tmp->node;
			max = tmp->node->size;
		}
		tmp = tmp->next;
	}

	return ret;
}

Node* NodeStack_getLowest(NodeStack* list)
{
	unsigned int min = 999999;
	Node* ret;
	NodeStack* tmp = list;
	while(tmp != NULL)
	{
		if(tmp->node != NULL && tmp->node->size < min)
		{
			ret = tmp->node;
			min = tmp->node->size;
		}
		tmp = tmp->next;
	}

	return ret;
}

ZONE getOppositeZone()
{
	ZONE curr = getZone(player);
	switch(curr)
	{
	case LEFT_UP:
		return RIGHT_DOWN;
		break;
	case LEFT_DOWN:
		return RIGHT_UP;
		break;
	case RIGHT_UP:
		return LEFT_DOWN;
		break;
	case RIGHT_DOWN:
		return LEFT_UP;
		break;
	}
}

Vec2 gotoZone(ZONE zone)
{
	Vec2 ret;
	switch(zone)
	{
	case LEFT_UP:
		ret.x = 7200/4;
		ret.y = 3200/4;
		break;
	case LEFT_DOWN:
		ret.x = 7200/4;
		ret.y = 3200 * (3/4);
		break;
	case RIGHT_UP:
		ret.x = 7200 * (3/4);
		ret.y = 3200/4;
		break;
	case RIGHT_DOWN:
		ret.x = 7200 * (3/4);
		ret.y = 3200 * (3/4);
		break;
	default:
		memset(&ret, 0, sizeof(Vec2));
		break;
	}
	return ret;
}

ZONE getZone(Node* p)
{
	Vec2 playerPos;
	memset(&playerPos, 0, sizeof(Vec2));
	
	if(p == NULL)
		return LEFT_UP;

	playerPos.x = p->x;
	playerPos.y = p->y;

	if(playerPos.x < 7200/2)
	{
		if(playerPos.y < 3200/2)
			return LEFT_UP;
		else
			return RIGHT_UP;
	}
	else
	{
		if(playerPos.y < 3200/2)
			return LEFT_DOWN;
		else
			return RIGHT_DOWN;
	}

	return LEFT_UP;
}

unsigned int getFoodNum(NodeStack* list)
{
	NodeStack* tmp = list;
	unsigned int ret = 0;
	while(tmp != NULL)
	{
		if(tmp->node != NULL && tmp->node->type == FOOD)
			ret += (tmp->node->size == 10 ? 1 : 2);
		tmp = tmp->next;
	}
	return ret;
}

void NodeStack_push(NodeStack** list, Node* elem)
{
	NodeStack* new = malloc(sizeof(NodeStack));
	new->node = elem;
	new->next = *list;
	*list = new;
}

void NodeStack_clear(NodeStack** list)
{
	NodeStack* tmp = *list;
	while(tmp != NULL)
	{
		NodeStack* next = tmp->next;
		if(tmp->node != NULL)
		{
			free(tmp->node);
			tmp->node = NULL;
		}
		free(tmp);
		tmp = NULL;
		tmp = next;
	}
}

Node* NodeStack_get(NodeStack* list, unsigned int id)
{
	NodeStack* tmp = list;
	while(tmp != NULL)
	{
		if(tmp ->node != NULL && tmp->node->nodeID == id)
			return tmp->node;
		tmp = tmp->next;
	}
	return NULL;
}

NodeStack* NodeStack_remove(NodeStack* list, unsigned int id)
{
	NodeStack* prev;
	NodeStack* tmp;
	if(list == NULL)
		return list;

	prev = list;

	if(prev->node->nodeID == id)
	{
		list = prev->next;
		free(prev->node);
		prev->node = NULL;
		free(prev);
		return list;
	}

	tmp = prev->next;
	while(tmp != NULL)
	{
		if(tmp->node != NULL && tmp->node->nodeID == id)
		{
			prev->next = tmp->next;
			free(tmp->node);
			tmp->node = NULL;
			free(tmp);
			return list;
		}
		prev = tmp;
		tmp = tmp->next;
	}

	return list;
}

char NodeStack_find(NodeStack* list, unsigned int id)
{
	NodeStack* tmp = list;
	while(tmp != NULL)
	{
		if(tmp->node != NULL && tmp->node->nodeID == id)
			return 1;
		tmp = tmp->next;
	}
	return 0;
}

size_t NodeStack_length(NodeStack* list)
{
	size_t ret = 0;
	NodeStack* tmp = list;
	while(tmp != NULL)
	{
		ret++;
		tmp = tmp->next;
	}

	return ret;
}

void NodeStack_update(NodeStack** list, Node* elem)
{
	NodeStack* tmp = *list;
	while(tmp != NULL)
	{
		if(tmp->node != NULL && elem != NULL && tmp->node->nodeID == elem->nodeID)
		{
			free(tmp->node);
			tmp->node = elem;
			break;
		}
		tmp = tmp->next;
	}
}

double getDistance(Node* n1, Node* n2)
{
	return sqrt(pow((double)n1->x - (double)n2->x, 2) + pow((double)n1->y - (double)n2->y, 2));
}

double getDist(Vec2 a, Vec2 b)
{
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

Vec2f Vec2f_normalize(Vec2 vec)
{
	Vec2 zero;
	memset(&zero, 0, sizeof(Vec2f));

	double dist = getDist(zero, vec);

	Vec2f ret;
	ret.x = vec.x / dist;
	ret.y = vec.y / dist;

	return ret; 
}

void printHex(char* data, size_t size)
{
	int j = 0;
	for(int i = 0; i < size; i++)
	{
		if(j % 16 == 0)
			printf("\n0x%08x : ", j);
		printf("0x%x ", data[i] & 0xff);
		j++;
	}
	puts("");
}
