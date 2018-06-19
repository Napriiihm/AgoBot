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
		node->isSafe = 1;

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
				if(player == NULL)
					player = node;
				else if(node->size > player->size)
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
	{
		MoveZero(wsi);
		return;
	}

	Vec2 playerPos; playerPos.x = player->x; playerPos.y = player->y;
	Vec2 playerPosScreen = World2Screen(playerPos, playerPos);

	if(split_timer > 0)
		split_timer--;

	NodeStack* avoids = NULL, *foods = NULL, *passiveThreats = NULL;
	Node* small = NULL;
	Node* split_ball;

	if(split_timer <= 0)
		split_ball = NULL;

	double small_dist = 0;
	unsigned int small_value = 0;

	unsigned int zoneVal = getFoodNum(nodes);

	unsigned int marge = DEFAULT_MARGE;

	puts("[IA] Looking around...");
	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		Node* node = tmp->node;
		if(node == NULL || player == node || node->size == 0 || player->size == 0 || (node->type == PLAYER && strcmp(node->name, BotName) == 0))
		{
			tmp = tmp->next;
			continue;
		}

		Vec2 nodePos = World2Screen(NodetoVec2(node), playerPos);
		double dist = getDistance(player, node);
		if(node->type == VIRUS)
		{
			if(player->size > 105 && player_length != 16)
			{
				NodeStack_push(&passiveThreats, node);
				drawDebugCircle(nodePos.x, nodePos.y, node->size - AVOID_VIRUS_DISTANCE, 255, 0, 0);
				if(dist < player->size + node->size - AVOID_VIRUS_DISTANCE)
					NodeStack_push(&avoids, node);
			}
		}
		else if(node->size > player->size)
		{
			marge = DEFAULT_MARGE + node->size;
			NodeStack_push(&passiveThreats, node);

			/*if(canSplit(node, player))
			{
				drawDebugCircle(nodePos.x, nodePos.y, splitDistance(node), 125, 255, 125);
				marge = splitDistance(node);
			}*/

            drawDebugCircle(nodePos.x, nodePos.y, marge, 255, 0, 0);

            if (dist - player->size < marge)
               	NodeStack_push(&avoids, node);
		}
		else if(player->size / node->size > 1.1f && node->type != VIRUS)
		{
			NodeStack_push(&foods, node);		
		}

		tmp = tmp->next;
	}
	puts("[IA] Figured out what's around !");

	tmp = passiveThreats;
	while(tmp != NULL)
	{
		Node* node = tmp->node;
		if(node == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		double dangerRadius = 0;
		if(node->type == VIRUS && player->size > 105 && player_length != 16)
			dangerRadius = node->size + AVOID_VIRUS_DISTANCE;
		else if(node->size / player->size > 1.1f)
		{
			if(canSplit(node, player))
				dangerRadius = splitDistance(node);
			else
				dangerRadius = DEFAULT_MARGE;
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

			double dist = getDistance(food, node);
			if(dist + player->size <= dangerRadius)
				food->isSafe = 0;

			tmp2 = tmp2->next;
		}

		tmp = tmp->next;
	}

	NodeStack* safeFoods = NULL;
	tmp = foods;
	while(tmp != NULL)
	{
		Node* food = tmp->node;
		if(food == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		Vec2 nodePos = World2Screen(NodetoVec2(food), playerPos);

		if(food->isSafe)
		{
			NodeStack_push(&safeFoods, food);
			drawDebugCircle(nodePos.x, nodePos.y, food->size / 2, 0, 255, 0);
		}
		else
			drawDebugCircle(nodePos.x, nodePos.y, food->size / 2, 255, 0, 0);

		tmp = tmp->next;
	}

	if(NodeStack_length(avoids) > 0)
	{
		puts("[IA] Warning, threats arround !");
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

			printf("[IA] Done with threats %d! add (%d, %d) to target.\n", node->nodeID&0xffff, offset.x, offset.y);

			tmp = tmp->next;
		}

		if(isNearWall(player))
			escapeWall(player, &target);

		Move(wsi, target);

		target = World2Screen(target, playerPos);
		drawDebugLine(playerPosScreen, target, 0, 0, 255);

		printf("[IA] Done with threats, move to (%d, %d)\n\n", target.x, target.y);

		return;
	}

	tmp = safeFoods;
	while(tmp != NULL)
	{
		Node* food = tmp->node;
		if(food == NULL)
		{
			tmp = tmp->next;
			continue;
		}		

		double dist = getDistance(food, player);

		if(player_length < 4 && split_timer == 0 && canSplit(player, food) && food->type == PLAYER && dist < splitDistance(player))
		{
			printf("Splitting\n");
			split_ball = food;
			split_timer = 6000000;
		}	

        if(dist < 5000)
        {
        	if(food->type == VIRUS && player->size > 110)
        	{
        		small = food;
        	}
        	else if(small == NULL || pow(dist, 2) / food->size < small_dist)
        	{
        		if(small_value == 0)
        			small_value = food->size * 5;
        		else
        			small_value--;

        		if(small_value > 0)
        		{
        			small = food;
        			small_dist = pow(dist, 2) / food->size;
        		}
        	}
        }

		tmp = tmp->next;
	}

	if(split_ball != NULL)
	{
		puts("[IA] We have some splitTargets !");

		Move(wsi, NodetoVec2(split_ball));
		Split(wsi);

		split_ball = NULL;
	}
	else if(small && small->type != VIRUS)
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
	puts("\n");
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

void IAV3(struct lws* wsi)
{
	if(player == NULL)
		return;

	Vec2 playerPos = NodetoVec2(player);
	Vec2f result; memset(&result, 0, sizeof(Vec2f));
	Vec2 ret; memset(&ret, 0, sizeof(Vec2));

	char split = 0;
	Node* split_target = NULL;
	NodeStack* threats = NULL;
	int split_timer = 0;

	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		Node* check = tmp->node;
		if(check == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		double influence = 0;
		if(check->type == PLAYER)
		{
			if(strcmp(check->name, BotName) == 0)
				influence = 0;
			else if(getMass(player) / 1.3f > getMass(check))
				influence = check->size * 2.5f;
			else if(getMass(check) / 1.3 > getMass(player))
				influence = -check->size;
		}
		else if(check->type == FOOD)
			influence = 1;
		else if(check->type == VIRUS)
		{
			if(getMass(player) / 1.3 > getMass(check))
			{
				if(player_length >= 16)
					influence = check->size * 2.5f;
				else 
					influence = -1;
			}
		}
		else
			influence = check->size;

		if(influence == 0 || (check->type == PLAYER && strcmp(check->name, BotName)))
		{
			tmp = tmp->next;
			continue;
		}

		Vec2 checkPos = NodetoVec2(check);
		Vec2 displacement;
		displacement.x = checkPos.x - playerPos.x;
		displacement.y = checkPos.y - playerPos.y;

		double distance = getDistance(check, player);
		if(influence < 0)
		{
			distance -= player->size + check->size;
			if(check->type == PLAYER)
				NodeStack_push(&threats, check);
		}

		if(distance < 1)
			distance = 1;

		influence /= distance;

		Vec2f force = Vec2_normalize(displacement);
		force.x *= influence;
		force.y *= influence;

		if(check->type == PLAYER && getMass(player) / 2.6f > getMass(check) && getMass(player) / 5.f < getMass(check)
			&& split == 0 && split_timer == 0 && player_length < 3)
		{
			double distSplit = max(splitDistance(player), player->size * 4);
			if(distance < distSplit - player->size - check->size)
			{
				split_target = check;
				split = 1;
			}
		}
		else
		{
			result.x += force.x;
			result.y += force.y;
		}

		tmp = tmp->next;
	}

	result = Vec2f_normalize(result);

	if(split)
	{
		if(NodeStack_length(threats) > 0)
		{
			;//TODO
		}
		else
		{
			ret.x = split_target->x;
			ret.y = split_target->y;
			split_timer = 16;
			Move(wsi, ret);
			Split(wsi);
			return;
		}
	}
	ret.x = playerPos.x + result.x * 800;
	ret.y = playerPos.y + result.y * 800;
	Move(wsi, ret);
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