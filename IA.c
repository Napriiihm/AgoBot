#include "IA.h"

void IAInit(const char* name) { BotName = malloc(strlen(name)+1); strcpy(BotName, name); }
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
	playerTotalSize = 1;
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

		if(newPlayerNodeId == node->nodeID)
		{
			NodeStack_update(&playerNodes, node);
			newPlayerNodeId = 0;
		}

		if(node->flags&0x8) //si la cellule a un nom
		{
			node->type = PLAYER;
			size_t nameLength = strlen(pos + NodeSize); //taille du nom
			node->name = malloc(nameLength+1); //on aloue la memoire pour le nom
			strcpy(node->name, data + startNodePos + (i+1)*NodeSize + totalNameLength); //on copie le nom
			totalNameLength += nameLength+1;//on augment la taille total des noms
			if(strcmp(node->name, BotName) == 0) //si la cellule est noter bot
			{
				NodeStack_push(&playerNodes, node);
				player_length++;
				player = node;
				playerID = player->nodeID;
				playerTotalSize += node->size;
			}
		}
		else if(node->flags&0x1)
			node->type = VIRUS;
		else
			node->type = FOOD;
		
		//char* temp = "0";
		//printf("    Node : [id:%u x:%d y:%d size:%d F:0x%x R:0x%x G:0x%x B:0x%x T:0x%x, N:%s]\n", node->nodeID, node->x, node->y, node->size & 0xFFFF, node->flags, node->R & 0xff, node->G & 0xff, node->B & 0xff, node->type, node->flags&8 ? node->name : temp);

		if(NodeStack_find(nodes, node->nodeID) == 0) //si on pas a deja cette cellule dans notre liste on l'update
			NodeStack_push(&nodes, node); //on ajoute cette cellule a notre list
		else
			NodeStack_update(&nodes, node);

		memcpy(&end, data + startNodePos + (i+1)*(NodeSize) + totalNameLength, sizeof(unsigned int)); //la nouvelle fin (check si c'est 0)
		i++;
	}

	//printf("Player : %p[%d]\n", player, playerID);

	unsigned int new_pos = startNodePos + i*(NodeSize) + totalNameLength + sizeof(unsigned int); //nouvelle pos aprés avoir lu les cellules

	unsigned short nbDead; //nombre de cellule morte depuis la derniére fois
	memcpy(&nbDead, data + new_pos, sizeof(unsigned short)); //copie

	for(int j = 0; j < nbDead; j++) //pour chaque cellule morte
	{
		unsigned int nodeID;
		memcpy(&nodeID, data + new_pos + sizeof(unsigned short) + j * sizeof(unsigned int), sizeof(unsigned int)); //on prend l'id
		
		//if(NodeStack_find(playerNodes, nodeID))
		//	playerNodes = NodeStack_remove(playerNodes, nodeID);

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

	printf("Send move packet\n");
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
	return (node1->size / (float)node2->size > 2.8f && node1->size / (float)node2->size < 20.f);
}

