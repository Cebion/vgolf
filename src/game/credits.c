#include <allegro.h>
#include <string.h>
#include "credits.h"
#include "message.h"
#include "globals.h"

void credits_add_item(CREDITS * cp, char * text, FONT * fp, int oy, int color)
{
	strcpy(cp->item[cp->items].text, text);
	cp->item[cp->items].font = fp;
	cp->item[cp->items].oy = oy;
	cp->item[cp->items].color = color;
	cp->items++;
}

void credits_add_image(CREDITS * cp, BITMAP * bp, int ox, int oy)
{
	cp->image[cp->images].bp = bp;
	cp->image[cp->images].ox = ox;
	cp->image[cp->images].oy = oy;
	cp->images++;
	if(cp->images >= MAX_CIMAGES)
	{
		cp->images = MAX_CIMAGES - 1;
	}
}

void credits_logic(CREDITS * cp)
{
	cp->y -= cp->vy;
	if(cp->y == cp->image[cp->current_image].oy)
	{
		cp->fade_dir = 8;
		cp->fade_level = 0;
	}
	cp->fade_level += cp->fade_dir;
	if(cp->fade_level < 0)
	{
		cp->fade_dir = 0;
		cp->fade_level = 0;
		cp->current_image++;
		if(cp->current_image >= cp->images)
		{
			cp->current_image = cp->images - 1;
			cp->done = 1;
		}
	}
	if(cp->fade_level > 255)
	{
		cp->fade_dir = 0;
		cp->fade_level = 255;
		cp->image_ticks = 300;
	}
	if(cp->fade_level == 255)
	{
		cp->image_ticks--;
		if(cp->image_ticks <= 0)
		{
			cp->fade_dir = -8;
		}
	}
}

void credits_render(BITMAP * bp, CREDITS * cp)
{
	int i;
	
	color_map = &fade_map;
	draw_lit_sprite(bp, cp->image[cp->current_image].bp, cp->image[cp->current_image].ox - cp->image[cp->current_image].bp->w / 2, bp->h / 2 - cp->image[cp->current_image].bp->h / 2, cp->fade_level);
	for(i = 0; i < cp->items; i++)
	{
		outline_text_shadow(bp, cp->item[i].font, bp->w / 2 + 2, cp->y + cp->item[i].oy - 2, -2, 2, cp->item[i].color, makecol(0, 0, 0), makecol(0, 0, 0), cp->item[i].text);
//		textprintf_centre_ex(bp, cp->item[i].font, SCREEN_W / 2, cp->y + cp->item[i].oy, cp->item[i].color, -1, "%s", cp->item[i].text);
	}
}
