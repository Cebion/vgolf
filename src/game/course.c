#include <allegro.h>
#include <stdio.h>
#include "defines.h"
#include "course.h"
#include "hole.h"

static char fname[1024] = {0};

//returns the filename to the course datfile so we can load the file.. (ie. "vgolf.dat#blah.pcx")
//TODO: deallocate temp_str
static char * course_filename(const char * str, char * filename)
{
	strcpy(fname, str);
	strcat(fname, "#");
	strcat(fname, filename);
	
	return fname;
}

//give the hole some predefined values
static void preinitialize_hole(HOLE * hole)
{
	hole->s.x = 320;
	hole->s.y = 240;
	hole->deccel_factor = 0.05f;
}

static int save_pcx_cond(PACKFILE * fp, BITMAP * bp, PALETTE palette)
{
	if(bp)
	{
		pack_putc(1, fp);
		save_pcx_pf(fp, bp, palette);
	}
	else
	{
		pack_putc(0, fp);
	}
	return 1;
}

static BITMAP * load_pcx_cond(PACKFILE * fp, PALETTE palette)
{
	if(pack_getc(fp))
	{
		return load_pcx_pf(fp, palette);
	}
	return NULL;
}

//load the course info data (.crn)
int vgolf_load_course_info_old(VGOLF_COURSE * cr, const char * filename)
{
	int i;
	PACKFILE * in_file;

	in_file = pack_fopen(filename, "r");
	if(!in_file)
	{
		return 0;
	}

	for(i = 0; i < 128; i++) cr->name[i] = pack_getc(in_file);
	cr->course_par = pack_igetw(in_file);
	cr->num_holes = pack_igetw(in_file);
	cr->empty_color = pack_igetw(in_file);

	pack_fclose(in_file);
	return 1;
}

//load up relevant info so we can see the basics of the course in the course selection screen
int vgolf_load_course_preview(VGOLF_COURSE * cr, const char * filename)
{
	sprintf(fname, "%s#course_icon", filename);
	cr->image_bank.iconpic = load_pcx(fname, NULL);
	
	sprintf(fname, "%s#course_info", filename);
	
	//load the course info
	vgolf_load_course_info_old(cr, fname);
	
	return 1;
}

