#include <allegro.h>
#include "../modules/g-idle.h"
#include "fx.h"

PALETTE fx_source_palette;
PALETTE fx_destination_palette;
PALETTE fx_temp_palette;
int fx_fade_level;
int fx_fade_speed;
int fx_faded;
int fx_fade_finished;
void(*fx_fade_proc)();

int closest_match(int color, PALETTE p1, PALETTE p2)
{
    int j;
    int Closest;
    int Difference[3];
    int new_color = 0;

    /* figure out the color map */
    Closest = 1000;
    for(j = 1; j < 256; j++)
    {
        Difference[0] = p1[color].r - p2[j].r;
        if(Difference[0] < 0)
        {
            Difference[0] = -Difference[0];
        }
        Difference[1] = p1[color].g - p2[j].g;
        if(Difference[1] < 0)
        {
            Difference[1] = -Difference[1];
        }
        Difference[2] = p1[color].b - p2[j].b;
        if(Difference[2] < 0)
        {
            Difference[2] = -Difference[2];
        }
        if((Difference[0] + Difference[1] + Difference[2]) < Closest)
        {
            Closest = Difference[0] + Difference[1] + Difference[2];
            new_color = j;
        }
    }
    return new_color;
}

void convert_palette(BITMAP * bp, PALETTE p1, PALETTE p2)
{
    int i, j;
    int Closest;
    int Difference[3];
    unsigned char ColorMap[256] = {0};

    /* in case someone is using non-transparent black */
    int other_black = 0;
    int br = 255, bb = 255, bg = 255;

    /* figure out the color map */
    for(i = 1; i < 256; i++)
    {
        Closest = 1000;
        for(j = 1; j < 256; j++)
        {
            Difference[0] = p1[i].r - p2[j].r;
            if(Difference[0] < 0)
            {
                Difference[0] = -Difference[0];
            }
            Difference[1] = p1[i].g - p2[j].g;
            if(Difference[1] < 0)
            {
                Difference[1] = -Difference[1];
            }
            Difference[2] = p1[i].b - p2[j].b;
            if(Difference[2] < 0)
            {
                Difference[2] = -Difference[2];
            }
            if((Difference[0] + Difference[1] + Difference[2]) < Closest)
            {
                Closest = Difference[0] + Difference[1] + Difference[2];
                ColorMap[i] = j;
            }
        }
        /* map the "other" black */
        if(p2[i].r < br && p2[i].g < bg && p2[i].b < bb)
        {
            other_black = i;
            br = p2[i].r;
            bg = p2[i].g;
            bb = p2[i].b;
        }
    }

    /* map the colors to the picture */
    for(i = 0; i < bp->h; i++)
    {
        for(j = 0; j < bp->w; j++)
        {
            /* do this if non-transparent black */
            if(bp->line[i][j] != 0 && p1[bp->line[i][j]].r == 0 && p1[bp->line[i][j]].g == 0 && p1[bp->line[i][j]].b == 0)
            {
                bp->line[i][j] = other_black;
            }

            /* otherwise convert normally */
            else
            {
                bp->line[i][j] = ColorMap[bp->line[i][j]];
            }
        }
    }
}

BITMAP * turn_bitmap(BITMAP * bp, int dir)
{
	BITMAP * newbp;
	
	newbp = create_bitmap(bp->h, bp->w);
	if(!newbp)
	{
		return bp;
	}
	clear_bitmap(newbp);
	if(dir < 0)
	{
		rotate_sprite(newbp, bp, 0, 0, itofix(-64));
	}
	else
	{
		rotate_sprite(newbp, bp, 0, 0, itofix(64));
	}
	return newbp;
}

void fx_fade_start(PALETTE p1, PALETTE p2, int speed, void(*proc)())
{
	int i;
	
	for(i = 0; i < 256; i++)
	{
		fx_source_palette[i].r = p1[i].r;
		fx_source_palette[i].g = p1[i].g;
		fx_source_palette[i].b = p1[i].b;
		fx_destination_palette[i].r = p2[i].r;
		fx_destination_palette[i].g = p2[i].g;
		fx_destination_palette[i].b = p2[i].b;
	}
	fx_fade_proc = proc;
	fx_fade_level = 0;
	fx_fade_speed = speed;
	fx_faded = 1;
	fx_fade_finished = 0;
	set_palette(p1);
}

void fx_fade_logic(void)
{
	
	/* do this first, we want to make sure the last palette step
	   is drawn to the screen before executing the procdure */
	if(fx_fade_finished && fx_faded == 0)
	{
		if(fx_fade_proc)
		{
			fx_fade_proc();
		}
		fx_fade_finished = 0;
	}
	
	/* update if the fade level hasn't reached maximum */
	if(fx_fade_level < 64)
	{
		fx_fade_level += fx_fade_speed;
		fx_faded = 1;
		
		/* when we reach the maximum fade level, we want to ensure
		   the fx_fade_proc will be called next time we run the logic */
		if(fx_fade_level >= 64)
		{
			fx_fade_finished = 1;
			fx_fade_speed = 0;
		}
	}
}

