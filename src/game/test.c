#define NCDNET_PRE_ALLEGRO

#include <joynet.h>
#include "ncdnet.h"

JOYNET_CLIENT * client = NULL;
int listed = 0;

int main(int argc, char * argv[])
{
	joynet_use_console = 1;
	if(!joynet_init())
	{
		printf("Error!\n");
		return 0;
	}
	ncdnet_install();
	client = joynet_create_client();
	if(!client)
	{
		printf("Error!\n");
		return 0;
	}
	joynet_connect(client, "127.0.0.1", 5555);
	while(!kbhit())
	{
		joynet_poll_client(client);
		if(client->state == CLIENT_STATE_CONNECTED && !listed)
		{
			sprintf(joynet_internal_buffer, "\1Test Server");
			joynet_add_message(client->outbox, NCDNET_MESSAGE_START_SERVER, NULL, joynet_internal_buffer, 0);
			listed = 1;
		}
		Sleep(50);
	}
	getch();
	joynet_connect(client, "127.0.0.1", 5555);
	while(1)
	{
		if(client->state == CLIENT_STATE_CONNECTED)
		{
			joynet_add_message(client->outbox, NCDNET_MESSAGE_CLOSE_SERVER, NULL, joynet_internal_buffer, 0);
			break;
		}
		joynet_poll_client(client);
		Sleep(50);
	}
	while(client->state != CLIENT_STATE_NOT_CONNECTED)
	{
		joynet_poll_client(client);
		Sleep(50);
	}
	return 0;
}
