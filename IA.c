#include "IA.h"

void IAInit()
{
	player = malloc(sizeof(Player));
	map = malloc(sizeof(Map));
}

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
			size_t nameLength = strlen(pos + NodeSize); //taille du nom
			node->name = malloc(nameLength+1); //on aloue la memoire pour le nom
			strcpy(node->name, data + startNodePos + (i+1)*NodeSize + totalNameLength); //on copie le nom
			totalNameLength += nameLength+1;//on augment la taille total des noms
			if(strcmp(node->name, "AgoBot") == 0) //si la cellule est noter bot
			{
				player->x = (float)node->x; //la position de noter player est la position de cette cellule
				player->y = (float)node->y;
				player->size = node->size; //la taille de notre joueur est la taille de la cellule courrante
			}
		}
		/*
		char* temp = "0";
		printf("    Node : [id:%u x:%d y:%d size:%d F:0x%x R:0x%x G:0x%x B:0x%x N:%s]\n", node->nodeID, node->x, node->y, node->size & 0xFFFF, node->flags, node->R & 0xff, node->G & 0xff, node->B & 0xff, node->flags&8 ? node->name : temp);
		*/

		if(NodeStack_find(nodes, node->nodeID) == 0) //si on a pas deja cette cellule dans notre liste
			NodeStack_push(&nodes, node); //on ajoute cette cellule a notre list
		else
			free(node); //sinon on la suprime

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
		NodeStack_remove(&nodes, nodeID); //on suprime de notre liste
	}
}

void Move(char** ret)
{
	*ret = malloc(13);
	*ret[0] = 0x10;

	Node* near = GetNearestFood(nodes, player);
	if(near != NULL)
	{
		if(player->x == near->x && player->y == near->y)
		{
			NodeStack_remove(&nodes, near->nodeID);
			near = GetNearestFood(nodes, player);
		}

		int x = near->x;
		int y = near->y;

		int t = 0;
		memcpy(*ret+1, &x, sizeof(int));
		memcpy(*ret+5, &y, sizeof(int));
		memcpy(*ret+9, &t, sizeof(int));
	}
}

char* IAStep(unsigned char* payload)
{
	unsigned char opcode = payload[0];
	switch(opcode)
	{
	case 16:
		//printf("World Update Packet\n");
		UpdateNodes(payload+1);
		break;

	case 17:
		//printf("View Update\n");
		break;

	case 18:
		//printf("Reset all Cells\n");
		//NodeStack_clear(&nodes);
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
		memcpy(map, payload+1, sizeof(Map));
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

	char* ret;

	Move(&ret);

	return ret;
}