//load a course from file
VGOLF_COURSE * vgolf_load_course_old(const char * filename)
{
	VGOLF_COURSE * cr;
	char fname[1024];
	int i;

	cr = (VGOLF_COURSE *)malloc(sizeof(VGOLF_COURSE));
	if(!cr)
	{
		return NULL;
	}
	strcpy(cr->fname, filename);
	strcpy(cr->fname_prefix, filename);
	cr->fname_prefix[strlen(filename)] = '#';
	sprintf(fname, "%s#course_info", filename);

	//load the course info
	if(!vgolf_load_course_info_old(cr, fname))
	{
		return NULL;
	}
	
	destroy_bitmap(load_pcx(course_filename(filename, "cr->palette"), cr->palette));
	
	       //course preview icon
	cr->image_bank.iconpic = load_pcx(course_filename(filename, "course_icon"), NULL);
	if(!cr->image_bank.iconpic)
	{
//		return NULL;
	}

	//tee
	cr->image_bank.vert_pic[0][0] = load_pcx(course_filename(filename, "vpic0000_pcx"), NULL);
	if(!cr->image_bank.vert_pic[0][0])
	{
//		return NULL;
	}

	//hole
	cr->image_bank.vert_pic[1][0] = load_pcx(course_filename(filename, "vpic0100_pcx"), NULL);
	if(!cr->image_bank.vert_pic[1][0])
	{
//		return 0;
	}
	cr->image_bank.vert_pic[1][1] = load_pcx(course_filename(filename, "vpic0101_pcx"), NULL);
	if(!cr->image_bank.vert_pic[1][1])
	{
//		return 0;
	}

	//trees
	cr->image_bank.object_pic[0][0] = load_pcx(course_filename(filename, "obj00-00_pcx"), NULL);
	if(!cr->image_bank.object_pic[0][0])
	{
//		return 0;
	}
	cr->image_bank.object_pic[1][0] = load_pcx(course_filename(filename, "obj01-00_pcx"), NULL);
	if(!cr->image_bank.object_pic[1][0])
	{
//		return 0;
	}

	//laser/wall
	
	/*
	cr->image_bank.object_pic[2][0] = load_pcx(course_filename(filename, "obj02-00_pcx"), NULL);
	if(!cr->image_bank.object_pic[2][0])
	{
		return 0;
	}
	cr->image_bank.object_pic[2][1] = load_pcx(course_filename(filename, "obj02-01_pcx"), NULL);
	if(!cr->image_bank.object_pic[2][1])
	{
		return 0;
	}
	*/

	//teleport pad
	
	/*
	cr->image_bank.object_pic[3][0] = load_pcx(course_filename(filename, "obj03-00_pcx"), NULL);
	if(!cr->image_bank.object_pic[3][0])
	{
		return 0;
	}
	cr->image_bank.object_pic[3][1] = load_pcx(course_filename(filename, "obj03-01_pcx"), NULL);
	if(!cr->image_bank.object_pic[3][1])
	{
		return 0;
	}
	*/

	//conveyor belt
	
	/*
	cr->image_bank.object_pic[4][0] = load_pcx(course_filename(filename, "obj04-00_pcx"), NULL);
	if(!cr->image_bank.object_pic[4][0])
	{
		return 0;
	}
	cr->image_bank.object_pic[4][1] = load_pcx(course_filename(filename, "obj04-01_pcx"), NULL);
	if(!cr->image_bank.object_pic[4][1])
	{
		return 0;
	}
	cr->image_bank.object_pic[4][2] = load_pcx(course_filename(filename, "obj04-02_pcx"), NULL);
	if(!cr->image_bank.object_pic[4][2])
	{
		return 0;
	}
	cr->image_bank.object_pic[4][3] = load_pcx(course_filename(filename, "obj04-03_pcx"), NULL);
	if(!cr->image_bank.object_pic[4][3])
	{
		return 0;
	}
	*/

/* [[new stuff */
	//laser/wall
	cr->image_bank.object_pic[2][0] = load_pcx(course_filename(filename, "obj02-00_pcx"), NULL);
	if(!cr->image_bank.object_pic[2][0])
	{
                //return 0;
	}
	cr->image_bank.object_pic[2][1] = load_pcx(course_filename(filename, "obj02-01_pcx"), NULL);
	if(!cr->image_bank.object_pic[2][1])
	{
                //return 0;
	}

	//teleport pad
	cr->image_bank.object_pic[3][0] = load_pcx(course_filename(filename, "obj03-00_pcx"), NULL);
	if(!cr->image_bank.object_pic[3][0])
	{
                //return 0;
	}
	cr->image_bank.object_pic[3][1] = load_pcx(course_filename(filename, "obj03-01_pcx"), NULL);
	if(!cr->image_bank.object_pic[3][1])
	{
                //return 0;
	}

	//conveyor belt
	cr->image_bank.object_pic[4][0] = load_pcx(course_filename(filename, "obj04-00_pcx"), NULL);
	if(!cr->image_bank.object_pic[4][0])
	{
                //return 0;
	}
	cr->image_bank.object_pic[4][1] = load_pcx(course_filename(filename, "obj04-01_pcx"), NULL);
	if(!cr->image_bank.object_pic[4][1])
	{
                //return 0;
	}
	cr->image_bank.object_pic[4][2] = load_pcx(course_filename(filename, "obj04-02_pcx"), NULL);
	if(!cr->image_bank.object_pic[4][2])
	{
                //return 0;
	}
	cr->image_bank.object_pic[4][3] = load_pcx(course_filename(filename, "obj04-03_pcx"), NULL);
	if(!cr->image_bank.object_pic[4][3])
	{
                //return 0;
	}
/* new stuff]] (enabled loading of space course images..)*/

	//animals
	cr->image_bank.animal_pic[0][0] = load_pcx(course_filename(filename, "an0000_pcx"), cr->palette);
	if(!cr->image_bank.animal_pic[0][0])
	{
//		return 0;
	}
	cr->image_bank.animal_pic[0][1] = load_pcx(course_filename(filename, "an0001_pcx"), NULL);
	if(!cr->image_bank.animal_pic[0][1])
	{
//		return 0;
	}
	cr->image_bank.animal_pic[0][2] = load_pcx(course_filename(filename, "an0002_pcx"), NULL);
	if(!cr->image_bank.animal_pic[0][2])
	{
//		return 0;
	}
	cr->image_bank.animal_pic[0][3] = load_pcx(course_filename(filename, "an0003_pcx"), NULL);
	if(!cr->image_bank.animal_pic[0][3])
	{
//		return 0;
	}
	cr->image_bank.animal_pic[1][0] = load_pcx(course_filename(filename, "an0100_pcx"), NULL);
	if(!cr->image_bank.animal_pic[1][0])
	{
//		return 0;
	}
	cr->image_bank.animal_pic[1][1] = load_pcx(course_filename(filename, "an0101_pcx"), NULL);
	if(!cr->image_bank.animal_pic[1][1])
	{
//		return 0;
	}
	cr->image_bank.animal_pic[1][2] = load_pcx(course_filename(filename, "an0102_pcx"), NULL);
	if(!cr->image_bank.animal_pic[1][2])
	{
//		return 0;
	}
	cr->image_bank.animal_pic[1][3] = load_pcx(course_filename(filename, "an0103_pcx"), NULL);
	if(!cr->image_bank.animal_pic[1][3])
	{
//		return 0;
	}

	//load victory screen images
	cr->image_bank.victory_bgpic = load_pcx(course_filename(filename, "victbg_pcx"), NULL);
	if(!cr->image_bank.victory_bgpic)
	{
//		return 0;
	}
	cr->image_bank.victory_podiumpic = load_pcx(course_filename(filename, "victpodm_pcx"), NULL);
	if(!cr->image_bank.victory_podiumpic)
	{
//		return 0;
	}
	cr->image_bank.victory_cloudpic[0] = load_pcx(course_filename(filename, "victcld0_pcx"), NULL);
	if(!cr->image_bank.victory_cloudpic[0])
	{
//		return 0;
	}
	cr->image_bank.victory_cloudpic[1] = load_pcx(course_filename(filename, "victcld1_pcx"), NULL);
	if(!cr->image_bank.victory_cloudpic[1])
	{
//		return 0;
	}
	cr->image_bank.victory_trophypic[0] = load_pcx(course_filename(filename, "victtr00_pcx"), NULL);
	if(!cr->image_bank.victory_trophypic[0])
	{
//		return 0;
	}
	cr->image_bank.victory_trophypic[1] = load_pcx(course_filename(filename, "victtr01_pcx"), NULL);
	if(!cr->image_bank.victory_trophypic[1])
	{
//		return 0;
	}
	cr->image_bank.victory_trophypic[2] = load_pcx(course_filename(filename, "victtr02_pcx"), NULL);
	if(!cr->image_bank.victory_trophypic[2])
	{
//		return 0;
	}

	//load coin images
	cr->image_bank.coinpic[0] = load_pcx(course_filename(filename, "coin00_pcx"), NULL);
	if(!cr->image_bank.coinpic[0])
	{
//		return 0;
	}
	cr->image_bank.coinpic[1] = load_pcx(course_filename(filename, "coin01_pcx"), NULL);
	if(!cr->image_bank.coinpic[1])
	{
//		return 0;
	}
	cr->image_bank.coinpic[2] = load_pcx(course_filename(filename, "coin02_pcx"), NULL);
	if(!cr->image_bank.coinpic[2])
	{
//		return 0;
	}
	cr->image_bank.coinpic[3] = load_pcx(course_filename(filename, "coin03_pcx"), NULL);
	if(!cr->image_bank.coinpic[3])
	{
//		return 0;
	}

	//load scoreboard images
	//cr->scoreboard.bgpic = load_pcx(course_filename(filename, "scorebd_pcx"), NULL);
	cr->scoreboard.bgpic = load_pcx(course_filename(filename, "scrbdseg_pcx"), NULL);
	if(!cr->scoreboard.bgpic)
	{
//		return 0;
	}
	cr->scoreboard.gridpic = load_pcx(course_filename(filename, "scoregd_pcx"), NULL);
	cr->scoreboard.gridpic = NULL;
	if(!cr->scoreboard.gridpic)
	{
//		return 0;
	}

	//width and height of the numbers on the scoreboard
	cr->scoreboard.num_w = 17;
	cr->scoreboard.num_h = 17;
	//x,y values of the scoreboard for positioning on the screen
	cr->scoreboard.x = 0;
	cr->scoreboard.y = 0;
	//x,y offset values of the course name on the scoreboard
	cr->scoreboard.cname_x = 20;
	cr->scoreboard.cname_y = 15;
	//x,y offset values of the menu items on the scoreboard
	cr->scoreboard.menu_x[0] = 130;
	cr->scoreboard.menu_y[0] = 440;
	cr->scoreboard.menu_x[1] = 420;
	cr->scoreboard.menu_y[1] = 440;
	//x,y offset values of the grids on the scoreboard (player 1-4)
	cr->scoreboard.grid_x[0] = 20 + 30;
	cr->scoreboard.grid_y[0] = 50 + 8;
	cr->scoreboard.grid_x[1] = 20 + 320 + 10;
	cr->scoreboard.grid_y[1] = 50 + 8;
	cr->scoreboard.grid_x[2] = 20 + 30;
	cr->scoreboard.grid_y[2] = 250;
	cr->scoreboard.grid_x[3] = 20 + 320 + 10;
	cr->scoreboard.grid_y[3] = 250;
	//# of segments on scoreboard and # of active ones
	cr->scoreboard.num_scoreboard_segs = 10;
	cr->scoreboard.scoreboard_seg_count = 0;
	cr->scoreboard.scoreboard_seg_counter = 0;
	cr->scoreboard.scoreboard_seg_counter_end = 48;
	
	for(i = 0; i < cr->num_holes; i++)
	{
		sprintf(fname, "%s#hole%02d_hol", cr->fname, i);
		preinitialize_hole(&cr->hole[i]);
		if(!load_hole(&cr->hole[i], fname))
		{
			allegro_message("Unable to load hole!");
			return NULL;
		}
	}
	        
//	preinitialize_hole(&cr->current_hole);
        
	//successful
	return cr;
}

