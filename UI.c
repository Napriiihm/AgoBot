#include "UI.h"
#include "IA.h"

void DrawCircle(Circle* circle)
{
	filledCircleRGBA(pRenderer, circle->x, circle->y, circle->radius, circle->color.r, circle->color.g, circle->color.b, circle->color.a);
}

void drawDebugLine(Vec2 start, Vec2 end, char r, char g, char b)
{
	aalineRGBA(pRenderer, start.x, start.y, end.x, end.y, r, g, b, 255);
}

void drawDebugCircle(int x, int y, short radius, char r, char g, char b)
{
	aacircleRGBA(pRenderer, x, y, radius, r, g, b, 255);
}

void drawWalls()
{
	if(player == NULL)
		return;

	Vec2 topLeft, topRight, bottomLeft, bottomRight;
	topLeft.x = 0; topLeft.y = 0;
	topRight.x = 7200; topRight.y = 0;
	bottomLeft.x = 0; bottomLeft.y = 3200;
	bottomRight.x = 7200; bottomRight.y = 3200;

	Vec2 topLeftWall, topRightWall, bottomLeftWall, bottomRightWall;

	topLeftWall.x = topLeft.x + WALL_ESCAPE_DISTANCE;
	topLeftWall.y = topLeft.y + WALL_ESCAPE_DISTANCE;
	topRightWall.x = topRight.x - WALL_ESCAPE_DISTANCE;
	topRightWall.y = topRight.y + WALL_ESCAPE_DISTANCE;
	bottomLeftWall.x = bottomLeft.x - WALL_ESCAPE_DISTANCE;
	bottomLeftWall.y = bottomLeft.y + WALL_ESCAPE_DISTANCE;
	bottomRightWall.x = bottomRight.x - WALL_ESCAPE_DISTANCE;
	bottomRightWall.y = bottomRight.y - WALL_ESCAPE_DISTANCE;

	Vec2 playerPos = NodetoVec2(player);

	topLeft = World2Screen(topLeft, playerPos);
	topRight = World2Screen(topRight, playerPos);
	bottomLeft = World2Screen(bottomLeft, playerPos);
	bottomRight = World2Screen(bottomRight, playerPos);

	topLeftWall = World2Screen(topLeftWall, playerPos);
	topRightWall = World2Screen(topRightWall, playerPos);
	bottomLeftWall = World2Screen(bottomLeftWall, playerPos);
	bottomRightWall = World2Screen(bottomRightWall, playerPos);

	drawDebugLine(topLeft, topRight, 0, 0, 0);
	drawDebugLine(topRight, bottomRight, 0, 0, 0);
	drawDebugLine(bottomRight, bottomLeft, 0, 0, 0);
	drawDebugLine(bottomLeft, topLeft, 0, 0, 0);

	drawDebugLine(topLeftWall, topRightWall, 0, 255, 255);
	drawDebugLine(topRightWall, bottomRightWall, 0, 255, 255);
	drawDebugLine(bottomRightWall, bottomLeftWall, 0, 255, 255);
	drawDebugLine(bottomLeftWall, topLeftWall, 0, 255, 255);
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

	if(TTF_Init() == -1)
		return 0;

	pFont = TTF_OpenFont("Roboto-Regular.ttf", 60);

	return 1;
}

Vec2 getZoom()
{
	Vec2 ret; memset(&ret, 0, sizeof(Vec2));
	if(player == NULL)
		return ret;

	double factor = pow(min(64.0 / playerTotalSize, 1), 0.4);
    ret.x = WINDOW_WIDTH / factor;
    ret.y = WINDOW_HEIGTH / factor;

    return ret;
}

Circle Node2Circle(Node* node)
{
	Vec2 zoom = getZoom();
	Circle ret;
	ret.radius = node->size * WINDOW_HEIGTH / zoom.y;
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

	Vec2 zoom = getZoom();

	ret.x = (pos.x - playerPos.x + zoom.x / 2) * WINDOW_WIDTH / zoom.x;
	ret.y = (pos.y - playerPos.y + zoom.y / 2) * WINDOW_HEIGTH / zoom.y;	

	return ret;
}

void DrawAllNodes()
{
	if(player == NULL)
		return;

	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		if(tmp->node != NULL)
		{
			Circle nodeCircle = Node2Circle(tmp->node);

			Vec2 playerPos;
			playerPos.x = player->x;
			playerPos.y = player->y; 

			Vec2 nodePos;
			nodePos.x = tmp->node->x;
			nodePos.y = tmp->node->y;
			nodePos = World2Screen(nodePos, playerPos);

			nodeCircle.x = nodePos.x;
			nodeCircle.y = nodePos.y;

			DrawCircle(&nodeCircle);

			if(tmp->node->type == PLAYER)
			{
				char* toWrite = malloc(strlen(tmp->node->name) + 1 + 6);
				sprintf(toWrite, "%s [%d]", tmp->node->name, tmp->node->size);
				SDL_Color color;
				color.r = 255 - tmp->node->R;
				color.g = 255 - tmp->node->G;
				color.b = 255 - tmp->node->B;
				color.a = 255;
				SDL_Surface* textSurface = TTF_RenderUTF8_Blended(pFont, toWrite, color);
				SDL_Texture* texture = SDL_CreateTextureFromSurface(pRenderer, textSurface);

				Vec2 zoom = getZoom();
				unsigned short nodeSize = tmp->node->size;
				SDL_Rect rekt;
				rekt.x = nodePos.x - nodeSize / 2;
				rekt.y = nodePos.y;
				rekt.w = nodeSize * WINDOW_WIDTH / zoom.x;
				rekt.h = nodeSize * WINDOW_HEIGTH / zoom.y;

				SDL_RenderCopy(pRenderer, texture, NULL, &rekt);

				SDL_FreeSurface(textSurface);
				SDL_DestroyTexture(texture);
			}
		}
		tmp = tmp->next;
	}
}

void Clear()
{
	SDL_RenderClear(pRenderer);
}

void Render()
{
	drawWalls();

	SDL_SetRenderDrawColor(pRenderer, 225, 225, 225, 255);

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