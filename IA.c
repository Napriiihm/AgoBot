#include "IA.h"

void IAInit(const char* name) { BotName = malloc(strlen(name)+1); strcpy(BotName, name); }
char* getName() { return BotName; }

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

		if(NodeStack_find(nodes, node->nodeID) == 0) //si on pas a deja cette cellule dans notre liste on arrete
			NodeStack_push(&nodes, node); //on ajoute cette cellule a notre list
		else
		{
			nodes = NodeStack_remove(nodes, node->nodeID);
			NodeStack_push(&nodes, node);
		}

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
		nodes = NodeStack_remove(nodes, nodeID); //on suprime de notre liste
	}
}

void MoveZero(struct lws* wsi)
{
	unsigned char* packet = malloc(13);
	memset(packet, 0, 13);
	*packet = 16;
	Vec2 pos;
	memset(&pos, 0, sizeof(Vec2));
	memcpy(packet+1, &pos, sizeof(pos));

	sendCommand(wsi, packet, 13);
}

void Move(struct lws *wsi, Vec2 pos)
{
	unsigned char* packet = malloc(13);
	memset(packet, 0, 13);
	*packet = 16;
	memcpy(packet+1, &pos, sizeof(pos));

	sendCommand(wsi, packet, 13);
}

void Split(struct lws *wsi)
{
	unsigned char* packet = malloc(1);
	packet[0] = 17;

	sendCommand(wsi, packet, 1);
}

void IAUpdate(struct lws *wsi)
{
	/*
	if(split_timer > 0)
		split_timer--;

	if(player == NULL)
	{
		MoveZero(wsi);
		return;
	}

	Vec2 playerPos;
	playerPos.x = player->x;
	playerPos.y = player->y;

	Vec2 result; memset(&result, 0, sizeof(Vec2));

	unsigned char split = 0;
	Node* splitTarget = NULL;
	NodeStack* threats;

	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		Node* check = tmp->node;
		if(check == NULL)
		{
			tmp = tmp->next;
			continue;
		}

		int influence = 0;
		if(check->type == PLAYER)
		{
			if(NodeStack_find(playerNodes, check->nodeID))
			{
				tmp = tmp->next;
				continue;
			}
			else if(player->size / 1.3f > check->size)
				influence = check->size * 2.5f;
			else if(check->size / 1.3f > player->size)
				influence = -check->size;
		}
		else if(check->type == FOOD)
			influence = 1;
		else if(check->type == VIRUS)
		{
			if(player->size / 1.3 > check->size)
			{
				if(player_length == 16)
					influence = check->size * 2.5;
				else
					influence = -1;
			}
		}
		else
			influence = check->size;

		if(influence == 0 || NodeStack_find(playerNodes, check->nodeID))
		{
			tmp = tmp->next;
			continue;
		}

		Vec2 checkPos;
		checkPos.x = check->x;
		checkPos.y = check->y;

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

		Vec2 force = normalize(displacement);
		force.x *= influence;
		force.y *= influence;

		if(check->type == PLAYER && player->size / 2.6 > check->size && player->size / 5 < check->size && !split && split_timer == 0 && player_length < 3)
		{
			double endDist = max(splitDistance(player), player->size * 4);

			if(distance < endDist - player->size - check->size)
			{
				splitTarget = check;
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

	result = normalize(result);

	if(split)
	{
		if(NodeStack_length(threats) > 0)
		{
			if(((Node*)NodeStack_getLargest(threats))->size / 2.6 > player->size)
			{
				Vec2 splitPos;
				splitPos.x = splitTarget->x;
				splitPos.y = splitTarget->y;
				Move(wsi, splitPos);
				split_timer = 16;
				Split(wsi);
				return;
			}
		}
		else
		{
			Vec2 splitPos;
			splitPos.x = splitTarget->x;
			splitPos.y = splitTarget->y;
			Move(wsi, splitPos);
			split_timer = 16;
			Split(wsi);
			return;
		}
	}

	Vec2 mvt;
	mvt.x = player->x + result.x * 800;
	mvt.y = player->y + result.y * 800;
	Move(wsi, mvt);
	*/
	
	if(player == NULL)
	{
		MoveZero(wsi);
		return;
	}

	if(split_timer > 0)
		split_timer--;

	NodeStack* avoids = NULL;
	Node* small = NULL;
	Node* split_ball = NULL;
	double small_dist = 0;
	unsigned int small_value = 0; 

	unsigned int zoneVal = getFoodNum(nodes);

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
		if(node->type == VIRUS)
		{
			if(player->size > node->size && dist < player->size + 10 && player_length > 4 && player_length < 8)
				NodeStack_push(&avoids, node);
		}
		else if(node->size / player->size > 1.1f)
		{
			unsigned int marge = 7000;

			if(player_length > 2)
				marge = 15000;
			else if (node->size / player->size <= 3.5f && node->size / player->size > 1.5f)
                marge = 10000;

            if (dist < marge)
               	NodeStack_push(&avoids, node);
		}
		else if(node->size / player->size <= 0.7f)
		{
			if(player_length < 3 && split_timer == 0 && player->size > 70 && dist < 5000 && player->size / 2.6 > node->size && player->size / 5 < node->size && node->type == PLAYER)
			{
				printf("Splitting\n");
				split_ball = node;
			}

            if(getDistance(node, player) < 5000)
            {
            	if(small == NULL || pow(dist, 2) / node->size < small_dist)
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
		
		Vec2 target;
		NodeStack* tmp = avoids;
		while(tmp != NULL)
		{
			Node* node = tmp->node;
			
			Vec2 offset;
			offset.x = player->x - node->x;
			offset.y = player->y - node->y;
			offset = normalize(offset);
			
			target.x += offset.x;
			target.y += offset.y;

			tmp = tmp->next;
		}

		target = normalize(target);

		Move(wsi, target);

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

		const char* name = "food";
		printf("Hunting %s (%d)\n", small->type==PLAYER ? small->name : name, small_value);
	}
	else
	{
		Vec2 zero;
		memset(&zero, 0, sizeof(Vec2));

		Move(wsi, zero);

		printf("Nothings\n");
	}
	
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
		break;

	case 21:
		//printf("Draw debug line\n");
		break;

	case 32:
		//printf("Owns blob\n");
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
		//printf("Unknown opcode : %x\n", opcode);
		break;
	}

	//Loop(exit);
}
