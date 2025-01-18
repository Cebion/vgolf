#include <allegro.h>
#include "avatar.h"
#include "../modules/fx.h"

VGOLF_AVATAR * vgolf_create_avatar(BITMAP * b1, BITMAP * b2, BITMAP * b3, BITMAP * b4)
{
	VGOLF_AVATAR * ap;
	
	ap = malloc(sizeof(VGOLF_AVATAR));
	if(!ap)
	{
		return NULL;
	}
	ap->image[0] = b1;
	ap->image[1] = b2;
	ap->image[2] = b3;
	ap->image[3] = b4;
	return ap;
}

VGOLF_AVATAR * vgolf_load_avatar(const char * fn, PALETTE palette)
{
	VGOLF_AVATAR * ap;
	BITMAP * bp;
	PALETTE pal;
	int i;
	
	bp = load_pcx(fn, pal);
	if(!bp)
	{
		return NULL;
	}
	ap = malloc(sizeof(VGOLF_AVATAR));
	if(!ap)
	{
		destroy_bitmap(bp);
		return NULL;
	}
	if(palette)
	{
		convert_palette(bp, pal, palette);
	}
	for(i = 0; i < VGOLF_AVATAR_MAX_IMAGES; i++)
	{
		ap->image[i] = create_bitmap(64, 58);
		clear_to_color(ap->image[i], 0);
		stretch_blit(bp, ap->image[i], (i * (bp->w / 4)), 0, bp->w / 4, bp->h, 0, 0, 64, 58);
	}
	destroy_bitmap(bp);
	return ap;
}

int vgolf_save_avatar(VGOLF_AVATAR * ap, PALETTE pal, const char * fn)
{
	BITMAP * bp;
	int i;
	
	bp = create_bitmap(ap->image[0]->w * 4, ap->image[0]->h);
	if(!bp)
	{
		return 0;
	}
	clear_to_color(bp, 0);
	for(i = 0; i < 4; i++)
	{
		if(ap->image[i])
		{
			draw_sprite(bp, ap->image[i], i * ap->image[0]->w, 0);
		}
	}
	save_pcx(fn, bp, pal);
	destroy_bitmap(bp);
	return 1;
}

void vgolf_destroy_avatar(VGOLF_AVATAR * ap)
{
	int i;
	
	for(i = 0; i < VGOLF_AVATAR_MAX_IMAGES; i++)
	{
		if(ap->image[i])
		{
			destroy_bitmap(ap->image[i]);
		}
	}
	free(ap);
}