void fx_fade_render(void)
{
	if(fx_faded)
	{
		fade_interpolate(fx_source_palette, fx_destination_palette, fx_temp_palette, fx_fade_level, 0, 255);
		#ifdef ALLEGRO_WINDOWS
			IdleUntilVSync();
		#endif
		set_palette(fx_temp_palette);
		#ifdef ALLEGRO_WINDOWS
			DoneVSync();
		#endif
		fx_faded = 0;
	}
}

void fx_draw_3d_sprite(BITMAP * bp, BITMAP * sprite, int x, int y, int z, int ox, int oy)
{
	float space_x, space_y;
	
	space_x = bp->w;
	space_y = bp->w;
	
	/* upper left and bottom right points in 3d */
//	float obj_x[2], obj_y[2], obj_z[2];
	int obj_x[2], obj_y[2], obj_z[2];
	
	/* upper left and bottom right points in 2d */
//	float screen_x[2], screen_y[2];
	int screen_x[2], screen_y[2];
	int screen_w, screen_h;

	obj_x[0] = x - ox;
	obj_x[0] *= space_x;
	obj_y[0] = y - oy;
	obj_y[0] *= space_y;
	obj_x[1] = x + sprite->w - ox;
	obj_x[1] *= space_x;
	obj_y[1] = y + sprite->h - oy;
	obj_y[1] *= space_y;
	obj_z[0] = z + bp->w;
	obj_z[1] = z + bp->w;
	
	/* clip sprites at z = 0 */
	if(obj_z[0] > 0)
	{
		screen_x[0] = obj_x[0] / obj_z[0] + ox;
		screen_x[1] = obj_x[1] / obj_z[0] + ox;
		screen_y[0] = obj_y[0] / obj_z[1] + oy;
		screen_y[1] = obj_y[1] / obj_z[1] + oy;
		screen_w = screen_x[1] - screen_x[0];
		screen_h = screen_y[1] - screen_y[0];
		masked_stretch_blit(sprite, bp, 0, 0, sprite->w, sprite->h, screen_x[0], screen_y[0], screen_w, screen_h);
	}
}

FX_PARTICLE fx_particle[FX_MAX_PARTICLES];
void(*fx_particle_end_proc)();

int fx_particle_system_qsort_helper(const void * e1, const void * e2)
{
    FX_PARTICLE * thing1 = (FX_PARTICLE *)e1;
    FX_PARTICLE * thing2 = (FX_PARTICLE *)e2;

    if(thing1->z > thing2->z)
	{
        return -1;
    }
    if(thing1->z < thing2->z)
    {
        return 1;
    }

    // they are equal...
    return 0;
}

void fx_particle_system_init(void)
{
	int i;

	for(i = 0; i < FX_MAX_PARTICLES; i++)
	{
		fx_particle[i].active = 0;
	}
}

void fx_particle_system_logic(void)
{
	int i;
	int count = 0;
	int a = 0;
	
	for(i = 0; i < FX_MAX_PARTICLES; i++)
	{
		if(fx_particle[i].active)
		{
			if(fx_particle[i].logic_proc)
			{
				fx_particle[i].logic_proc(&fx_particle[i]);
			}
			count++;
			a = 1;
		}
	}
	if(a && count <= 0 && fx_particle_end_proc)
	{
		fx_particle_end_proc();
	}
}

void fx_particle_system_render(BITMAP * bp)
{
	int i;
	
    qsort(fx_particle, FX_MAX_PARTICLES, sizeof(FX_PARTICLE), fx_particle_system_qsort_helper);
	for(i = 0; i < FX_MAX_PARTICLES; i++)
	{
		if(fx_particle[i].active)
		{
			if(fx_particle[i].bp)
			{
				fx_draw_3d_sprite(bp, fx_particle[i].bp, fixtoi(fx_particle[i].x), fixtoi(fx_particle[i].y), fixtoi(fx_particle[i].z), bp->w / 2, bp->h / 2);
			}
		}
	}
}

int fx_particle_system_add_particle(BITMAP * bp, fixed x, fixed y, fixed z, fixed vx, fixed vy, fixed vz, void(*proc)(FX_PARTICLE * pp))
{
	int i;
	
	for(i = 0; i < FX_MAX_PARTICLES; i++)
	{
		if(!fx_particle[i].active)
		{
			fx_particle[i].x = x;
			fx_particle[i].y = y;
			fx_particle[i].z = z;
			fx_particle[i].vx = vx;
			fx_particle[i].vy = vy;
			fx_particle[i].vz = vz;
			fx_particle[i].bp = bp;
			fx_particle[i].logic_proc = proc;
			return 1;
			break;
		}
	}
	return 0;
}

void fx_particle_system_set_proc(void(*proc)())
{
	fx_particle_end_proc = proc;
}