//try and free a bitmap from memory..
static int trash_bitmap(BITMAP * bp)
{
	if(bp != NULL)
	{
		destroy_bitmap(bp);
		return 1;
	}
	return 0;
}

VGOLF_COURSE * vgolf_load_course_info(const char * filename)
{
	VGOLF_COURSE * cr;
	PACKFILE * fp;
	char header[16] = {0};
	
	fp = pack_fopen(filename, "rp");
	if(!fp)
	{
		return 0;
	}
	pack_fread(header, 16, fp);
	if(stricmp(header, "vGolf_crs"))
	{
		return NULL;
	}
	
	cr = malloc(sizeof(VGOLF_COURSE));
	if(!cr)
	{
		return NULL;
	}
	memset(cr, 0, sizeof(VGOLF_COURSE));
	
	/* course info */
	pack_fread(cr->name, 128, fp);
	pack_fread(cr->author, 256, fp);
	pack_fread(cr->comment, 1024, fp);
	cr->num_holes = pack_igetl(fp);
	cr->course_par = pack_igetl(fp);
	cr->image_bank.iconpic = load_pcx_cond(fp, cr->palette);
	pack_fclose(fp);
	return cr;
}

VGOLF_COURSE * vgolf_load_course(const char * filename)
{
	VGOLF_COURSE * cr;
	PACKFILE * fp;
	char header[16] = {0};
	int i;
	
	fp = pack_fopen(filename, "rp");
	if(!fp)
	{
		return 0;
	}
	pack_fread(header, 16, fp);
	if(stricmp(header, "vGolf_crs"))
	{
		return NULL;
	}
	
	cr = malloc(sizeof(VGOLF_COURSE));
	if(!cr)
	{
		return NULL;
	}
	memset(cr, 0, sizeof(VGOLF_COURSE));
	
	/* course info */
	pack_fread(cr->name, 128, fp);
	pack_fread(cr->author, 256, fp);
	pack_fread(cr->comment, 1024, fp);
	cr->num_holes = pack_igetl(fp);
	cr->course_par = pack_igetl(fp);
	cr->image_bank.iconpic = load_pcx_cond(fp, cr->palette);
	
	/* course data */
	//tee
	cr->image_bank.vert_pic[0][0] = load_pcx_cond(fp, cr->palette);
	//hole
	cr->image_bank.vert_pic[1][0] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.vert_pic[1][1] = load_pcx_cond(fp, cr->palette);
	//trees
	cr->image_bank.object_pic[0][0] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.object_pic[1][0] = load_pcx_cond(fp, cr->palette);
	//laser/wall
	cr->image_bank.object_pic[2][0] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.object_pic[2][1] = load_pcx_cond(fp, cr->palette);
	//teleport pad
	cr->image_bank.object_pic[3][0] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.object_pic[3][1] = load_pcx_cond(fp, cr->palette);
	//conveyor belt
	cr->image_bank.object_pic[4][0] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.object_pic[4][1] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.object_pic[4][2] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.object_pic[4][3] = load_pcx_cond(fp, cr->palette);
	//animals
	cr->image_bank.animal_pic[0][0] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.animal_pic[0][1] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.animal_pic[0][2] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.animal_pic[0][3] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.animal_pic[1][0] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.animal_pic[1][1] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.animal_pic[1][2] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.animal_pic[1][3] = load_pcx_cond(fp, cr->palette);
	//load victory screen images
	cr->image_bank.victory_bgpic = load_pcx_cond(fp, cr->palette);
	cr->image_bank.victory_podiumpic = load_pcx_cond(fp, cr->palette);
	cr->image_bank.victory_cloudpic[0] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.victory_cloudpic[1] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.victory_trophypic[0] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.victory_trophypic[1] = load_pcx_cond(fp, cr->palette);
	cr->image_bank.victory_trophypic[2] = load_pcx_cond(fp, cr->palette);
	//load scoreboard images
	cr->scoreboard.bgpic = load_pcx_cond(fp, cr->palette);

	for(i = 0; i < cr->num_holes; i++)
	{
		preinitialize_hole(&cr->hole[i]);
		load_hole_fp(&cr->hole[i], fp);
	}
	pack_fclose(fp);
	
	//width and height of the numbers on the scoreboard
	cr->scoreboard.num_w = 17;
	cr->scoreboard.num_h = 17;
	//x,y values of the scoreboard for positioning on the screen
	cr->scoreboard.x = 0;
	cr->scoreboard.y = 0;
	//x,y offset values of the course name on the scoreboard
	cr->scoreboard.cname_x = 20;
	cr->scoreboard.cname_y = 15;
	//x,y offset values of the menu items on the scoreboard
	cr->scoreboard.menu_x[0] = 130;
	cr->scoreboard.menu_y[0] = 440;
	cr->scoreboard.menu_x[1] = 420;
	cr->scoreboard.menu_y[1] = 440;
	//x,y offset values of the grids on the scoreboard (player 1-4)
	cr->scoreboard.grid_x[0] = 20 + 30;
	cr->scoreboard.grid_y[0] = 50 + 8;
	cr->scoreboard.grid_x[1] = 20 + 320 + 10;
	cr->scoreboard.grid_y[1] = 50 + 8;
	cr->scoreboard.grid_x[2] = 20 + 30;
	cr->scoreboard.grid_y[2] = 250;
	cr->scoreboard.grid_x[3] = 20 + 320 + 10;
	cr->scoreboard.grid_y[3] = 250;
	//# of segments on scoreboard and # of active ones
	cr->scoreboard.num_scoreboard_segs = 10;
	cr->scoreboard.scoreboard_seg_count = 0;
	cr->scoreboard.scoreboard_seg_counter = 0;
	cr->scoreboard.scoreboard_seg_counter_end = 48;
	
	return cr;
}

