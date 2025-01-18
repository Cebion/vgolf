#ifndef FX_H
#define FX_H

#include <allegro.h>

/* '3d' sprite drawing routines */
int fx_3d_to_2d_x(int x, int y, int z, int ox, int oy, int space_x, int space_y);
int fx_3d_to_2d_y(int x, int y, int z, int ox, int oy, int space_x, int space_y);
void fx_draw_sprite_3d(BITMAP * dest, BITMAP * source, int x, int y, int z, int ox, int oy);

extern int fx_fade_finished;
extern int fx_fade_level;
extern int fx_fade_speed;

int closest_match(int color, PALETTE p1, PALETTE p2);
void convert_palette(BITMAP * bp, PALETTE p1, PALETTE p2);
BITMAP * turn_bitmap(BITMAP * bp, int dir);

/* passive fading routines */
void fx_fade_start(PALETTE p1, PALETTE p2, int speed, void(*proc)());
void fx_fade_logic(void);
void fx_fade_render(void);

/* particle effect routines */
#define FX_MAX_PARTICLES 1024

typedef struct
{
	
	fixed x, y, z;
	fixed vx, vy, vz;
	int active;
	BITMAP * bp;
	
	void(*logic_proc)();
	
} FX_PARTICLE;

void fx_particle_system_init(void);
void fx_particle_system_logic(void);
void fx_particle_system_render(BITMAP * bp);
int fx_particle_system_add_particle(BITMAP * bp, fixed x, fixed y, fixed z, fixed vx, fixed vy, fixed vz, void(*proc)(FX_PARTICLE * pp));
void fx_particle_system_set_proc(void(*proc)());
void fx_draw_3d_sprite(BITMAP * bp, BITMAP * sprite, int x, int y, int z, int ox, int oy);

#endif
