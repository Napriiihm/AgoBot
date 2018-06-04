#include "IA.h"

void UpdateNodes(unsigned char* data)
{
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
			if(strcmp(node->name, "AgoBot") == 0) //si la cellule est noter bot
			{
				player = node;
				printf("PlayerPos (%d, %d)\n", player->x, player->y);
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
			free(node);

		memcpy(&end, data + startNodePos + (i+1)*(NodeSize) + totalNameLength, sizeof(unsigned int)); //la nouvelle fin (check si c'est 0)
		i++;
	}

	unsigned int new_pos = startNodePos + i*(NodeSize) + totalNameLength + sizeof(unsigned int); //nouvelle pos aprés avoir lu les cellules

	unsigned short nbDead; //nombre de cellule morte depuis la derniére fois
	memcpy(&nbDead, data + new_pos, sizeof(unsigned short)); //copie

	for(int j = 0; j < nbDead; j++) //pour chaque cellule morte
	{
		printf("Dell\n");
		unsigned int nodeID;
		memcpy(&nodeID, data + new_pos + sizeof(unsigned short) + j * sizeof(unsigned int), sizeof(unsigned int)); //on prend l'id
		nodes = NodeStack_remove(nodes, nodeID); //on suprime de notre liste
	}
}

void Move(unsigned char** packet, Vec2 pos)
{
	*packet = malloc(13);
	memset(*packet, 0, 13);
	**packet = 16;
	memcpy((*packet)+1, &pos, sizeof(pos));
}

unsigned char* IAUpdate()
{
	unsigned char* ret = NULL;

	NodeStack* avoids = NULL;
	Node* small = NULL;
	double small_dist = 0;
	unsigned int small_value = 0; 

	NodeStack* tmp = nodes;
	while(tmp != NULL)
	{
		Node* node = tmp->node;
		if(node == NULL)
			continue;

		double dist = getDistance(player, node) - node->size;
		if(node->type == VIRUS)
		{
			if(player->size > node->size && dist < (player->size - 15))
				NodeStack_push(&avoids, node);
		}
		else if(node->size / player->size > 1.1f)
		{
			unsigned int marge = 200;

			if (node->size / player->size <= 3.5f && node->size / player->size > 1.5f)
                marge = 700;

            if (dist < marge)
                NodeStack_push(&avoids, node);
		}
		else if(node->size / player->size <= 0.8f)
		{
			/*
			if (client.my_balls.length < 4 && split_timer == 0 &&
                    ball.size > 40 &&
                    ball.size / my_ball.size < 0.5 &&
                    ball.size / my_ball.size > 0.1 &&
                    dist < 500 && dist > 200) {
                    split_ball = ball;
                }
            */
			printf("Dist = %f < %d\n", getDistance(node, player), 5000);
            if(getDistance(node, player) < 5000)
            {
            	if(small == NULL || pow(dist, 2) / node->size < small_dist)
            	{
            		puts("Food");
            		if(small_value == 0)
            			small_value = node->size;
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
		Move(&ret, target);
		printf("Avoiding '%d' balls\n", NodeStack_length(avoids));
	}
	else if(small)
	{
		Vec2 target;
		target.x = small->x;
		target.y = small->y;
		Move(&ret, target);
		const char* name = "food";
		printf("Hunting %s (%d)\n", small->type==PLAYER ? small->name : name, small_value);
	}
	else
	{
		Vec2 zero;
		memset(&zero, 0, sizeof(Vec2));
		Move(&ret, zero);
		printf("Nothings\n");
	}
	NodeStack_clear(&avoids);
	return ret;
}

void IARecv(unsigned char* payload)
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
}
