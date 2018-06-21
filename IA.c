#include "IA.h"

void IAInit(const char* name) 
{ 
	BotName = malloc(strlen(name)+1);
	strcpy(BotName, name);

	nearestFood = NULL;
	state = IDLE;
}

char* getName() { return BotName; }

char isPlayer(Node* node)
{
	if(player == NULL || node == NULL)
		return 0;

	if(node->type != PLAYER)
		return 0;

	return strcmp(node->name, BotName) == 0;
}

void UpdateNodes(unsigned char* data)
{
	player = NULL;
	player_length = 0;
	size_t NodeSize = 18; //sizeof(Node) - sizeof(char*)
	size_t totalNameLength = 0; //taille des noms calculé pour skip vers les prochains nodes

	unsigned short deadLen; //taille des cellule manger
	memcpy(&deadLen, data, 2); //copie vers la variable

	unsigned int startNodePos = 2 + deadLen * 2 * sizeof(int); //les nodes commence a cette position
	unsigned int end; //variable gérant l'octet aprés le node courrant pout checker si c'est 0x00
	memcpy(&end, data + startNodePos, sizeof(unsigned int)); //end = premier octet de la premiere cellule

	int i = 0; //compteur de cellule
	while(end != 0) //tant qu'il reste des cellules
	{
		unsigned char* pos = data + startNodePos + i*NodeSize + totalNameLength; //position de la cellule courrante
		Node* node = malloc(sizeof(Node)); //on alloue la memoire pour la cellule qu'on crée

		memcpy(node, pos, NodeSize); //on copie toute les donnée recu dans notre cellule

		node->isSafe = 1;

		if(node->flags&0x8) //si la cellule a un nom
		{
			node->type = PLAYER;

			size_t nameLength = strlen(pos + NodeSize); //taille du nom
			node->name = malloc(nameLength+1); //on aloue la memoire pour le nom
			strcpy(node->name, data + startNodePos + (i+1)*NodeSize + totalNameLength); //on copie le nom
			totalNameLength += nameLength+1;//on augment la taille total des noms

			if(strcmp(node->name, BotName) == 0) //si la cellule est noter bot
			{
				player_length++;
				player = node;
			}
		}
		else if(node->flags&0x1)
			node->type = VIRUS;
		else
			node->type = FOOD;
		
		NodeStack_update(&nodes, node);

		memcpy(&end, data + startNodePos + (i+1)*(NodeSize) + totalNameLength, sizeof(unsigned int)); //la nouvelle fin (check si c'est 0)
		i++;
	}

	unsigned int new_pos = startNodePos + i*(NodeSize) + totalNameLength + sizeof(unsigned int); //nouvelle pos aprés avoir lu les cellules

	unsigned short nbDead; //nombre de cellule morte depuis la derniére fois
	memcpy(&nbDead, data + new_pos, sizeof(unsigned short)); //copie

	for(int j = 0; j < nbDead; j++) //pour chaque cellule morte
	{
		unsigned int nodeID;
		memcpy(&nodeID, data + new_pos + sizeof(unsigned short) + j * sizeof(unsigned int), sizeof(unsigned int)); //on prend l'id

		nodes = NodeStack_remove(nodes, nodeID); //on suprime de notre liste
	}
}

void Move(struct lws *wsi, Vec2 pos)
{
	unsigned char* packet = malloc(13);
	memset(packet, 0, 13);
	*packet = 16;

	memcpy(packet+1, &pos, sizeof(pos));

	sendCommand(wsi, packet, 13);
}

void MoveZero(struct lws* wsi)
{
	Vec2 pos; memset(&pos, 0, sizeof(Vec2));

	Move(wsi, pos);
}

void Split(struct lws *wsi)
{
	unsigned char* packet = malloc(1);
	packet[0] = 17;

	sendCommand(wsi, packet, 1);
}

char canSplit(Node* node1, Node* node2)
{
	return node1->size / 2.f >= node2->size;
}

