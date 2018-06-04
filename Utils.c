#include "Utils.h"

Node* GetNearestFood(NodeStack* list, Player* p)
{
	double nearDist = 999999;
	NodeStack* tmp = list;
	Node* near = NULL;
	while(tmp != NULL)
	{
		if(tmp->node != NULL && tmp->node->size <= 23)
		{
			float dist = sqrt(pow((float)tmp->node->x - p->x, 2) + pow((float)tmp->node->y - p->y, 2));
			if(dist < nearDist)
			{
				nearDist = dist;
				near = tmp->node;
			}
		}
		tmp = tmp->next;
	}
	return near;
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

void NodeStack_remove(NodeStack** list, unsigned int id)
{

	NodeStack* prev;
	NodeStack* tmp = *list;
	if(tmp == NULL)
		return;

	while(tmp != NULL)
	{
		if(tmp->node != NULL && tmp->node->nodeID == id)
		{
			NodeStack* next = tmp->next;
			free(tmp->node);
			tmp->node = NULL;
			prev = next;
			return;
		}
		prev = tmp;
		tmp = tmp->next;
	}
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
