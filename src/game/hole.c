#include <allegro.h>
#include "hole.h"

int vgolf_hole_editor_mode = 0;

int load_hole_fp(HOLE * hole, PACKFILE * in_file)
{
	int i,j,k, temp_w, temp_h;
	char temp_char;
        
	//read header..
	temp_char = pack_getc(in_file);
	if(temp_char != 'C') return 0;
	temp_char = pack_getc(in_file);
	if(temp_char != 'R') return 0;
	temp_char = pack_getc(in_file);
	if(temp_char != 'S') return 0;
	temp_char = pack_getc(in_file);
	if(temp_char != 36) return 0;

	for(i = 0; i < 128; i++) hole->name[i] = pack_getc(in_file);
	hole->par_score = pack_igetw(in_file);
	hole->num_polys = pack_igetw(in_file);
	if(hole->num_polys > MAX_HOLE_POLYS)
	{
		allegro_message("poly out of bounds");
		exit(0);
	}
	for(i = 0; i < hole->num_polys; i++)
	{
		hole->body[i].poly.num_vertices = pack_igetw(in_file);
		if(hole->body[i].poly.num_vertices > MAX_OBJECT_VERTICES)
		{
			allegro_message("vertex out of bounds");
			exit(0);
		}
		hole->body[i].poly.color = pack_igetw(in_file);
		for(j = 0; j < hole->body[i].poly.num_vertices; j++)
		{
			hole->body[i].poly.p[j].x = pack_igetw(in_file);
			if(pack_getc(in_file)) hole->body[i].poly.p[j].x *= -1;
			hole->body[i].poly.p[j].y = pack_igetw(in_file);
			if(pack_getc(in_file)) hole->body[i].poly.p[j].y *= -1;
			pack_igetw(in_file);
			pack_getc(in_file);
			if(j > 0)
			{ //do some correction so we don't get vertical lines..
				if(hole->body[i].poly.p[j-1].x == hole->body[i].poly.p[j].x)
				{
					hole->body[i].poly.p[j].x += 0.5;
				}
			}
		}
		hole->body[i].friction = 0;
		hole->body[i].restitution = hole->body[i].mass = hole->body[i].inv_mass = 0;
		hole->body[i].density = 1;
		hole->body[i].pos = hole->s;
		hole->body[i].vel.x = hole->body[i].vel.y = 0;
	}

	hole->num_verts = pack_igetw(in_file);
	for(i = 0; i < hole->num_verts; i++)
	{
		hole->vert[i].s.x = pack_igetw(in_file);
		if(pack_getc(in_file)) hole->vert[i].s.x *= -1;
		hole->vert[i].s.y = pack_igetw(in_file);
		if(pack_getc(in_file)) hole->vert[i].s.y *= -1;
		pack_igetw(in_file);
		pack_getc(in_file);
		hole->vert[i].flag = pack_igetw(in_file);
		hole->vert[i].radius = pack_igetw(in_file);
		hole->vert[i].num_refs = pack_igetw(in_file);
		for(j = 0; j < hole->vert[i].num_refs; j++)
		{
			hole->vert[i].ref_no[j] = pack_igetw(in_file);
		}
	}
	
	/* course editor shouldn't create new polys when loading */
	if(!vgolf_hole_editor_mode)
	{
		for(i = 0; i < hole->num_verts; i++)
		{
			/* generate laser poly */
			if(hole->vert[i].flag == VERT_FLAG_LASER_SEND)
			{
				hole->body[hole->num_polys].poly.num_vertices = 2;
				hole->body[hole->num_polys].poly.color = POLY_TYPE_EMPTY_WALL;
				hole->body[hole->num_polys].poly.p[0].x = hole->vert[i].s.x;
				hole->body[hole->num_polys].poly.p[0].y = hole->vert[i].s.y;
				hole->body[hole->num_polys].poly.p[1].x = hole->vert[hole->vert[i].ref_no[5]].s.x;
				hole->body[hole->num_polys].poly.p[1].y = hole->vert[hole->vert[i].ref_no[5]].s.y;
				hole->body[hole->num_polys].friction = 0;
				hole->body[hole->num_polys].restitution = hole->body[i].mass = hole->body[i].inv_mass = 0;
				hole->body[hole->num_polys].density = 1;
				hole->body[hole->num_polys].pos = hole->s;
				hole->body[hole->num_polys].vel.x = hole->body[i].vel.y = 0;
				hole->vert[i].ref_no[4] = hole->num_polys;
				hole->num_polys++;
			}
			
			/* fix conveyor polys */
			if(hole->vert[i].flag == VERT_FLAG_CONVEYOR)
			{
				hole->body[hole->vert[i].ref_no[5]].poly.color = POLY_TYPE_CONVEYOR_WALL;
			}
		}
	}
	
	/* certain ref_no[] need to be copied to new location on load (temporary, remove me after converting courses) */
/*	else
	{
		for(i = 0; i < hole->num_verts; i++)
		{
			if(hole->vert[i].flag == VERT_FLAG_POLY_DOOR_HORIZ || hole->vert[i].flag == VERT_FLAG_POLY_DOOR_VERT)
			{
				hole->vert[i].ref_no[6] = hole->vert[hole->vert[i].ref_no[4]].ref_no[4];
				hole->vert[i].ref_no[7] = hole->vert[hole->vert[i].ref_no[4]].ref_no[5];
				hole->vert[i].num_refs += 2;
			}
		}
	} */

	//load hole bitmap overlays (if present..)
	for(i = 0; i < MAX_HOLE_PIC_LAYERS; i++)
	{
		temp_w = pack_igetw(in_file);
		temp_h = pack_igetw(in_file);
		if(temp_w != 0 && temp_h != 0)
		{
			hole->layer_pic[i] = create_bitmap(temp_w, temp_h);
			if(!hole->layer_pic[i])
			{
				return 0;
			}
			for(j = 0; j < temp_h; j++)
			{
				for(k = 0; k < temp_w; k++)
				{
					putpixel(hole->layer_pic[i], k, j, pack_getc(in_file));
				}
			}
		}
		else
		{
			hole->layer_pic[i] = NULL;
		}
	}

	return 1;
}