int vgolf_save_course(VGOLF_COURSE * cr, const char * filename)
{
	PACKFILE * fp;
	char header[16] = "vGolf_crs";
	int i;
	
	fp = pack_fopen(filename, "wp");
	if(!fp)
	{
		return 0;
	}
	header[15] = 1;
	pack_fwrite(header, 16, fp);
	
	/* course info */
	pack_fwrite(cr->name, 128, fp);
	pack_fwrite(cr->author, 256, fp);
	pack_fwrite(cr->comment, 1024, fp);
	pack_iputl(cr->num_holes, fp);
	pack_iputl(cr->course_par, fp);
	save_pcx_cond(fp, cr->image_bank.iconpic, cr->palette);
	
	/* course data */
	//tee
	save_pcx_cond(fp, cr->image_bank.vert_pic[0][0], cr->palette);
	//hole
	save_pcx_cond(fp, cr->image_bank.vert_pic[1][0], cr->palette);
	save_pcx_cond(fp, cr->image_bank.vert_pic[1][1], cr->palette);
	//trees
	save_pcx_cond(fp, cr->image_bank.object_pic[0][0], cr->palette);
	save_pcx_cond(fp, cr->image_bank.object_pic[1][0], cr->palette);
	//laser/wall
	save_pcx_cond(fp, cr->image_bank.object_pic[2][0], cr->palette);
	save_pcx_cond(fp, cr->image_bank.object_pic[2][1], cr->palette);
	//teleport pad
	save_pcx_cond(fp, cr->image_bank.object_pic[3][0], cr->palette);
	save_pcx_cond(fp, cr->image_bank.object_pic[3][1], cr->palette);
	//conveyor belt
	save_pcx_cond(fp, cr->image_bank.object_pic[4][0], cr->palette);
	save_pcx_cond(fp, cr->image_bank.object_pic[4][1], cr->palette);
	save_pcx_cond(fp, cr->image_bank.object_pic[4][2], cr->palette);
	save_pcx_cond(fp, cr->image_bank.object_pic[4][3], cr->palette);
	//animals
	save_pcx_cond(fp, cr->image_bank.animal_pic[0][0], cr->palette);
	save_pcx_cond(fp, cr->image_bank.animal_pic[0][1], cr->palette);
	save_pcx_cond(fp, cr->image_bank.animal_pic[0][2], cr->palette);
	save_pcx_cond(fp, cr->image_bank.animal_pic[0][3], cr->palette);
	save_pcx_cond(fp, cr->image_bank.animal_pic[1][0], cr->palette);
	save_pcx_cond(fp, cr->image_bank.animal_pic[1][1], cr->palette);
	save_pcx_cond(fp, cr->image_bank.animal_pic[1][2], cr->palette);
	save_pcx_cond(fp, cr->image_bank.animal_pic[1][3], cr->palette);
	//load victory screen images
	save_pcx_cond(fp, cr->image_bank.victory_bgpic, cr->palette);
	save_pcx_cond(fp, cr->image_bank.victory_podiumpic, cr->palette);
	save_pcx_cond(fp, cr->image_bank.victory_cloudpic[0], cr->palette);
	save_pcx_cond(fp, cr->image_bank.victory_cloudpic[1], cr->palette);
	save_pcx_cond(fp, cr->image_bank.victory_trophypic[0], cr->palette);
	save_pcx_cond(fp, cr->image_bank.victory_trophypic[1], cr->palette);
	save_pcx_cond(fp, cr->image_bank.victory_trophypic[2], cr->palette);
	//load scoreboard images
	save_pcx_cond(fp, cr->scoreboard.bgpic, cr->palette);

	for(i = 0; i < cr->num_holes; i++)
	{
		save_hole_fp(&cr->hole[i], fp);
	}
	pack_fclose(fp);
	
	return 1;
}

