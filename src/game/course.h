#ifndef VGOLF_COURSE_H
#define VGOLF_COURSE_H

#include <allegro.h>
#include "hole.h"

typedef struct
{
	
	BITMAP * vert_pic[MAX_COURSE_VERT_PIC_TYPES][MAX_VERT_ANIMS];
	BITMAP * animal_pic[MAX_COURSE_ANIMAL_TYPES][MAX_ANIMAL_ANIMS];
	BITMAP * object_pic[MAX_COURSE_OBJECT_TYPES][MAX_OBJECT_ANIMS];
	BITMAP * victory_bgpic;               //victory screen bg
	BITMAP * victory_podiumpic;           //victory screen podium
	BITMAP * victory_cloudpic[4];         //victory screen floater
	BITMAP * victory_trophypic[3];        //victory screen trophies (gold, silver, bronze)
	BITMAP * iconpic;              //the hole's icon
	BITMAP * coinpic[MAX_COIN_PICS];      //coins..

} COURSE_IMAGE_BANK;

typedef struct
{
	
	BITMAP * bgpic;                   //scoreboard background image
	BITMAP * gridpic;                 //image for displaying the total course strokes thusfar
	BITMAP * numpic[19];              //images of the numbers (0-18)
	
	int num_w, num_h;                 //width, height of the numbers
	int x,y;                          //screen coordinates for the scoreboard
	int menu_x[2], menu_y[2];         //screen coordinates for scoreboard menu items
	int grid_x[MAX_PLAYERS], grid_y[MAX_PLAYERS];  //screen coordinates for all players' grids
	int cname_x, cname_y;             //screen coordinates for displaying the course name
	
	short num_scoreboard_segs;        //# of "wood" segments on scoreboard
	short scoreboard_seg_count;       //# of activated segments on scoreboard currently
	short scoreboard_seg_counter;     //counter for having the segments gradually appear onscreen
	short scoreboard_seg_counter_end; //the ending value for the timer (increase for bigger delay)
	
} SCOREBOARD;

typedef struct
{
	
	HOLE current_hole;                //just keep the current hole in memory
	HOLE hole[18];
	COURSE_IMAGE_BANK image_bank;     //bank of all images used for course display
	SCOREBOARD scoreboard;            //course scoreboard
	PALETTE palette;
	char name[128];                   //course name
	char author[256];
	char comment[1024];
	char fname[1024];                   //course filename
	char fname_prefix[1024];            //course filename prefix
	int num_holes;                    //# holes in this course
	int empty_color;                  //color this course uses for filling empty screen space
	int course_par;                   //total course par score
	
} VGOLF_COURSE;

VGOLF_COURSE * vgolf_load_course_old(const char * filename);

VGOLF_COURSE * vgolf_load_course_info(const char * filename);
VGOLF_COURSE * vgolf_load_course(const char * filename);
int vgolf_save_course(VGOLF_COURSE * cr, const char * filename);
void vgolf_destroy_course(VGOLF_COURSE * cp);

#endif