int load_hole(HOLE * hole, char * filename)
{
	PACKFILE * in_file;

	in_file = pack_fopen(filename, "r");
	if(!in_file)
	{
		return 0;
	}
	load_hole_fp(hole, in_file);
	pack_fclose(in_file);
	return 1;
}

/* free hole image data */
void free_hole(HOLE * hole)
{
	int i;
	
	for(i = 0; i < MAX_HOLE_PIC_LAYERS; i++)
	{
		if(hole->layer_pic[i])
		{
			destroy_bitmap(hole->layer_pic[i]);
			hole->layer_pic[i] = NULL;
		}
	}
}

int save_hole_fp(HOLE * hole, PACKFILE * out_file)
{
	int i,j,k;
	
	//write header..
	pack_putc('C', out_file);
	pack_putc('R', out_file);
	pack_putc('S', out_file);
	pack_putc(36, out_file);
	
	for(i = 0; i < 128; i++) pack_putc(hole->name[i], out_file);
	pack_iputw(hole->par_score, out_file);
	pack_iputw(hole->num_polys, out_file);
	for(i = 0; i < hole->num_polys; i++)
	{
		pack_iputw(hole->body[i].poly.num_vertices, out_file);
		pack_iputw(hole->body[i].poly.color, out_file);
		for(j = 0; j < hole->body[i].poly.num_vertices; j++)
		{
			pack_iputw(abs((int)hole->body[i].poly.p[j].x), out_file);
			pack_putc((hole->body[i].poly.p[j].x < 0), out_file);
			pack_iputw(abs((int)hole->body[i].poly.p[j].y), out_file);
			pack_putc((hole->body[i].poly.p[j].y < 0), out_file);
			pack_iputw(0, out_file);
			pack_putc(0, out_file);
		}
	}

	pack_iputw(hole->num_verts, out_file);
	for(i = 0; i < hole->num_verts; i++)
	{
		pack_iputw(abs((int)hole->vert[i].s.x), out_file);
		pack_putc((hole->vert[i].s.x < 0), out_file);
		pack_iputw(abs((int)hole->vert[i].s.y), out_file);
		pack_putc((hole->vert[i].s.y < 0), out_file);
		pack_iputw(0, out_file);
		pack_putc(0, out_file);
		pack_iputw(hole->vert[i].flag, out_file);
		pack_iputw(hole->vert[i].radius, out_file);
		pack_iputw(hole->vert[i].num_refs, out_file);
		for(j = 0; j < hole->vert[i].num_refs; j++)
		{
			pack_iputw(hole->vert[i].ref_no[j], out_file);
		}
	}

	//save hole bitmap overlays (if present..)
	for(i = 0; i < MAX_HOLE_PIC_LAYERS; i++)
	{
		if(hole->layer_pic[i] == NULL)
		{
			pack_iputw(0, out_file);
			pack_iputw(0, out_file);
		}
		else
		{
			pack_iputw(hole->layer_pic[i]->w, out_file);
			pack_iputw(hole->layer_pic[i]->h, out_file);
			for(j = 0; j < hole->layer_pic[i]->h; j++)
			{
				for(k = 0; k < hole->layer_pic[i]->w; k++)
				{
					pack_putc(getpixel(hole->layer_pic[i], k, j), out_file);
				}
			}
		}
	}

	return 1;
}

