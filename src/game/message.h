#ifndef MMESSAGE_H
#define MMESSAGE_H

#include <allegro.h>

#define NCDMSG_MAX 64

typedef struct
{
	
	char text[1024];
	int time;
	int color;
	
} NCDMSG_MESSAGE;

typedef struct
{

	NCDMSG_MESSAGE message[NCDMSG_MAX];
	int messages;
	
} NCDMSG_QUEUE;

void shadow_sprite(BITMAP * bp, BITMAP * sprite, int x, int y, int sx, int sy, int sc);
void outline_text(BITMAP * bp, FONT * fp, int x, int y, int c1, int c2, char * buffer);
void outline_text_shadow(BITMAP * bp, FONT * fp, int x, int y, int sx, int sy, int c1, int c2, int c3, char * buffer);
void outline_text_shadow_center(BITMAP * bp, FONT * fp, int x, int y, int sx, int sy, int c1, int c2, int c3, char * buffer);
void ncdmsg_add(NCDMSG_QUEUE * qp, char * dp, int color, int time);
void ncdmsg_logic(NCDMSG_QUEUE * qp);

#endif
