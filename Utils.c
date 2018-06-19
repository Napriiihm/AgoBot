#include "Utils.h"

#include "IA.h"

double max(double a, double b) { return (a > b) ? a : b; }
double min(double a, double b) { return (a > b) ? b : a; }

char isNearWall(Node* node)
{
	if(node->x - node->size < WALL_ESCAPE_DISTANCE || node->x + node->size > 7200 - WALL_ESCAPE_DISTANCE)
		return 1;

	if(node->y - node->size < WALL_ESCAPE_DISTANCE || node->x + node->size > 3200 - WALL_ESCAPE_DISTANCE)
		return 1;

	return 0;
}

void escapeWall(Node* node, Vec2* target)
{
	if(node->x - node->size < WALL_ESCAPE_DISTANCE || node->x + node->size > 7200 - WALL_ESCAPE_DISTANCE)
		target->x = -target->x;

	if(node->y - node->size < WALL_ESCAPE_DISTANCE || node->x + node->size > 3200 - WALL_ESCAPE_DISTANCE)
		target->y = -target->y;
}

char virusSurLeChemin(Node *food)
{
	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		Node* virus = tmp->node;
		if(virus == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		if(virus->type == VIRUS)
		{
			double distancePlayerVirus = getDistance(player, virus);
			double distancePlayerFood = getDistance(player, food);

			if(player->x - virus->x == 0)
			{
				if(distancePlayerFood > distancePlayerVirus - virus->size || getDistance(virus, food) < virus->size) 
					return 1;
			}
			else
			{
				double marge = 2 * player->size + player->size;
				double angleVirus = atan((virus->size + marge) / distancePlayerVirus);

				double a = (player->y - virus->y) / (player->x - virus->x);
				double b = player->y - a * player->x;

				double coteOppose = abs((a * food->x - food->y + b) / sqrt(pow(a, 2) + pow(-1, 2)));

				double angleFood = asin(coteOppose / distancePlayerFood);

				if(distancePlayerFood > distancePlayerVirus - 1.5 * virus->size && angleFood < angleVirus || getDistance(virus, food) < virus->size)
					return 1;
			}
		}
		tmp = tmp->next;
	}

	return 0;
}

double Vec2_length(Vec2 vec)
{
	return sqrt(pow(vec.x, 2) + pow(vec.y, 2));
}

int getMass(Node* node)
{
	return sqrt(100 * node->size);
}

double splitDistance(Node* node)
{
	return node->size * 3.f;
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

void Stack_push(Stack** stack, void* elem)
{
	Stack* new = malloc(sizeof(Stack));
	new->elem = elem;
	new->next = *stack;
	*stack = new;
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
	while(*list != NULL)
	{
		if((*list)->node == NULL)
		{
			*list = (*list)->next;
			continue;
		}
		*list = NodeStack_remove(*list, (*list)->node->nodeID);
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
			return;
		}
		tmp = tmp->next;
	}

	NodeStack_push(list, elem);
}

double getDistance(Node* n1, Node* n2)
{
	return sqrt(pow((double)n1->x - (double)n2->x, 2) + pow((double)n1->y - (double)n2->y, 2));
}

double getDist(Vec2 a, Vec2 b)
{
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

double getDistf(Vec2f a, Vec2f b)
{
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

Vec2f Vec2f_normalize(Vec2f vec)
{
	Vec2f zero;
	memset(&zero, 0, sizeof(Vec2f));

	double dist = getDistf(zero, vec);

	Vec2f ret;
	ret.x = vec.x / dist;
	ret.y = vec.y / dist;

	return ret; 
}

Vec2f Vec2_normalize(Vec2 vec)
{
	Vec2 zero;
	memset(&zero, 0, sizeof(Vec2));

	double dist = getDist(zero, vec);

	Vec2f ret;
	ret.x = vec.x / dist;
	ret.y = vec.y / dist;

	return ret; 
}

Vec2f Vec2toVec2f(Vec2 vec)
{
	Vec2f ret;
	ret.x = (double)vec.x;
	ret.y = (double)vec.y;

	return ret;
}

Vec2 Vec2ftoVec2(Vec2f vec)
{
	Vec2 ret;
	ret.x = (int)vec.x;
	ret.y = (int)vec.y;

	return ret;
}

Vec2 NodetoVec2(Node* node)
{
	Vec2 ret;
	ret.x = node->x;
	ret.y = node->y;

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