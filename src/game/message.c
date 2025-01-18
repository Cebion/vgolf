#include <string.h>
#include "message.h"

void outline_text(BITMAP * bp, FONT * fp, int x, int y, int c1, int c2, char * buffer)
{
	textprintf_ex(bp, fp, x - 1, y, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x + 1, y, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y - 1, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y + 1, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y, c1, -1, "%s", buffer);
}

void outline_text_shadow(BITMAP * bp, FONT * fp, int x, int y, int sx, int sy, int c1, int c2, int c3, char * buffer)
{
	textprintf_ex(bp, fp, x - 1 + sx, y + sy, c3, -1, "%s", buffer);
	textprintf_ex(bp, fp, x + 1 + sx, y + sy, c3, -1, "%s", buffer);
	textprintf_ex(bp, fp, x + sx, y - 1 + sy, c3, -1, "%s", buffer);
	textprintf_ex(bp, fp, x + sx, y + 1 + sy, c3, -1, "%s", buffer);
//	textprintf_ex(bp, fp, x, y, c1, -1, "%s", buffer);
	textprintf_ex(bp, fp, x - 1, y, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x + 1, y, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y - 1, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y + 1, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y, c1, -1, "%s", buffer);
}

void outline_text_shadow_center(BITMAP * bp, FONT * fp, int x, int y, int sx, int sy, int c1, int c2, int c3, char * buffer)
{
	textprintf_centre_ex(bp, fp, x - 1 + sx, y + sy, c3, -1, "%s", buffer);
	textprintf_centre_ex(bp, fp, x + 1 + sx, y + sy, c3, -1, "%s", buffer);
	textprintf_centre_ex(bp, fp, x + sx, y - 1 + sy, c3, -1, "%s", buffer);
	textprintf_centre_ex(bp, fp, x + sx, y + 1 + sy, c3, -1, "%s", buffer);
//	textprintf_ex(bp, fp, x, y, c1, -1, "%s", buffer);
	textprintf_centre_ex(bp, fp, x - 1, y, c2, -1, "%s", buffer);
	textprintf_centre_ex(bp, fp, x + 1, y, c2, -1, "%s", buffer);
	textprintf_centre_ex(bp, fp, x, y - 1, c2, -1, "%s", buffer);
	textprintf_centre_ex(bp, fp, x, y + 1, c2, -1, "%s", buffer);
	textprintf_centre_ex(bp, fp, x, y, c1, -1, "%s", buffer);
}

void shadow_sprite(BITMAP * bp, BITMAP * sprite, int x, int y, int sx, int sy, int sc)
{
	draw_character_ex(bp, sprite, x + sx, y + sy, sc, -1);
	draw_sprite(bp, sprite, x, y);
}

void ncdmsg_add(NCDMSG_QUEUE * qp, char * dp, int color, int time)
{
	
	if(qp->messages < NCDMSG_MAX)
	{
		strcpy(qp->message[qp->messages].text, dp);
		qp->message[qp->messages].time = time;
		qp->message[qp->messages].color = color;
		qp->messages++;
	}
}

void ncdmsg_logic(NCDMSG_QUEUE * qp)
{
	int i, j;
	
	for(i = 0; i < qp->messages; i++)
	{
		if(qp->message[i].time > 0)
		{
			qp->message[i].time--;
			if(qp->message[i].time <= 0)
			{
				for(j = i + 1; j < qp->messages; j++)
				{
					strcpy(qp->message[j - 1].text, qp->message[j].text);
					qp->message[j - 1].time = qp->message[j].time;
					qp->message[j - 1].color = qp->message[j].color;
				}
				qp->message[j].time = 0;
				qp->messages--;
			}
		}
	}
}
