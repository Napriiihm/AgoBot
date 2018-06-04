#include "WS.h"
#include "IA.h"

// compile with gcc -Wall -g -o sock ./test-client.c -lwebsockets


// =====================================================================================================================================
//	Start of function definition
// =====================================================================================================================================

// Caught on CTRL C
void sighandler(int sig)
{
	forceExit = 1;
}

/**
\brief Allocate a packet structure and initialise it.
\param none
\return pointer to new allocated packet
****************************************************************************************************************************/
t_packet *allocatePacket()
{
	t_packet *tmp;

	if ((tmp=malloc(sizeof(t_packet))) == NULL ) return NULL;
	memset(tmp,0,sizeof(t_packet));
	return tmp;
}

/**
\brief Add a packet to the list of packet to be sent
\param wsi websocket descriptor
\param buf buffer to be sent
\param len length of packet
\return pointer to new allocated packet
****************************************************************************************************************************/
int sendCommand(struct lws *wsi,unsigned char *buf,unsigned int len)
{
	t_packet *tmp,*list=packetList;

	if (len > MAXLEN ) return -1;
	if ((tmp=allocatePacket()) == NULL ) return -1;
	memcpy(&(tmp->buf)[LWS_PRE],buf,len);
	tmp->len=len;
	if (packetList == NULL )
		packetList=tmp;
	else {
		while (list && list->next) {
			list=list->next;
		}
		list->next=tmp;
	}
	lws_callback_on_writable(wsi);
	return 1;
}


/****************************************************************************************************************************/
int writePacket(struct lws *wsi)
{
	t_packet *tmp=packetList;
	int ret;

	if (packetList == NULL ) return 0;

	packetList=tmp->next;
	ret=lws_write(wsi,&(tmp->buf)[LWS_PRE],tmp->len,LWS_WRITE_BINARY);
	free(tmp);
	lws_callback_on_writable(wsi);
	return(ret);
}

/****************************************************************************************************************************/

static int callbackOgar(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	static unsigned int offset=0;
	static unsigned char rbuf[MAXLEN];

	switch (reason) {
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		fprintf(stderr, "ogar: LWS_CALLBACK_CLIENT_ESTABLISHED\n");

		unsigned char connect[5] = {0xff, 0, 0, 0, 0};
		sendCommand(wsi, connect, 5);

		unsigned char start[5] = {0xfe, 13, 0, 0, 0};
		sendCommand(wsi, start, 5);

		char* name = "\0AgoBot\0";
		sendCommand(wsi, name, 8);

		printf("Conection etablie !\n");

		break;

 	case LWS_CALLBACK_CLIENT_WRITEABLE:
		if (writePacket(wsi) < 0 ) forceExit=1;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		// we have receive some data, check if it can be written in static allocated buffer (length)

		if (offset + len < MAXLEN ) {
			memcpy(rbuf+offset,in,len);
			offset+=len;
			// we have receive some data, check with websocket API if this is a final fragment
			if (lws_is_final_fragment(wsi)) {
				// call recv function here

				//printHex(rbuf, offset);

				char* send = IAStep(rbuf);

				sendCommand(wsi, send, 13);
				//printHex(send, 13);

				offset=0;
			}
		} else {	// length is too long... get others but ignore them...
			offset=MAXLEN;
		 	if ( lws_is_final_fragment(wsi) ) {
				offset=0;
			}
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

/****************************************************************************************************************************/
int main(int argc, char **argv)
{
	struct lws_context_creation_info info;
	struct lws_client_connect_info i;

	struct lws_context *context;
	const char *protocol,*temp;

	memset(&info, 0, sizeof info);
	memset(&i, 0, sizeof(i));

	srand(time(NULL));

	signal(SIGINT, sighandler);

	i.port = 1443;
	i.origin = "agar.io";

	lws_parse_uri("127.0.0.1", &protocol, &i.address, &i.port, &temp);

	i.ssl_connection = 0;
	i.host = i.address;
	i.ietf_version_or_minus_one = -1;
	i.client_exts = NULL;
	i.path="/";

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	context = lws_create_context(&info);
	if (context == NULL) {
		fprintf(stderr, "Creating libwebsocket context failed\n");
		return 1;
	}

	i.context = context;

	if (lws_client_connect_via_info(&i)); // just to prevent warning !!

	IAInit();

	forceExit=0;
	// the main magic here !!
	while (!forceExit) {
		lws_service(context, 1000);
	}
	// if there is some errors, we just quit
	lwsl_err("Exiting\n");
	lws_context_destroy(context);

	return 0;
}
