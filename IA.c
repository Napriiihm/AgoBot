#include "IA.h"

void IAInit()
{
	player = malloc(sizeof(Player));
	map = malloc(sizeof(Map));
}

void UpdateNodes(unsigned char* data)
{
	//printHex(data, 50);
	size_t NodeSize = 18;
	size_t totalNameLength = 0;

	//unsigned short deadLen = buffer[1] << 8 + buffer[2];
	unsigned short deadLen;
	memcpy(&deadLen, data, 2);
	
	unsigned int startNodePos = 2 + deadLen*2*sizeof(int);
	unsigned char end = data[startNodePos];

	int i = 0;
	while(end != 0)
	{
		char* pos = data + startNodePos + i*NodeSize + totalNameLength;
		Node* node = malloc(sizeof(Node));

		memcpy(node, pos, NodeSize);
		if(node->flags&0x8)
		{
			size_t nameLength = strlen(data + startNodePos + (i+1)*NodeSize + totalNameLength);
			node->name = malloc(nameLength);
			strcpy(node->name, data + startNodePos + (i+1)*NodeSize + totalNameLength);
			totalNameLength = nameLength+1;
			
			//printf("F : 0x%x, name = %s\n", node->flags, node->name);
			//printHex(pos, 0x30);
		}
		char* temp = "0";
		//printf("    Node : [id:%u x:%d y:%d size:%d F:0x%x R:0x%x G:0x%x B:0x%x N:%s]\n", node->nodeID, node->x, node->y, node->size & 0xFFFF, node->flags, node->R & 0xff, node->G & 0xff, node->B & 0xff, node->flags&8 ? node->name : temp);

		NodeStack_push(&nodes, node);

		end = data[startNodePos + (i+1)*(NodeSize) + totalNameLength];
		i++;
	}

	unsigned int new_pos = startNodePos + (i+1)*(NodeSize) + totalNameLength + 1;
	unsigned short nbDead;
	memcpy(&nbDead, data + new_pos, sizeof(unsigned short));
	for(int j = 0; j < nbDead; j++)
	{
		unsigned int nodeID;
		memcpy(&nodeID, data + new_pos + 4 + j * sizeof(unsigned int), sizeof(unsigned int));
		NodeStack_remove(&nodes, nodeID);
	}
	//puts("------------------------");
}

void Move(char** ret)
{
	*ret = malloc(13);
	*ret[0] = 0x10;

	Node* near = GetNearestFood(nodes, player);

	int x = near->x;
	int y = near->y;

	int t = 0;
	memcpy(*ret+1, &x, sizeof(int));
	memcpy(*ret+5, &y, sizeof(int));
	memcpy(*ret+9, &t, sizeof(int));
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
		memcpy(map, payload+1, sizeof(Map));
		player->x = map->bottom - map->top;
		player->y = map->right - map->left;
		printf("PlayerPos(%f, %f)\n", player->x, player->y);
		printf("MapPos(b:%f, t;%f, r;%f, l:%f)\n", map->bottom, map->top, map->right, map->left);
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