void vgolf_destroy_course(VGOLF_COURSE * cr)
{
	int i;
	
	if(!cr)
	{
		return;
	}

	//course preview icon
	trash_bitmap(cr->image_bank.iconpic);
	//tee
	trash_bitmap(cr->image_bank.vert_pic[0][0]);
	//hole
	trash_bitmap(cr->image_bank.vert_pic[1][0]);
	trash_bitmap(cr->image_bank.vert_pic[1][1]);
	//trees
	trash_bitmap(cr->image_bank.object_pic[0][0]);
	trash_bitmap(cr->image_bank.object_pic[1][0]);
	//laser/wall
	trash_bitmap(cr->image_bank.object_pic[2][0]);
	trash_bitmap(cr->image_bank.object_pic[2][1]);
	//teleport pad
	trash_bitmap(cr->image_bank.object_pic[3][0]);
	trash_bitmap(cr->image_bank.object_pic[3][1]);
	//conveyor belt
	trash_bitmap(cr->image_bank.object_pic[4][0]);
	trash_bitmap(cr->image_bank.object_pic[4][1]);
	trash_bitmap(cr->image_bank.object_pic[4][2]);
	trash_bitmap(cr->image_bank.object_pic[4][3]);

	//animals
	trash_bitmap(cr->image_bank.animal_pic[0][0]);
	trash_bitmap(cr->image_bank.animal_pic[0][1]);
	trash_bitmap(cr->image_bank.animal_pic[0][2]);
	trash_bitmap(cr->image_bank.animal_pic[0][3]);
	trash_bitmap(cr->image_bank.animal_pic[1][0]);
	trash_bitmap(cr->image_bank.animal_pic[1][1]);
	trash_bitmap(cr->image_bank.animal_pic[1][2]);
	trash_bitmap(cr->image_bank.animal_pic[1][3]);

	//load victory screen images
	trash_bitmap(cr->image_bank.victory_bgpic);
	trash_bitmap(cr->image_bank.victory_podiumpic);
	trash_bitmap(cr->image_bank.victory_cloudpic[0]);
	trash_bitmap(cr->image_bank.victory_cloudpic[1]);
	trash_bitmap(cr->image_bank.victory_trophypic[0]);
	trash_bitmap(cr->image_bank.victory_trophypic[1]);
	trash_bitmap(cr->image_bank.victory_trophypic[2]);

	//load scoreboard images
	//cr->scoreboard.bgpic = load_pcx(course_filename(filename, "scorebd_pcx"), NULL);
	trash_bitmap(cr->scoreboard.bgpic);
	trash_bitmap(cr->scoreboard.gridpic);
	
	for(i = 0; i < cr->num_holes; i++)
	{
		free_hole(&cr->hole[i]);
	}
	free(cr);
}
