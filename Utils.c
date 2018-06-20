#include "Utils.h"

#include "IA.h"

double max(double a, double b) { return (a > b) ? a : b; }
double min(double a, double b) { return (a > b) ? b : a; }

Vec2 rotateVec2(Vec2 vec, int angle)
{
	double rad = angle * M_PI / 180.f;
	Vec2f ret;
	ret.x = vec.x * cos(rad) - vec.y * sin(rad);
	ret.y = vec.x * sin(rad) + vec.y * cos(rad);

	return Vec2ftoVec2(ret);
}

int getAngleVirus(Node *virus, Node *player)
{
	int dist = 2 * player->size + virus->size;
	SIDE cas = NOTHING;

	if (virus->x < dist)	cas = LEFT;
	else if ((7200 - virus->x) < dist )	cas = RIGHT;
	else if (virus->y < dist)	cas = UP;
	else if ((3200 - virus->y) < dist)	cas = DOWN;
	
	int angle = 90;

	if (cas == LEFT)
	{
		if (player->y > virus->y ) angle =  95;
		else angle =  -95;

		if ( abs(virus->y - player->y) < 10) angle = 180;
	} else if (cas == RIGHT)
	{	
		if (player->y > virus->y) angle = 95;
		else angle = -95;

		if ( abs(virus->y - player->y) < 10) angle = 180;
	} else if (cas == UP)
	{
		if (player->x < virus->x ) angle =  95;
		else angle =  -95;

		if ( abs(virus->x - player->x) < 10) angle = 180;
	} else if (cas == DOWN)
	{
		if (player->x < virus->x ) angle =  -95;
		else angle =  95;

		if ( abs(virus->x - player->x) < 10) angle = 180;
	}



	return angle;
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
	return node->size * 2.f;
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

void NodeStack_clear(NodeStack* list)
{
	while(list != NULL)
	{
		if(list->node == NULL)
		{
			list = list->next;
			continue;
		}
		list = NodeStack_remove(list, list->node->nodeID);
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

	if(prev->node != NULL && prev->node->nodeID == id)
	{
		list = prev->next;
		free(prev->node);
		prev->node = NULL;
		free(prev);
		prev = NULL;
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
			tmp = NULL;
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
	return sqrt(pow((double)n1->x - (double)n2->x, 2) + pow((double)n1->y - (double)n2->y, 2)) - n1->size - n2->size;
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