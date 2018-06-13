#include "IA.h"

t_packet* packetList = NULL; 

struct lws_protocols protocols[] = { { "ogar_protocol", callbackOgar, 0,	20 }, { NULL, NULL, 0, 0 } }; 

void sighandler(int sig)
{
	forceExit = 1;
}

t_packet *allocatePacket()
{
	t_packet *tmp;

	if ((tmp = malloc(sizeof(t_packet))) == NULL ) 
		return NULL;

	memset(tmp, 0, sizeof(t_packet));

	return tmp;
}

int sendCommand(struct lws *wsi, unsigned char *buf, unsigned int len)
{
	t_packet *tmp, *list = packetList;

	if (len > MAXLEN ) 
		return -1;

	if ((tmp = allocatePacket()) == NULL ) 
		return -1;

	memcpy(&(tmp->buf)[LWS_PRE], buf, len);
	tmp->len = len;

	if (packetList == NULL )
		packetList = tmp;
	else 
	{
		while (list && list->next)
			list = list->next;

		list->next = tmp;
	}

	lws_callback_on_writable(wsi);

	return 1;
}

int writePacket(struct lws *wsi)
{
	t_packet *tmp = packetList;
	int ret;

	if (packetList == NULL ) 
		return 0;

	packetList = tmp->next;

	ret = lws_write(wsi, &(tmp->buf)[LWS_PRE], tmp->len, LWS_WRITE_BINARY);
	free(tmp);
	lws_callback_on_writable(wsi);
	return(ret);
}

int callbackOgar(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	static unsigned int offset = 0;
	static unsigned char rbuf[MAXLEN];

	switch (reason) 
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		fprintf(stderr, "ogar: LWS_CALLBACK_CLIENT_ESTABLISHED\n");

		unsigned char connect[5] = {0xff, 0, 0, 0, 0};
		sendCommand(wsi, connect, 5);

		unsigned char start[5] = {0xfe, 13, 0, 0, 0};
		sendCommand(wsi, start, 5);

		char* name = getName();
		printf("Name: %s\n", name);
		unsigned int nameLength = strlen(name) + 1;
		char* namePacket = malloc(nameLength + 1);
		namePacket[0] = 0;
		memcpy(namePacket + 1, name, nameLength);
		sendCommand(wsi, namePacket, nameLength + 1);

		printf("Conection etablie !\n");

		break;

 	case LWS_CALLBACK_CLIENT_WRITEABLE:
		if (writePacket(wsi) < 0 ) 
			forceExit = 1;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		if (offset + len < MAXLEN ) 
		{
			memcpy(rbuf+offset, in, len);
			offset += len;

			if (lws_is_final_fragment(wsi)) 
			{
				IARecv(rbuf, &forceExit);

				Clear();

				//IAV3(wsi);
				//IAV2(wsi);
				IAUpdate(wsi);

				Loop(&forceExit);

				offset = 0;
			}
		} 
		else 
		{
			offset = MAXLEN;
		 	if (lws_is_final_fragment(wsi))
				offset = 0;
		}
		break;

	case LWS_CALLBACK_CLOSED:
		lwsl_notice("ogar: LWS_CALLBACK_CLOSED\n");
		forceExit = 1;
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("ogar: LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
		forceExit = 1;
		break;

	case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
		lwsl_err("ogar: LWS_CALLBACK_COMPLETED_CLIENT_HTTP\n");
		forceExit = 1;
		break;

	default:
		break;
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct lws_context_creation_info info;
	struct lws_client_connect_info i;

	struct lws_context *context;
	const char *protocol, *temp;

	memset(&info, 0, sizeof info);
	memset(&i, 0, sizeof(i));

	srand(time(NULL));

	signal(SIGINT, sighandler);

	i.port = 1443;
	i.origin = "agar.io";

	if(argc >= 2)
	{
		if(lws_parse_uri(argv[1], &protocol, &i.address, &i.port, &temp))
			;
	}
	else
	{
		if(lws_parse_uri("127.0.0.1", &protocol, &i.address, &i.port, &temp))
			;
	}

	i.ssl_connection = 0;
	i.host = i.address;
	i.ietf_version_or_minus_one = -1;
	i.client_exts = NULL;
	i.path = "/";

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	context = lws_create_context(&info);
	if (context == NULL) 
	{
		fprintf(stderr, "Creating libwebsocket context failed\n");
		return 1;
	}

	i.context = context;

	InitUI();

	const char* name = "AgoBot";
	IAInit(name);

	if (lws_client_connect_via_info(&i))
		;

	forceExit = 0;
	while (!forceExit)
		lws_service(context, 1000);

	lwsl_err("Exiting\n");
	lws_context_destroy(context);

	return 0;
}