void IAUpdate(struct lws *wsi)
{
	if(player == NULL)
		return;

	Vec2 playerPos = NodetoVec2(player);
	Vec2 playerPosScreen = World2Screen(playerPos);

	NodeStack* foods = NULL, *targets = NULL, *threats = NULL, *viruses = NULL, *passiveThreats = NULL;

	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		Node* node = tmp->node;
		if(node == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		Vec2 nodePos = NodetoVec2(node);
		Vec2 nodePosScreen = World2Screen(NodetoVec2(node));

		double distance = getDistance(player, node);

		if(node->type == FOOD)
			NodeStack_push(&foods, node);
		else if(node->type == VIRUS)
		{
			Vec2 virusPosScreen = World2Screen(NodetoVec2(node));

			if(player->size > 105 && player_length < 16)
			{
				NodeStack_push(&passiveThreats, node);
				if(getWallDistance(node) < node->size + player->size * 2)
					drawDebugCircle(virusPosScreen.x, virusPosScreen.y, node->size + AVOID_VIRUS_DISTANCE_NEAR_WALL, 0xee, 0x82, 0xee);
				else
					drawDebugCircle(virusPosScreen.x, virusPosScreen.y, node->size + AVOID_VIRUS_DISTANCE, 255, 0, 0);
			}

			if(player->size > 105 && player_length < 16 && distance < AVOID_VIRUS_DISTANCE)
				NodeStack_push(&viruses, node);
		}
		else if(node->type == PLAYER && isPlayer(node) == 0)
		{
			//printf("player node %s\n", node->name);
			double mire = TARGET_MARGE + sqrt(pow(7200.f/2, 2) + pow(3200/2.f, 2)) / getWallDistance(node) * TARGET_WALL_FACTOR;
			//printf("WallDistance = %f\n", getWallDistance(node));

			if(canSplit(player, node))
			{
				//puts("I can split kill him !");
				mire = player->size * SPLIT_DISTANCE_COEF;
			}

			//printf("mire de %s = %f\n", node->name, mire);
			//printf("distance = %f\n", distance);
			//printf("difference de taille : %f\n", player->size / (float)node->size);

			if(player->size / (float)node->size > 1.2f && distance < mire)
			{
				//puts("TARGETTTT");
				drawDebugCircle(nodePosScreen.x, nodePosScreen.y, mire, 0, 0, 255);	
				NodeStack_push(&targets, node);
			}
			else if(node->size / (float)player->size > 1)
			{
				//printf("threat !!!\n");
				double marge = ENEMIE_SECURE_DISTANCE;
				if(canSplit(node, player))
				{
					//puts("can slit");
					marge = node->size * SPLIT_DISTANCE_COEF;
				}

				NodeStack_push(&passiveThreats, node);

				drawDebugCircle(nodePosScreen.x, nodePosScreen.y, marge + node->size, 255, 0, 0);
				if(distance < marge + node->size)
					NodeStack_push(&threats, node);					
			}	
			else
				;//puts("OSEF");			
		}

		tmp = tmp->next;
	}

	tmp = passiveThreats;
	while(tmp != NULL)
	{
		Node* threat = tmp->node;
		if(threat == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		double dangerRadius = 0;
		if(threat->type == VIRUS && player->size > 105 && player_length != 16)
		{
			if(getWallDistance(threat) < 2 * player->size + threat->size)
				dangerRadius = threat->size + AVOID_VIRUS_DISTANCE_NEAR_WALL;
			else
				dangerRadius = threat->size + AVOID_VIRUS_DISTANCE;
		}
		else if(threat->size / (float)player->size > 1.1f)
		{
			if(canSplit(threat, player))
				dangerRadius = threat->size * SPLIT_DISTANCE_COEF;
			else
				dangerRadius = ENEMIE_SECURE_DISTANCE + threat->size;
		}

		NodeStack* tmp2 = foods;
		while(tmp2 != NULL)
		{
			Node* food = tmp2->node;
			if(food == NULL)
			{
				tmp2 = tmp2->next;
				continue;
			}

			double dist = getDistance(food, threat) + threat->size;
			if(dist <= dangerRadius)
				food->isSafe = 0;

			tmp2 = tmp2->next;
		}

		tmp = tmp->next;
	}

	if(NodeStack_length(threats) > 0)
	{
		if(NodeStack_length(viruses) > 0)
		{
			//printf("DFKLGHSDFJLGSDFJLGHSDFLGHSDFHLG\n");
			Node* virus = viruses->node;
			if(virus != NULL)
			{
				Vec2 virusPos = NodetoVec2(virus);
				Vec2 dir; 
				dir.x = virusPos.x - playerPos.x;
				dir.y = virusPos.y - playerPos.y;

				Vec2 threatPos = NodetoVec2(NodeStack_getNearest(threats, player));

				Vec2 enemiPlayer;
				enemiPlayer.x = playerPos.x - threatPos.x;
				enemiPlayer.y = playerPos.y - threatPos.y;

				Vec2 playerVirus;
				playerVirus.x = virusPos.x - playerPos.x;
				playerVirus.y = virusPos.y - playerPos.y;
				
				double angle = getAngle(enemiPlayer, playerVirus);
				if(angle > 0)
					angle = -ESCAPE_VIRUS_ANGLE;
				else
					angle = ESCAPE_VIRUS_ANGLE;

				Vec2 rotate = rotateVec2(dir, angle);

				rotate.x = playerPos.x + rotate.x;
				rotate.y = playerPos.y + rotate.y;

				drawDebugLine(World2Screen(playerPos), World2Screen(virusPos), 25, 25, 200);
				drawDebugLine(World2Screen(playerPos), World2Screen(rotate), 0, 0, 255);

				Move(wsi, rotate);
			}
		}
		else
		{
			Vec2 escape;

			tmp = threats;
			while(tmp != NULL)
			{
				Node* threat = tmp->node;
				if(threat == NULL)
				{
					tmp = tmp->next;
					continue;
				}

				Vec2 threatPos = NodetoVec2(threat);
				Vec2 offset;
				offset.x = playerPos.x - threatPos.x;
				offset.y = playerPos.y - threatPos.y;

				drawDebugLine(World2Screen(playerPos), World2Screen(threatPos), 255, 0, 0);

				escape.x += offset.x;
				escape.y += offset.y;

				tmp = tmp->next;
			}

			if(getWallDistance(player) < WALL_ESCAPE_DISTANCE)
			{
				Vec2 wallForce = getWallForce(player);

				Vec2 wallForcePlace;
				wallForcePlace.x = playerPos.x + wallForce.x;
				wallForcePlace.y = playerPos.y + wallForce.y;
				drawDebugLine(World2Screen(playerPos), World2Screen(wallForcePlace), 125, 125, 125);

				escape.x += wallForce.x;
				escape.y += wallForce.y;
			}

			drawDebugLine(World2Screen(escape), World2Screen(playerPos), 0, 0, 255);
			Move(wsi, escape);
		}
	}
	else if(NodeStack_length(targets) > 0)
	{
		double bestValue = 9999999;
		Node* go = NULL;
		char split = 0;
		double splitTimer;

		tmp = targets;
		while(tmp != NULL)
		{
			Node* target = tmp->node;
			if(target == NULL)
			{
				tmp = tmp->next;
				continue;
			}

			double dist = getDistance(player, target);
			double value = dist * dist / (float)target->size;

			if(canSplit(player, target) && dist < player->size * SPLIT_DISTANCE_COEF && NodeStack_length(passiveThreats) == 0)
			{
				go = target;
				split = 1;
				//printf("Go Splitkill\n");
				splitTimer = clock();
				break;
			}

			if(go == NULL || value < bestValue)
			{
				go = target;
				bestValue = value;
			}

			tmp = tmp->next;
		}

		Vec2 move = NodetoVec2(go);

		if(NodeStack_length(viruses) > 0)
		{
			Node* virus = viruses->node;
			if(virus != NULL)
			{
				Vec2 virusPos = NodetoVec2(virus);
				Vec2 dir; 
				dir.x = virusPos.x - playerPos.x;
				dir.y = virusPos.y - playerPos.y;

				Vec2 targetPos = NodetoVec2(NodeStack_getNearest(targets, player));

				Vec2 playerTarget;
				playerTarget.x = targetPos.x - playerPos.x;
				playerTarget.y = targetPos.y - playerPos.y;

				Vec2 playerVirus;
				playerVirus.x = virusPos.x - playerPos.x;
				playerVirus.y = virusPos.y - playerPos.y;
				
				double angle = getAngle(playerTarget, playerVirus);
				if(angle > 0)
					angle = -ESCAPE_VIRUS_ANGLE;
				else
					angle = ESCAPE_VIRUS_ANGLE;

				Vec2 rotate = rotateVec2(dir, angle);

				rotate.x = playerPos.x + rotate.x;
				rotate.y = playerPos.y + rotate.y;

				drawDebugLine(World2Screen(playerPos), World2Screen(virusPos), 25, 25, 200);
				drawDebugLine(World2Screen(playerPos), World2Screen(rotate), 0, 0, 255);

				move = rotate;
			}
		}		

		Vec2 targetPosScreen = World2Screen(move);
		drawDebugLine(playerPosScreen, targetPosScreen, 255, 255, 0);
		Move(wsi, move);

		if(split && clock() - splitTimer > 2)
		{
			Split(wsi);
		}
	}
	else if(NodeStack_length(viruses) > 0)
	{
		Node* virus = viruses->node;
		if(virus != NULL)
		{
			Vec2 virusPos = NodetoVec2(virus);
			Vec2 dir; 
			dir.x = virusPos.x - playerPos.x;
			dir.y = virusPos.y - playerPos.y;

			int angle = getAngleVirus(virus, player);
			Vec2 rotate = rotateVec2(dir, angle);

			rotate.x = playerPos.x + rotate.x;
			rotate.y = playerPos.y + rotate.y;

			drawDebugLine(World2Screen(playerPos), World2Screen(virusPos), 25, 25, 200);
			drawDebugLine(World2Screen(playerPos), World2Screen(rotate), 0, 0, 255);

			Move(wsi, rotate);
		}
	}
	else if(NodeStack_length(foods) > 0)
	{
		Node* target = NULL;
		double bestValue = 99999999999;

		tmp = foods;
		while(tmp != NULL)
		{
			Node* food = tmp->node;
			if(food == NULL)
			{
				tmp = tmp->next;
				continue;
			}
			
			Vec2 foodPosScreen = World2Screen(NodetoVec2(food));
			if(food->isSafe)
				drawDebugCircle(foodPosScreen.x, foodPosScreen.y, food->size + 5, 0, 255, 0);
			else
				drawDebugCircle(foodPosScreen.x, foodPosScreen.y, food->size + 5, 255, 0, 0);

			if(!food->isSafe)
			{
				tmp = tmp->next;
				continue;
			}

			double distance = getDistance(player, food);
			double value = distance * distance * distance / (float)food->size;
			if(target == NULL || value < bestValue)
			{
				bestValue = value;
				target = food;
			}

			tmp = tmp->next;
		}

		if(target != NULL)
		{
			nearestFood = target;
			drawDebugLine(World2Screen(playerPos), World2Screen(NodetoVec2(target)), 0, 255, 0);
			Move(wsi, NodetoVec2(target));
		}
	}
	else
	{
		Vec2 middle;
		middle.x = 7200/2;
		middle.y = 3200/2;

		Move(wsi, middle);h
	}
}

void AddNode(unsigned char* data)
{
	unsigned int id;
	memcpy(&id, data, sizeof(unsigned int));

	//Node* node = NodeStack_get(nodes, id);
	//if(node != NULL && NodeStack_find(playerNodes, node->nodeID))
	//	NodeStack_push(&playerNodes, node);
}

void IARecv(unsigned char* payload, int* exit)
{
	unsigned char opcode = payload[0];
	switch(opcode)
	{
	case 16:
		UpdateNodes(payload+1);
		break;

	case 17:
		//printf("View Update\n");
		break;

	case 18:
		//printf("Reset all Cells\n");
		NodeStack_clear(nodes);
		break;

	case 20:
		//printf("Reset owned cells\n");
		NodeStack_clear(playerNodes);
		break;

	case 21:
		//printf("Draw debug line\n");
		break;

	case 32:
		//printf("Owns blob\n");
		AddNode(payload+1);
		break;

	case 49:
		//printf("FFA Leaderboard\n");
		break;

	case 50:
		//printf("Team Leaderboard\n");
		break;

	case 64:
		//printf("Game area size\n");
		break;

	case 72:
		//printf("HelloHelloHello\n");
		break;

	case 240:
		//printf("Message length\n");
		break;

	case 0:
		break;

	default:
		printf("Unknown opcode : %x\n", opcode);
		break;
	}
}