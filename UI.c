#include "UI.h"
#include "IA.h"

void DrawCircle(Circle* circle)
{
	filledCircleRGBA(pRenderer, circle->x, circle->y, circle->radius, circle->color.r, circle->color.g, circle->color.b, circle->color.a);
}

int InitUI()
{
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
		return 0;

	pWindow = SDL_CreateWindow("AgoBot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGTH, SDL_WINDOW_SHOWN);

	if(pWindow == NULL)
		return 0;

	pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
	if(pRenderer == NULL)
		return 0;

	return 1;
}

Circle Node2Circle(Node* node)
{
	Circle ret;
	ret.radius = node->size / 4;
	ret.x = node->x;
	ret.y = node->y;
	ret.color.r = node->R;
	ret.color.g = node->G;
	ret.color.b = node->B;
	ret.color.a = 255;
	return ret;
}

Vec2 World2Screen(Vec2 pos, Vec2 playerPos)
{
	Vec2 ret;
	memset(&ret, 0, sizeof(Vec2));

	Vec2 origin;

	origin.x = playerPos.x - (WINDOW_WIDTH);
	origin.y = playerPos.y - (WINDOW_HEIGTH);

	ret.x = pos.x - origin.x / 2;
	ret.y = pos.y - origin.y / 2;

	printf("Origin(%d, %d)\n", origin.x, origin.y);
	printf("PlayerPos(%d, %d)\n", playerPos.x, playerPos.y);
	printf("NodePos(%d, %d)\n", pos.x, pos.y);
	printf("Ret(%d, %d)\n\n", ret.x, ret.y);	

	return ret;
}

void DrawAllNodes()
{
	Node* p = NodeStack_get(nodes, playerID);
	if(p == NULL)
		return;

	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		if(tmp->node != NULL)
		{
			Circle nodeCircle = Node2Circle(tmp->node);

			Vec2 playerPos;
			playerPos.x = p->x;
			playerPos.y = p->y; 

			Vec2 nodePos;
			nodePos.x = tmp->node->x;
			nodePos.y = tmp->node->y;
			nodePos = World2Screen(nodePos, playerPos);

			nodeCircle.x = nodePos.x;
			nodeCircle.y = nodePos.y;

			DrawCircle(&nodeCircle);
		}
		tmp = tmp->next;
	}
}

void Render()
{
	SDL_RenderClear(pRenderer);

	DrawAllNodes();

	SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 255);

	SDL_RenderPresent(pRenderer);
}

void Loop(int* exit)
{
	SDL_Event evnt;

	while(SDL_PollEvent(&evnt))
	{
		if(evnt.type == SDL_QUIT)
			*exit = 1;
	}

	Render();
}