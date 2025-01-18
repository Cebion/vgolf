#ifndef VGOLF_AVATAR_H
#define VGOLF_AVATAR_H

#include <allegro.h>

#define VGOLF_MAX_AVATARS       256
#define VGOLF_AVATAR_MAX_IMAGES   4

typedef struct
{
	
	BITMAP * image[VGOLF_AVATAR_MAX_IMAGES];
	
} VGOLF_AVATAR;

VGOLF_AVATAR * vgolf_create_avatar(BITMAP * b1, BITMAP * b2, BITMAP * b3, BITMAP * b4);
VGOLF_AVATAR * vgolf_load_avatar(const char * fn, PALETTE palette);
int vgolf_save_avatar(VGOLF_AVATAR * ap, PALETTE pal, const char * fn);
void vgolf_destroy_avatar(VGOLF_AVATAR * ap);

#endif
