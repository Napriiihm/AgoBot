#include "Utils.h"

int max(int a, int b) { return (a > b) ? a : b; }

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

Vec2 normalize(Vec2 vec)
{
	Vec2 zero;
	memset(&zero, 0, sizeof(Vec2));

	double dist = getDist(zero, vec);

	Vec2 ret;
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