int save_hole(HOLE * hole, char * filename){
        PACKFILE * out_file;

        out_file = pack_fopen(filename, "w");
        if(!out_file)
        {
	        return 0;
        }
        save_hole_fp(hole, out_file);
        pack_fclose(out_file);

        return 1;
}

//load the hole's coin placement data..
int load_hole_coindata(HOLE * hole, char * filename){
        int i, j;
        char temp_char;
        PACKFILE * in_file;

        in_file = pack_fopen(filename, "r");
        if(!in_file)
        {
	        return 0;
        }

        //read header..
        temp_char = pack_getc(in_file);
        if(temp_char != 'C') return -1;
        temp_char = pack_getc(in_file);
        if(temp_char != 'O') return -1;
        temp_char = pack_getc(in_file);
        if(temp_char != 'I') return -1;
        temp_char = pack_getc(in_file);
        if(temp_char != 36) return -1;

        hole->num_coins = pack_igetw(in_file);
        for(i = hole->num_verts; i < hole->num_verts+hole->num_coins; i++){
                hole->vert[i].s.x = pack_igetw(in_file);
                if(pack_getc(in_file)) hole->vert[i].s.x *= -1;
                hole->vert[i].s.y = pack_igetw(in_file);
                if(pack_getc(in_file)) hole->vert[i].s.y *= -1;
                pack_igetw(in_file);
                pack_getc(in_file);
                hole->vert[i].flag = pack_igetw(in_file);
                hole->vert[i].radius = pack_igetw(in_file);
                hole->vert[i].num_refs = pack_igetw(in_file);
                for(j = 0; j < hole->vert[i].num_refs; j++){
                        hole->vert[i].ref_no[j] = pack_igetw(in_file);
                        }
                }

        pack_fclose(in_file);

        return 1;
}

int save_hole_coindata(HOLE * hole, char * filename){
        int i, j;
        PACKFILE * out_file;

        out_file = pack_fopen(filename, "w");
        
        //write header..
        pack_putc('C', out_file);
        pack_putc('O', out_file);
        pack_putc('I', out_file);
        pack_putc(36, out_file);

        pack_iputw(hole->num_coins, out_file);
        for(i = 0; i < hole->num_coins; i++){
                pack_iputw(abs((int)hole->vert[i].s.x), out_file);
                pack_putc((hole->vert[i].s.x < 0), out_file);
                pack_iputw(abs((int)hole->vert[i].s.y), out_file);
                pack_putc((hole->vert[i].s.y < 0), out_file);
                pack_iputw(0, out_file);
                pack_putc(0, out_file);
                pack_iputw(VERT_FLAG_COIN_0, out_file);
                pack_iputw(hole->vert[i].radius, out_file);
                pack_iputw(hole->vert[i].num_refs, out_file);
                for(j = 0; j < hole->vert[i].num_refs; j++){
                        pack_iputw(hole->vert[i].ref_no[j], out_file);
                        }
                }

        pack_fclose(out_file);

        return 1;
}

