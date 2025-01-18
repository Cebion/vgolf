#ifndef CREDITS_H
#define CREDITS_H

#include <allegro.h>

#define MAX_CREDITS 256
#define MAX_CIMAGES  32

typedef struct
{
	
	char text[256];
	FONT * font;
	int oy, color;
	
} CREDIT_ITEM;

typedef struct
{
	
	int ox;
	int oy;
	BITMAP * bp;
	
} CREDIT_IMAGE;

typedef struct
{

	CREDIT_ITEM item[MAX_CREDITS];
	int y, vy;
	int items;
	
	CREDIT_IMAGE image[MAX_CIMAGES];
	int fade_level;
	int fade_dir;
	int image_ticks;
	int images;
	int current_image;
	
	int done;

} CREDITS;

void credits_add_item(CREDITS * cp, char * text, FONT * fp, int oy, int color);
void credits_add_image(CREDITS * cp, BITMAP * bp, int ox, int oy);
void credits_logic(CREDITS * cp);
void credits_render(BITMAP * bp, CREDITS * cp);

#endif