void IAV3(struct lws* wsi)
{
	if(player == NULL)
		return;

	NodeStack *foods, *threats, *viruses, *splitTargets;

	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		if(tmp->node == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		Node* node = tmp->node;
		if(node->type != VIRUS && player->size / (float)node->size > 1.33f)
			NodeStack_push(&foods, node);
		else if(node->type != VIRUS && node->size / (float)player->size > 1.3f)
			NodeStack_push(&threats, node);
		else if(node->type == VIRUS && player->size / (float)node->size > 1.1f)
			NodeStack_push(&viruses, node);

		tmp = tmp->next;
	}

	double* badAngles[1024]; int badAnglesIndex = 0; memcpy(badAngles, 0, 1024 * sizeof(double*));
	double** obstaclesList[1024]; int obstaclesIndex = 0; memcpy(obstaclesList, 0, 1024 * sizeof(double**));

	int i = 0;
	tmp = threats;
	while(tmp != NULL)
	{
		if(tmp->node == NULL || isPlayer(tmp->node))
		{
			tmp = tmp->next;
			continue;
		}

		double enemyDistance = getDistance(tmp->node, player);

		double splitDangerDistance = tmp->node->size + SPLIT_DISTANCE + DANGER_DISTANCE;
		double normalDangerDistance = tmp->node->size + DANGER_DISTANCE;
		double shiftDistance = player->size;

		puts("Found distance.");

		char enemyCanSplit = canSplit(player, tmp->node);
		double secureDistance = enemyCanSplit ? splitDangerDistance : normalDangerDistance;

		NodeStack* tmp2 = foods;
		while(tmp2 != NULL)
		{
			if(getDistance(tmp2->node, player) < secureDistance + shiftDistance)
				foods = NodeStack_remove(foods, tmp2->node->nodeID);	
			tmp2 = tmp2->next;
		}

		puts("Remove unsecure food.");

		if(enemyCanSplit)
		{
			//drawCircle(pos, splitDangerDistance)
			//drawCircle(pos, splitDangerDistance + shiftDistance)
		}
		else
		{
			//drawCircle(pos, normalDangerDistance)
			//drawCircle(pos, normalDangerDistance + shiftDistance)
		}

		/*
		if (allPossibleThreats[i].danger && getLastUpdate() - allPossibleThreats[i].dangerTimeOut > 1000) {

            allPossibleThreats[i].danger = false;
        }
		*/

		puts("Figured out who was important.");

		if((enemyCanSplit && enemyDistance < splitDangerDistance) )//|| (enemyCanSplit && danger))
		{
			double* bad = malloc(3 * sizeof(double));
			double* angleRange = getAngleRange(player, tmp->node, i, splitDangerDistance);
			memcpy(bad, angleRange, 2 * sizeof(double));
			memcpy(bad + 2 * sizeof(double), &enemyDistance, sizeof(double));
			badAngles[badAnglesIndex++] = bad;
		}
		else if((!enemyCanSplit && enemyDistance < normalDangerDistance) ) //|| (!enemyCanSplit && danger))
		{
			double* bad = malloc(3 * sizeof(double));
			double* angleRange = getAngleRange(player, tmp->node, i, splitDangerDistance);
			memcpy(bad, angleRange, 2 * sizeof(double));
			memcpy(bad + 2 * sizeof(double), &enemyDistance, sizeof(double));
			badAngles[badAnglesIndex++] = bad;
		}
		else if(enemyCanSplit && enemyDistance < splitDangerDistance + shiftDistance)
		{
			double un = 1.f, zero = 0.f;
			double* angleRange = getAngleRange(player, tmp->node, i, splitDangerDistance);
			double angle1; memcpy(&angle1, angleRange, sizeof(double));
			Vec2f arg; memcpy(&arg, angleRange, 2 * sizeof(double));
			double angle2 = (double)rangeToAngle(arg);

			double** ret = malloc(2 * sizeof(double*));
			*ret = malloc(2 * sizeof(double));
			*(ret + sizeof(double)) = malloc(2 * sizeof(double));

			memcpy(*ret, &angle1, sizeof(double));
			memcpy(*ret + sizeof(double), &un, sizeof(double));
			memcpy(*(ret + sizeof(double)), &angle2, sizeof(double));
			memcpy(*(ret + sizeof(double)) + sizeof(double), &zero, sizeof(double));
			obstaclesList[obstaclesIndex++] = ret;
		}
		else if(! enemyCanSplit && enemyDistance < normalDangerDistance + shiftDistance)
		{
			double un = 1.f, zero = 0.f;
			double* angleRange = getAngleRange(player, tmp->node, i, splitDangerDistance);
			double angle1; memcpy(&angle1, angleRange, sizeof(double));
			Vec2f arg; memcpy(&arg, angleRange, 2 * sizeof(double));
			double angle2 = (double)rangeToAngle(arg);

			double** ret = malloc(2 * sizeof(double*));
			*ret = malloc(2 * sizeof(double));
			*(ret + sizeof(double)) = malloc(2 * sizeof(double));

			memcpy(*ret, &angle1, sizeof(double));
			memcpy(*ret + sizeof(double), &un, sizeof(double));
			memcpy(*(ret + sizeof(double)), &angle2, sizeof(double));
			memcpy(*(ret + sizeof(double)) + sizeof(double), &zero, sizeof(double));
			obstaclesList[obstaclesIndex++] = ret;
		}

		printf("Done with enemy: %d\n", i);

		tmp = tmp->next;
		i++;
	}	

	puts("done looking for enemies!");

	i = 0;
	tmp = viruses;
	while(tmp != NULL)
	{
		Node* node = tmp->node;
		if(node == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		/*
		if (player[k].size < allPossibleViruses[i].size) {
            drawCircle(allPossibleViruses[i].x, allPossibleViruses[i].y, allPossibleViruses[i].size + 10, 3);
            drawCircle(allPossibleViruses[i].x, allPossibleViruses[i].y, allPossibleViruses[i].size * 2, 6);

        } else {
            drawCircle(allPossibleViruses[i].x, allPossibleViruses[i].y, player[k].size + 50, 3);
            drawCircle(allPossibleViruses[i].x, allPossibleViruses[i].y, player[k].size * 2, 6);
        }
		*/

		double virusDistance = getDistance(node, player);
		if(player->size < node->size)
		{
			if(virusDistance < node->size * 2)
			{
				double un = 1.f, zero = 0.f;
				double* angleRange = getAngleRange(player, node, i, node->size + 10);
				double angle1; memcpy(&angle1, angleRange, sizeof(double));
				Vec2f arg; memcpy(&arg, angleRange, 2 * sizeof(double));
				double angle2 = (double)rangeToAngle(arg);

				double** ret = malloc(2 * sizeof(double*));
				*ret = malloc(2 * sizeof(double));
				*(ret + sizeof(double)) = malloc(2 * sizeof(double));

				memcpy(*ret, &angle1, sizeof(double));
				memcpy(*ret + sizeof(double), &un, sizeof(double));
				memcpy(*(ret + sizeof(double)), &angle2, sizeof(double));
				memcpy(*(ret + sizeof(double)) + sizeof(double), &zero, sizeof(double));
				obstaclesList[obstaclesIndex++] = ret;
			}
		}
		else
		{
			if(virusDistance < player->size * 2)
			{
				double un = 1.f, zero = 0.f;
				double* angleRange = getAngleRange(player, node, i, player->size + 50);
				double angle1; memcpy(&angle1, angleRange, sizeof(double));
				Vec2f arg; memcpy(&arg, angleRange, 2 * sizeof(double));
				double angle2 = (double)rangeToAngle(arg);

				double** ret = malloc(2 * sizeof(double*));
				*ret = malloc(2 * sizeof(double));
				*(ret + sizeof(double)) = malloc(2 * sizeof(double));

				memcpy(*ret, &angle1, sizeof(double));
				memcpy(*ret + sizeof(double), &un, sizeof(double));
				memcpy(*(ret + sizeof(double)), &angle2, sizeof(double));
				memcpy(*(ret + sizeof(double)) + sizeof(double), &zero, sizeof(double));
				obstaclesList[obstaclesIndex++] = ret;
			}
		}

		tmp = tmp->next;
		i++;
	}



}

void IAV2(struct lws* wsi)
{
	if(player == NULL)
	{
		MoveZero(wsi);
		return;
	}

	Vec2 playerPos = NodetoVec2(player);

	NodeStack *foods = NULL, *threats = NULL, *viruses = NULL, *splitTargets = NULL;

	Vec2 target; memset(&target, 0, sizeof(double));

	Node* split_target = NULL;

	puts("[IA] Looking around...");

	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		if(tmp->node == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		if(isPlayer(tmp->node))
		{
			tmp = tmp->next;
			continue;
		}

		Node* node = tmp->node;
		if(node->type == PLAYER && player_length < 6 && player->size > 100 && player->size / (float)node->size > 2.6)
			NodeStack_push(&splitTargets, node);
		else if(node->type == PLAYER && getDistance(player, node) < player->size * 3)
			NodeStack_push(&foods, node);
		else if(node->type != VIRUS && player->size / (float)node->size > 1.33f)
			NodeStack_push(&foods, node);
		else if(node->type != VIRUS && node->size / (float)player->size > 1.3f)
			NodeStack_push(&threats, node);
		else if(node->type == VIRUS && player->size / (float)node->size > 1.3f)
			NodeStack_push(&viruses, node);


		Vec2 pos = World2Screen(NodetoVec2(player), NodetoVec2(player));
		drawDebugCircle(pos.x, pos.y, player->size * 3, 255, 255, 0);

		tmp = tmp->next;
	}

	puts("[IA] Figured out what's around !");

	if(NodeStack_length(threats) > 0)
	{
		Vec2 offset;
		tmp = threats;
		puts("[IA] Looking for threats !");
		while(tmp != NULL)
		{
			Node* node = tmp->node;
			if(node == NULL)
			{
				tmp = tmp->next;
				continue;
			}

			double enemyDistance = getDistance(player, node);
			double splitDangerDistance = tmp->node->size + SPLIT_DISTANCE + DANGER_DISTANCE;
			double normalDangerDistance = tmp->node->size + DANGER_DISTANCE;
			double shiftDistance = player->size;

			char enemyCanSplit = canSplit(player, tmp->node);
			double secureDistance = enemyCanSplit ? splitDangerDistance : normalDangerDistance;

			if(enemyDistance > secureDistance)
			{
				tmp = tmp->next;
				continue;
			}

			Vec2 enemiePos = NodetoVec2(node);

			offset.x += enemiePos.x; offset.y += enemiePos.y;

			enemiePos = World2Screen(enemiePos, playerPos);

			drawDebugCircle(enemiePos.x, enemiePos.y, secureDistance, 255, 0, 0);

			printf("[IA] Done with threats %d! add (%d, %d) to target.\n", node->nodeID, offset.x, offset.y);

			tmp = tmp->next;
		}

		target.x = offset.x;
		target.y = offset.y;

		Move(wsi, target);
		drawDebugLine(World2Screen(playerPos, playerPos), target, 0, 0, 255);

		printf("[IA] Done with threats, move to (%d, %d)\n", target.x, target.y);

		return;
	}

	if(NodeStack_length(splitTargets) > 0)
	{
		puts("[IA] We have some splitTargets !");

		Node* starget = NULL;
		double value = 0;
		tmp = splitTargets;
		while(tmp != NULL)
		{
			Node* node = tmp->node;
			if(node == NULL)
			{
				tmp = tmp->next;
				continue;
			}

			double dist = getDistance(node, player);
			double currVal = node->size / dist;

			if(currVal > value)
			{
				starget = node;
				value = currVal;
			}

			tmp = tmp->next;
		}

		target = NodetoVec2(starget);
		Move(wsi, target);
		Split(wsi);

		printf("[IA] Split to (%d, %d)\n", target.x, target.y);

		return;
	}

	tmp = viruses;
	while(tmp != NULL)
	{
		Node* virus = tmp->node;
		if(virus == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		double dangerDistance = virus->size + DANGER_DISTANCE;

		Vec2 virusPos = NodetoVec2(virus);
		virusPos = World2Screen(virusPos, playerPos);

		drawDebugCircle(virusPos.x, virusPos.y, dangerDistance, 255, 0, 0);

		tmp = tmp->next;
	}

	Node* foodToGo;
	float foodValue = 0;

	puts("[IA] find which food is the best !");

	tmp = foods;
	while(tmp != NULL)
	{
		Node* food = tmp->node;
		if(food == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		if(player_length == 16 && player->size > 150 && food->type == VIRUS)
		{
			foodToGo = food;
			tmp = NULL;
			continue;
		}

		double distance = getDistance(food, player);
		float val = food->size / distance;
		if(val > foodValue)
		{
			foodValue = val;
			foodToGo = food;
			printf("	[IA] %d (%d, %d) has %f value\n", food->nodeID, food->x, food->y, val);
		}

		tmp = tmp->next;
	}

	target = NodetoVec2(foodToGo);
	Move(wsi, target);
	drawDebugLine(World2Screen(playerPos, playerPos), World2Screen(target, playerPos), 0, 255, 0);

	printf("[IA] Done with foods, move to (%d, %d)\n\n\n", target.x, target.y);
}

void IAUpdate(struct lws *wsi)
{
	if(player == NULL)
	{
		MoveZero(wsi);
		return;
	}

	Vec2 playerPos; playerPos.x = player->x; playerPos.y = player->y;
	Vec2 playerPosScreen = World2Screen(playerPos, playerPos);

	if(split_timer > 0)
		split_timer--;

	NodeStack* avoids = NULL;
	Node* small = NULL;
	Node* split_ball = NULL;
	double small_dist = 0;
	unsigned int small_value = 0;

	unsigned int zoneVal = getFoodNum(nodes);

	unsigned int marge = player->size * 4;
	//drawDebugCircle(playerPosScreen.x, playerPosScreen.y, marge, 255, 200, 0);

	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		Node* node = tmp->node;
		if(node == NULL || player == node || node->size == 0 || player->size == 0 || (node->type == PLAYER && strcmp(node->name, BotName) == 0))
		{
			tmp = tmp->next;
			continue;
		}

		double dist = getDistance(player, node) - node->size;
		/*if(node->type == VIRUS)
		{
			if(player->size > node->size * 1.1f)
				small = node;
			//if(player->size > node->size && dist < node->size && player->size < 150)
			//	NodeStack_push(&avoids, node);
		}
		else*/ if(node->size / player->size > 1.3f && node->type != VIRUS)
		{
			marge = 1000;

			if(player_length > 2)
				marge = 1500;
			else if (node->size / player->size <= 3.5f && node->size / player->size > 1.5f)
                marge = 1200;

            if (dist < marge)
               	NodeStack_push(&avoids, node);
		}
		else if(player->size / node->size > 1.1f)
		{
			/* Enemy split */
			/*if(player_length < 4 && split_timer == 0 && player->size > 70 && dist < 5000 && player->size / 2.6 > node->size && player->size / 5 < node->size && node->type == PLAYER)
			{
				printf("Splitting\n");
				split_ball = node;
			}*/
			

			/* Always split */
			
			if(split_timer == 0 && player->size > 150 && player_length < 3)
			{
				printf("Splitting\n");
				split_ball = node;
			}
			

            if(getDistance(node, player) < 5000)
            {
            	if(node->type == VIRUS && player->size > 110)
            	{
            		small = node;
            	}
            	else if(small == NULL || pow(dist, 2) / node->size < small_dist)
            	{
            		//puts("Food");
            		if(small_value == 0)
            			small_value = node->size * 5;
            		else
            			small_value--;

            		if(small_value > 0)
            		{
            			small = node;
            			small_dist = pow(dist, 2) / node->size;
            		}
            	}
            }
		}
		tmp = tmp->next;
	}

	if(NodeStack_length(avoids) > 0)
	{
		///TODO fix avoid target
		
		Vec2 target = playerPos;
		NodeStack* tmp = avoids;
		while(tmp != NULL)
		{
			Node* node = tmp->node;

			Vec2 Line;
			Line.x = node->x;
			Line.y = node->y;

			Line = World2Screen(Line, playerPos);
			drawDebugLine(playerPosScreen, Line, 255, 0, 0);
			
			Vec2 offset;
			offset.x = player->x - node->x;
			offset.y = player->y - node->y;
			
			target.x += offset.x;
			target.y += offset.y;

			tmp = tmp->next;
		}

		Move(wsi, target);

		target = World2Screen(target, playerPos);
		drawDebugLine(playerPosScreen, target, 0, 0, 255);

		printf("Avoiding '%d' balls, goto(%u, %u)\n", NodeStack_length(avoids), target.x, target.y);
	}
	/*else if(zoneVal < 10)
	{
		ZONE PlayerZone = getZone(player);
		ZONE targetZone;
		if(PlayerZone == LEFT_UP)
			targetZone = RIGHT_DOWN;
		else if(PlayerZone == LEFT_DOWN)
			targetZone = RIGHT_UP;
		else if(PlayerZone == RIGHT_UP)
			targetZone = LEFT_DOWN;
		else
			targetZone = LEFT_UP;

		Move(wsi, gotoZone(targetZone));
		printf("Changing zone...\n");
	}*/
	else if(split_ball != NULL)
	{
		Vec2 target;
		target.x = split_ball->x;
		target.y = split_ball->y;

		Move(wsi, target);
		Split(wsi);
		printf("Splitting\n");
	}
	else if(small)
	{
		Vec2 target;
		target.x = small->x;
		target.y = small->y;

		Move(wsi, target);

		target = World2Screen(target, playerPos);
		drawDebugLine(playerPosScreen, target, 0, 255, 0);

		const char* name = "food";
		printf("Hunting %s (%d)\n", small->type==PLAYER ? small->name : name, small_value);
	}
	else
	{
		ZONE zone = getOppositeZone();
		Vec2 pos = gotoZone(zone);
		Move(wsi, pos);
		printf("Nothings, goto(%d, %d)\n", pos.x, pos.y);
	}
}

void AddNode(unsigned char* data)
{
	unsigned int id;
	memcpy(&id, data, sizeof(unsigned int));

	//Node* node = NodeStack_get(nodes, id);
	//if(node != NULL && NodeStack_find(playerNodes, node->nodeID))
	//	NodeStack_push(&playerNodes, node);

	newPlayerNodeId = id;
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
		NodeStack_clear(&nodes);
		break;

	case 20:
		//printf("Reset owned cells\n");
		NodeStack_clear(&playerNodes);
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
		//memcpy(map, payload+1, sizeof(Map));
		//player->x = map->right - map->left;
		//player->y = map->top / 2.f;
		//printf("PlayerPos(%f, %f)\n", player->x, player->y);
		//printf("MapPos(b:%f, t;%f, r;%f, l:%f)\n", map->bottom, map->top, map->right, map->left);
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

	//Loop(exit);
}
