#include <allegro.h>
#include "../modules/wfsel.h"
#include "../modules/g-idle.h"
#include "../modules/ncdgui.h"
#include "../modules/gametime.h"
#include "../modules/fx.h"
#include "../game/course.h"
#include "../game/body.h"
#include "main.h"
#include "gui.h"
#include "guiproc.h"

int crsed_quit = 0;
BITMAP * crsed_menu_image = NULL;
BITMAP * crsed_screen = NULL;
BITMAP * crsed_course_screen = NULL;
BITMAP ** crsed_current_image = NULL;
PALETTE crsed_temp_palette;
char crsed_filename[1024] = {0};
char crsed_image_filename[1024] = {0};
VGOLF_COURSE * crsed_course = NULL;
int crsed_selected_hole = 0;
int crsed_hover_poly = -1;
int crsed_hover_point = -1;
int crsed_hover_vert = -1;
int crsed_hover_radius = -1;
int crsed_new_poly = -1;
int crsed_new_point = -1;
int crsed_selected_poly = 0;
int crsed_selected_point = 0;
int crsed_selected_vert = 0;
int crsed_selected_radius = -1;
int crsed_selected_vert_type = VERT_FLAG_TEE;
int crsed_selected_image = 0;
int crsed_dragging = 0;
int crsed_mode = 0;
int crsed_vert_mode = 0;
int crsed_connect_count = 0;
int crsed_zoom = 1;
int crsed_ox = 0;
int crsed_oy = 0;
int crsed_mouse_x, crsed_mouse_y, crsed_mouse_b;
int crsed_peg_x, crsed_peg_y;
int crsed_old_mouse_x, crsed_old_mouse_y, crsed_old_mouse_b;
int crsed_click_tick = 0;
int crsed_click_x, crsed_click_y;
int crsed_change_count = 0;
int crsed_changes = 0;

NCDFS_FILTER_LIST * crsed_filter_course_files = NULL;
NCDFS_FILTER_LIST * crsed_filter_image_files = NULL;
NCDFS_FILTER_LIST * crsed_filter_save_image_files = NULL;
NCDFS_FILTER_LIST * crsed_filter_hole_files = NULL;
NCDFS_FILTER_LIST * crsed_filter_all_files = NULL;

int crsed_object_type[] = 
{
	VERT_FLAG_TEE, // {"TEE", crsed_menu_object_type_TEE, NULL, 0, NULL},
	VERT_FLAG_HOLE, //{"CUP", crsed_menu_object_type_CUP, NULL, 0, NULL},
	0,
	VERT_FLAG_TREE_0, //{"TREE_0", crsed_menu_object_type_TREE_0, NULL, 0, NULL},
	VERT_FLAG_TREE_1, //{"TREE_1", crsed_menu_object_type_TREE_1, NULL, 0, NULL},
	VERT_FLAG_ANIMAL_0, //{"ANIMAL_0", crsed_menu_object_type_ANIMAL_0, NULL, 0, NULL},
	VERT_FLAG_ANIMAL_1, //{"ANIMAL_1", crsed_menu_object_type_ANIMAL_1, NULL, 0, NULL},
	0,
	VERT_FLAG_HAZARD_WATER, //{"HAZARD_WATER", crsed_menu_object_type_HAZARD_WATER, NULL, 0, NULL},
	VERT_FLAG_HAZARD_SAND, //{"HAZARD_SAND", crsed_menu_object_type_HAZARD_SAND, NULL, 0, NULL},
	VERT_FLAG_HAZARD_ROUGH, //{"HAZARD_ROUGH", crsed_menu_object_type_HAZARD_ROUGH, NULL, 0, NULL},
	VERT_FLAG_HAZARD_SLIME, //{"HAZARD_SLIME", crsed_menu_object_type_HAZARD_SLIME, NULL, 0, NULL},
	VERT_FLAG_INDENT_HILL, //{"HILL", crsed_menu_object_type_HILL, NULL, 0, NULL},
	VERT_FLAG_INDENT_RUT, //{"RUT", crsed_menu_object_type_RUT, NULL, 0, NULL},
	0,
	VERT_FLAG_PIPE_ENTER, //{"PIPE_ENTER", crsed_menu_object_type_PIPE_ENTER, NULL, 0, NULL},
	VERT_FLAG_PIPE_EXIT, //{"PIPE_EXIT", crsed_menu_object_type_PIPE_EXIT, NULL, 0, NULL},
	VERT_FLAG_TELEPORT, //{"TELEPORT", crsed_menu_object_type_TELEPORT, NULL, 0, NULL},
	0,
	VERT_FLAG_CLOUD_HORIZ, //{"CLOUD_HORIZ", crsed_menu_object_type_CLOUD_HORIZ, NULL, 0, NULL},
	VERT_FLAG_CLOUD_VERT, //{"CLOUD_VERT", crsed_menu_object_type_CLOUD_VERT, NULL, 0, NULL},
	VERT_FLAG_BUBBLE_HORIZ, //{"CLOUD_HORIZ", crsed_menu_object_type_CLOUD_HORIZ, NULL, 0, NULL},
	VERT_FLAG_BUBBLE_VERT, //{"CLOUD_VERT", crsed_menu_object_type_CLOUD_VERT, NULL, 0, NULL},
	VERT_FLAG_BLINKER, //{"BLINKER", crsed_menu_object_type_BLINKER, NULL, 0, NULL},
	0,
	VERT_FLAG_LASER_SEND, //{"LASER_SEND", crsed_menu_object_type_LASER_SEND, NULL, 0, NULL},
	VERT_FLAG_LASER_RECV, //{"LASER_RECEIVE", crsed_menu_object_type_LASER_RECEIVE, NULL, 0, NULL},
	VERT_FLAG_LASER_RESPAWN, //{"LASER_RESPAWN", crsed_menu_object_type_LASER_RESPAWN, NULL, 0, NULL},
	0,
	VERT_FLAG_POLY_MORPH, //{"POLY_MORPH", crsed_menu_object_type_POLY_MORPH, NULL, 0, NULL},
	VERT_FLAG_POLY_MOTION_AREA, //{"POLY_MOTION_AREA", crsed_menu_object_type_POLY_MOTION_AREA, NULL, 0, NULL},
	VERT_FLAG_POLY_MOVE_HORIZ, //{"POLY_MOVE_HORIZ", crsed_menu_object_type_POLY_MOVE_HORIZ, NULL, 0, NULL},
	VERT_FLAG_POLY_MOVE_VERT, //{"POLY_MOVE_VERT", crsed_menu_object_type_POLY_MOVE_VERT, NULL, 0, NULL},
	VERT_FLAG_POLY_DOOR_HORIZ, //{"DOOR_HORIZ", crsed_menu_object_type_DOOR_HORIZ, NULL, 0, NULL},
	VERT_FLAG_POLY_DOOR_VERT, //{"DOOR_VERTICAL", crsed_menu_object_type_DOOR_VERTICAL, NULL, 0, NULL},
	VERT_FLAG_SMASH_RESPAWN, //{"DOOR_RESPAWN", crsed_menu_object_type_DOOR_RESPAWN, NULL, 0, NULL},
	VERT_FLAG_CONVEYOR, //{"CONVEYOR", crsed_menu_object_type_CONVEYOR, NULL, 0, NULL},
};

char * crsed_object_type_name[] = 
{
	"TEE", // {"TEE", crsed_menu_object_type_TEE, NULL, 0, NULL},
	"HOLE", //{"CUP", crsed_menu_object_type_CUP, NULL, 0, NULL},
	"",
	"TREE_0", //{"TREE_0", crsed_menu_object_type_TREE_0, NULL, 0, NULL},
	"TREE_1", //{"TREE_1", crsed_menu_object_type_TREE_1, NULL, 0, NULL},
	"ANIMAL_0", //{"ANIMAL_0", crsed_menu_object_type_ANIMAL_0, NULL, 0, NULL},
	"ANIMAL_1", //{"ANIMAL_1", crsed_menu_object_type_ANIMAL_1, NULL, 0, NULL},
	"",
	"HAZARD_WATER", //{"HAZARD_WATER", crsed_menu_object_type_HAZARD_WATER, NULL, 0, NULL},
	"HAZARD_SAND", //{"HAZARD_SAND", crsed_menu_object_type_HAZARD_SAND, NULL, 0, NULL},
	"HAZARD_ROUGH", //{"HAZARD_ROUGH", crsed_menu_object_type_HAZARD_ROUGH, NULL, 0, NULL},
	"HAZARD_SLIME", //{"HAZARD_SLIME", crsed_menu_object_type_HAZARD_SLIME, NULL, 0, NULL},
	"INDENT_HILL", //{"HILL", crsed_menu_object_type_HILL, NULL, 0, NULL},
	"INDENT_RUT", //{"RUT", crsed_menu_object_type_RUT, NULL, 0, NULL},
	"",
	"PIPE_ENTER", //{"PIPE_ENTER", crsed_menu_object_type_PIPE_ENTER, NULL, 0, NULL},
	"PIPE_EXIT", //{"PIPE_EXIT", crsed_menu_object_type_PIPE_EXIT, NULL, 0, NULL},
	"TELEPORT", //{"TELEPORT", crsed_menu_object_type_TELEPORT, NULL, 0, NULL},
	"",
	"CLOUD_HORIZ", //{"CLOUD_HORIZ", crsed_menu_object_type_CLOUD_HORIZ, NULL, 0, NULL},
	"CLOUD_VERT", //{"CLOUD_VERT", crsed_menu_object_type_CLOUD_VERT, NULL, 0, NULL},
	"BUBBLE_HORIZ", //{"CLOUD_HORIZ", crsed_menu_object_type_CLOUD_HORIZ, NULL, 0, NULL},
	"BUBBLE_VERT", //{"CLOUD_VERT", crsed_menu_object_type_CLOUD_VERT, NULL, 0, NULL},
	"BLINKER", //{"BLINKER", crsed_menu_object_type_BLINKER, NULL, 0, NULL},
	"",
	"LASER_SEND", //{"LASER_SEND", crsed_menu_object_type_LASER_SEND, NULL, 0, NULL},
	"LASER_RECV", //{"LASER_RECEIVE", crsed_menu_object_type_LASER_RECEIVE, NULL, 0, NULL},
	"LASER_RESPAWN", //{"LASER_RESPAWN", crsed_menu_object_type_LASER_RESPAWN, NULL, 0, NULL},
	"",
	"POLY_MORPH", //{"POLY_MORPH", crsed_menu_object_type_POLY_MORPH, NULL, 0, NULL},
	"POLY_MOTION_AREA", //{"POLY_MOTION_AREA", crsed_menu_object_type_POLY_MOTION_AREA, NULL, 0, NULL},
	"POLY_MOVE_HORIZ", //{"POLY_MOVE_HORIZ", crsed_menu_object_type_POLY_MOVE_HORIZ, NULL, 0, NULL},
	"POLY_MOVE_VERT", //{"POLY_MOVE_VERT", crsed_menu_object_type_POLY_MOVE_VERT, NULL, 0, NULL},
	"POLY_DOOR_HORIZ", //{"DOOR_HORIZ", crsed_menu_object_type_DOOR_HORIZ, NULL, 0, NULL},
	"POLY_DOOR_VERT", //{"DOOR_VERTICAL", crsed_menu_object_type_DOOR_VERTICAL, NULL, 0, NULL},
	"SMASH_RESPAWN", //{"DOOR_RESPAWN", crsed_menu_object_type_DOOR_RESPAWN, NULL, 0, NULL},
	"CONVEYOR" //{"CONVEYOR", crsed_menu_object_type_CONVEYOR, NULL, 0, NULL},
};

char * crsed_image_type_name[] = 
{
	"Title",
	"Scoreboard",
	"End BG",
	"End Podium",
	"End Cloud 1",
	"End Cloud 2",
	"1st Trophy",
	"2nd Trophy",
	"3rd Trophy",
	"Hole",
	"Hole Sunk",
	"Tree 1",
	"Tree 2",
	"Laser Off",
	"Laser On",
	"Teleport 1",
	"Teleport 2",
	"Conveyor 1",
	"Conveyor 2",
	"Conveyor 3",
	"Conveyor 4",
	"Animal 1 Right 1",
	"Animal 1 Right 2",
	"Animal 1 Left 1",
	"Animal 1 Left 2",
	"Animal 2 Right 1",
	"Animal 2 Right 2",
	"Animal 2 Left 1",
	"Animal 2 Left 2"
};

void crsed_fix_window_title(void)
{
	char window_text[1024] = {0};
	
	if(ustricmp(crsed_filename, ""))
	{
		usprintf(window_text, "%svGolf Course Editor - %s (%s)", crsed_changes ? "*" : "", crsed_filename, crsed_course->name);
	}
	else
	{
		usprintf(window_text, "%svGolf Course Editor - Untitled (%s)", crsed_changes ? "*" : "", crsed_course->name);
	}
	set_window_title(window_text);
}

void crsed_mark_change(void)
{
	crsed_changes++;
	crsed_fix_window_title();
}

int crsed_initialize(int argc, char * argv[])
{
	allegro_init();
	set_window_title("vGolf Course Editor");
	if(install_keyboard())
	{
		allegro_message("Can't set up keyboard!");
		return 0;
	}
	if(install_mouse() < 0)
	{
		allegro_message("Can't set up mouse!");
		return 0;
	}
	
	crsed_filter_course_files = ncdfs_filter_list_create();
	if(!crsed_filter_course_files)
	{
		allegro_message("Could not create file list filter!");
		return 0;
	}
	ncdfs_filter_list_add(crsed_filter_course_files, "crs", "Course Files (*.crs)", 1);

	crsed_filter_image_files = ncdfs_filter_list_create();
	if(!crsed_filter_image_files)
	{
		allegro_message("Could not create file list filter!");
		return 0;
	}
	ncdfs_filter_list_add(crsed_filter_image_files, "pcx;bmp;tga;lbm", "Image Files (*.pcx;*.bmp;*.tga;*.lbm)", 1);

	crsed_filter_hole_files = ncdfs_filter_list_create();
	if(!crsed_filter_hole_files)
	{
		allegro_message("Could not create file list filter!");
		return 0;
	}
	ncdfs_filter_list_add(crsed_filter_hole_files, "vgh", "Hole Files (*.vgh)", 1);
	
	crsed_filter_save_image_files = ncdfs_filter_list_create();
	if(!crsed_filter_save_image_files)
	{
		allegro_message("Could not create file list filter!");
		return 0;
	}
	ncdfs_filter_list_add(crsed_filter_save_image_files, "pcx;bmp;tga", "Image Files (*.pcx;*.bmp;*.tga)", 1);
	
	crsed_filter_all_files = ncdfs_filter_list_create();
	if(!crsed_filter_all_files)
	{
		allegro_message("Could not create file list filter!");
		return 0;
	}
	ncdfs_filter_list_add(crsed_filter_all_files, "*", "All Files (*.*)", 1);

	InitIdleSystem();
	
	if(set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 512, 0, 0))
	{
		return 0;
	}
	set_display_switch_mode(SWITCH_PAUSE);
	PrepVSyncIdle();
	
	crsed_screen = create_bitmap(640, 512);
	if(!crsed_screen)
	{
		return 0;
	}
	crsed_course_screen = create_sub_bitmap(crsed_screen, 0, 16, 640, 480);
	if(!crsed_course_screen)
	{
		return 0;
	}
	
	vgolf_hole_editor_mode = 1;
	crsed_course = vgolf_load_course("course_editor.dat");
	if(!crsed_course)
	{
		return 0;
	}
	set_palette(crsed_course->palette);
	crsed_fix_window_title();
	
	show_mouse(NULL);
	if(show_os_cursor(2) < 0)
	{
		ncdgui_initialize(NCDGUI_CURSOR_SOFTWARE);
	}
	else
	{
		ncdgui_initialize(NCDGUI_CURSOR_OS);
	}
	
	crsed_prepare_menus();
	
	gametime_init(60); // 100hz timer
	return 1;
}

void crsed_exit(void)
{
}

void crsed_delete_poly(int poly)
{
	int i;

	for(i = poly; i < crsed_course->hole[crsed_selected_hole].num_polys - 1; i++)
	{
		memcpy(&crsed_course->hole[crsed_selected_hole].body[i], &crsed_course->hole[crsed_selected_hole].body[i + 1], sizeof(BODY));
	}
	crsed_course->hole[crsed_selected_hole].num_polys--;
	if(crsed_selected_poly >= crsed_course->hole[crsed_selected_hole].num_polys)
	{
		crsed_selected_poly = crsed_course->hole[crsed_selected_hole].num_polys - 1;
		if(crsed_selected_poly < 0)
		{
			crsed_selected_poly = 0;
			crsed_prepare_menus();
		}
	}
	crsed_mark_change();
}

void crsed_delete_vert(int vert)
{
	int i;

	for(i = vert; i < crsed_course->hole[crsed_selected_hole].num_verts - 1; i++)
	{
		memcpy(&crsed_course->hole[crsed_selected_hole].vert[i], &crsed_course->hole[crsed_selected_hole].vert[i + 1], sizeof(VERT));
	}
	crsed_course->hole[crsed_selected_hole].num_verts--;
	for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_verts; i++)
	{
		if(crsed_course->hole[crsed_selected_hole].vert[i].flag == VERT_FLAG_TELEPORT)
		{
			if(crsed_course->hole[crsed_selected_hole].vert[i].ref_no[0] > vert)
			{
				crsed_course->hole[crsed_selected_hole].vert[i].ref_no[0]--;
			}
			else if(crsed_course->hole[crsed_selected_hole].vert[i].ref_no[0] >= crsed_course->hole[crsed_selected_hole].num_verts)
			{
				crsed_course->hole[crsed_selected_hole].vert[i].ref_no[0] = 0;
			}
		}
	}
	if(crsed_selected_vert >= crsed_course->hole[crsed_selected_hole].num_verts)
	{
		crsed_selected_vert = crsed_course->hole[crsed_selected_hole].num_verts - 1;
		if(crsed_selected_vert < 0)
		{
			crsed_selected_vert = 0;
			crsed_prepare_menus();
		}
	}
	crsed_mark_change();
}

/* a = (c * sin(A)) / sin(C) */
fixed solve_asa_triangle_a(fixed A, fixed c, fixed B)
{
	fixed C = fixsub(itofix(128), fixadd(A, B));
	return fdiv(fmul(c, fixsin(A)), fixsin(C));
}

/* b = (c * sin(B)) / sin(C) */
fixed solve_asa_triangle_b(fixed A, fixed c, fixed B)
{
	fixed C = fixsub(itofix(128), fixadd(A, B));
	return fdiv(fmul(c, fixsin(B)), fixsin(C));
}

void crsed_poly_logic(void)
{
	int i, j;
	int a, b, c;
	fixed angle;
	fixed hypotenuse;
	int iangle;
	int A = 0;
	
	if(key[KEY_DEL])
	{
		if(KEY_EITHER_CTRL)
		{
			crsed_delete_poly(crsed_selected_poly);
		}
		else
		{
			for(i = crsed_selected_point; i < crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices - 1; i++)
			{
				memcpy(&crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[i], &crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[i + 1], sizeof(VERTEX));
			}
			crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices--;
			if(crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices < 2)
			{
				crsed_delete_poly(crsed_selected_poly);
			}
			else if(crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices > 3)
			{
				if(crsed_selected_point == 0)
				{
					memcpy(&crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices - 1], &crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[0], sizeof(VERTEX));
				}
				else if(crsed_selected_point == crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices)
				{
					memcpy(&crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[0], &crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices - 1], sizeof(VERTEX));
				}
			}
			else if(crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices == 3)
			{
				crsed_delete_poly(crsed_selected_poly);
			}
			if(crsed_selected_point >= crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices)
			{
				crsed_selected_point--;
			}
			crsed_mark_change();
		}
		key[KEY_DEL] = 0;
	}
	crsed_hover_poly = crsed_new_poly;
	crsed_hover_point = crsed_new_poly;
	if(crsed_hover_poly < 0)
	{
		for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_polys; i++)
		{
			for(j = 0; j < crsed_course->hole[crsed_selected_hole].body[i].poly.num_vertices; j++)
			{
				if(crsed_mouse_x >= crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].x + crsed_course->hole[crsed_selected_hole].body[i].pos.x - 2 && crsed_mouse_x <= crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].x + crsed_course->hole[crsed_selected_hole].body[i].pos.x + 2 && crsed_mouse_y >= crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].y + crsed_course->hole[crsed_selected_hole].body[i].pos.y - 2 && crsed_mouse_y <= crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].y + crsed_course->hole[crsed_selected_hole].body[i].pos.y + 2)
				{
					crsed_hover_poly = i;
					crsed_hover_point = j;
					break;
				}
			}
		}
	}
	if(crsed_mouse_b & 1)
	{
		crsed_click_tick++;
		if(crsed_click_tick == 1)
		{
			if(crsed_new_poly >= 0 && crsed_new_point >= 0)
			{
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices--;
				if(crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices < 2)
				{
					crsed_course->hole[crsed_selected_hole].num_polys--;
				}
				else if(crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices > 2)
				{
					if(distance(crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 1].x, crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 1].y, crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[0].x, crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[0].y) < 4)
					{
						memcpy(&crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 1], &crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[0], sizeof(VERTEX));
					}
					else
					{
						memcpy(&crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices], &crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[0], sizeof(VERTEX));
						crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices++;
					}
					crsed_selected_poly = crsed_new_poly;
					crsed_prepare_menus();
					crsed_mark_change();
				}
				crsed_new_poly = -1;
				crsed_new_point = -1;
			}
			else if(crsed_hover_poly >= 0 && crsed_hover_point >= 0)
			{
				if(!crsed_dragging)
				{
					crsed_selected_poly = crsed_hover_poly;
					crsed_selected_point = crsed_hover_point;
					crsed_peg_x = crsed_mouse_x - crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[crsed_selected_point].x - 320;
					crsed_peg_y = crsed_mouse_y - crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[crsed_selected_point].y - 240;
					crsed_dragging = 1;
				}
			}
		}
		else
		{
			if(crsed_dragging)
			{
				if(crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[crsed_selected_point].x != crsed_mouse_x - crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].pos.x - crsed_peg_x || crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[crsed_selected_point].y != crsed_mouse_y - crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].pos.y - crsed_peg_y)
				{
					crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[crsed_selected_point].x = crsed_mouse_x - crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].pos.x - crsed_peg_x;
					crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[crsed_selected_point].y = crsed_mouse_y - crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].pos.y - crsed_peg_y;
					if(crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices > 2)
					{
						if(crsed_selected_point == 0)
						{
							crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices - 1].x = crsed_mouse_x - crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].pos.x - crsed_peg_x;
							crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices - 1].y = crsed_mouse_y - crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].pos.y - crsed_peg_y;
						}
						else if(crsed_selected_point == crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.num_vertices - 1)
						{
							crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[0].x = crsed_mouse_x - crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].pos.x - crsed_peg_x;
							crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[0].y = crsed_mouse_y - crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].pos.y - crsed_peg_y;
						}
					}
					crsed_mark_change();
				}
			}
		}
	}
	else if((crsed_mouse_b & 2) || key[KEY_INSERT])
	{
		crsed_click_tick++;
		if(crsed_click_tick == 1)
		{
			if(crsed_new_poly < 0 && crsed_new_point < 0 && crsed_course->hole[crsed_selected_hole].num_polys < MAX_HOLE_POLYS)
			{
				crsed_new_poly = crsed_course->hole[crsed_selected_hole].num_polys;
				crsed_new_point = 0;
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.x = 320;
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.y = 240;
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[0].x = crsed_mouse_x - crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.x;
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[0].y = crsed_mouse_y - crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.y;
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[1].x = crsed_mouse_x - crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.x;
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[1].y = crsed_mouse_y - crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.y;
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices = 2;
				crsed_course->hole[crsed_selected_hole].num_polys++;
			}
			else if(crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices < MAX_OBJECT_VERTICES - 2)
			{
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices].x = crsed_mouse_x - crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.x;
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices].y = crsed_mouse_y - crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.y;
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices++;
			}
		}
	}
	else
	{
		crsed_click_tick = 0;
		crsed_dragging = 0;
		if(crsed_new_poly >= 0 && crsed_new_point >= 0)
		{
			/* handle angle snap logic */
			if(key[KEY_LCONTROL])
			{
				/* to-do: figure out how to close the gap so vertex is placed at the same x- or y-coordinate as the mouse */
				if(crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices > 1)
				{
					a = crsed_mouse_x - (crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 2].x + crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.x);
					b = crsed_mouse_y - (crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 2].y + crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.y);
					angle = fixatan2(itofix(a), itofix(b));
					if(angle < 0)
					{
						c = -8;
					}
					else
					{
						c = 8;
					}
					iangle = -((fixtoi(angle) + c) / 16) * 16 + 64;
					if(iangle < 0)
					{
						iangle += 256;
					}
					if(iangle == 0 || iangle == 128)
					{
						hypotenuse = itofix(abs(a));
					}
					else if(iangle == 64 || iangle == 192)
					{
						hypotenuse = itofix(abs(b));
					}
					else
					{
						switch(iangle)
						{
							case 16:
							{
								A = 16;
								break;
							}
							case 32:
							{
								A = 64 - 32;
								break;
							}
							case 48:
							{
								A = 64 - 48;
								break;
							}
							case 80:
							{
								A = 80 - 64;
								break;
							}
							case 96:
							{
								A = 96 - 64;
								break;
							}
							case 112:
							{
								A = 16;
								break;
							}
							case 144:
							{
								A = 144 - 128;
								break;
							}
							case 160:
							{
								A = 160 - 128;
								break;
							}
							case 176:
							{
								A = 16;
								break;
							}
							case 208:
							{
								A = 208 - 192;
								break;
							}
							case 224:
							{
								A = 224 - 192;
								break;
							}
							case 240:
							{
								A = 16;
								break;
							}
						}
						if(abs(a) < abs(b))
						{
							hypotenuse = solve_asa_triangle_b(itofix(A), itofix(abs(b)), itofix(64));
						}
						else
						{
							hypotenuse = solve_asa_triangle_b(itofix(A), itofix(abs(a)), itofix(64));
						}
					}
					crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 1].x = crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 2].x + fixtoi(fmul(fixcos(itofix(iangle)), hypotenuse));
					crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 1].y = crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 2].y + fixtoi(fmul(fixsin(itofix(iangle)), hypotenuse));
				}
			}
			else
			{
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 1].x = crsed_mouse_x - crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.x;
				crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.p[crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].poly.num_vertices - 1].y = crsed_mouse_y - crsed_course->hole[crsed_selected_hole].body[crsed_new_poly].pos.y;
			}
		}
	}
}

void crsed_vert_logic(void)
{
	int i, j;
	
	if(key[KEY_DEL])
	{
		crsed_delete_vert(crsed_selected_vert);
		key[KEY_DEL] = 0;
	}
	
	switch(crsed_vert_mode)
	{
		
		/* normal vert edit mode */
		case 0:
		{
			crsed_hover_vert = -1;
			crsed_hover_radius = -1;
			for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_verts; i++)
			{
				switch(crsed_course->hole[crsed_selected_hole].vert[i].flag)
				{
					case VERT_FLAG_TEE:
					case VERT_FLAG_HOLE:
					case VERT_FLAG_TREE_0:
					case VERT_FLAG_TREE_1:
					case VERT_FLAG_LASER_SEND:
					case VERT_FLAG_LASER_RECV:
					case VERT_FLAG_LASER_RESPAWN:
					{
						if(crsed_mouse_x >= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x - 2 && crsed_mouse_x <= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x + 2 && crsed_mouse_y >= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y - 2 && crsed_mouse_y <= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y + 2)
						{
							crsed_hover_vert = i;
							i = 65536;
						}
						break;
					}
					default:
					{
						if(crsed_mouse_x >= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x - 2 && crsed_mouse_x <= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x + 2 && crsed_mouse_y >= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y - 2 && crsed_mouse_y <= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y + 2)
						{
							crsed_hover_vert = i;
							break;
						}
						else if(crsed_mouse_x >= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x + crsed_course->hole[crsed_selected_hole].vert[i].radius - 2 && crsed_mouse_x <= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x + crsed_course->hole[crsed_selected_hole].vert[i].radius + 2 && crsed_mouse_y >= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y - 2 && crsed_mouse_y <= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y + 2)
						{
							crsed_hover_vert = i;
							crsed_hover_radius = i;
							i = 65536;
						}
						break;
					}
				}
			}
			if(crsed_mouse_b & 1)
			{
				crsed_click_tick++;
				if(crsed_click_tick == 1)
				{
					if(crsed_hover_vert >= 0)
					{
						if(!crsed_dragging)
						{
							crsed_selected_vert = crsed_hover_vert;
							crsed_selected_radius = crsed_hover_radius;
							crsed_peg_x = crsed_mouse_x - crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].s.x - 320;
							crsed_peg_y = crsed_mouse_y - crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].s.y - 240;
							crsed_dragging = 1;
						}
					}
				}
				else
				{
					if(crsed_dragging)
					{
						if(crsed_selected_radius >= 0)
						{
							if(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].radius != distance(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].s.x + 320, crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].s.y + 240, crsed_mouse_x, crsed_mouse_y))
							{
								crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].radius = distance(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].s.x + 320, crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].s.y + 240, crsed_mouse_x, crsed_mouse_y);
								if(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].radius < 0)
								{
									crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].radius = -crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].radius;
								}
								crsed_mark_change();
							}
						}
						else
						{
							if(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].s.x != crsed_mouse_x - crsed_course->hole[crsed_selected_hole].s.x - crsed_peg_x || crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].s.y != crsed_mouse_y - crsed_course->hole[crsed_selected_hole].s.y - crsed_peg_y)
							{
								crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].s.x = crsed_mouse_x - crsed_course->hole[crsed_selected_hole].s.x - crsed_peg_x;
								crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].s.y = crsed_mouse_y - crsed_course->hole[crsed_selected_hole].s.y - crsed_peg_y;
								crsed_mark_change();
							}
						}
					}
				}
			}
			else if((crsed_mouse_b & 2) || key[KEY_INSERT])
			{
				crsed_click_tick++;
				if(crsed_click_tick == 1 && crsed_course->hole[crsed_selected_hole].num_verts < MAX_HOLE_VERTS)
				{
					crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].num_verts].s.x = crsed_mouse_x - 320;
					crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].num_verts].s.y = crsed_mouse_y - 240;
					crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].num_verts].radius = 10;
					crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].num_verts].flag = crsed_selected_vert_type;
					crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].num_verts].num_refs = 8;
					for(i = 0; i < crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].num_verts].num_refs; i++)
					{
						crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].num_verts].ref_no[i] = 0;
					}
					crsed_selected_vert = crsed_course->hole[crsed_selected_hole].num_verts;
					crsed_course->hole[crsed_selected_hole].num_verts++;
					crsed_mark_change();
				}
			}
			else
			{
				crsed_click_tick = 0;
				crsed_dragging = 0;
			}
			break;
		}
		
		/* define connection between verts */
		case 1:
		{
			crsed_hover_vert = -1;
			crsed_hover_radius = -1;
			switch(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag)
			{
				case VERT_FLAG_PIPE_ENTER:
				{
					for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_verts; i++)
					{
						if(crsed_mouse_x >= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x - 2 && crsed_mouse_x <= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x + 2 && crsed_mouse_y >= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y - 2 && crsed_mouse_y <= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y + 2 && crsed_course->hole[crsed_selected_hole].vert[i].flag == VERT_FLAG_PIPE_EXIT)
						{
							crsed_hover_vert = i;
							i = 65536;
						}
					}
					if(crsed_mouse_b & 1)
					{
						crsed_click_tick++;
						if(crsed_click_tick == 1)
						{
							if(crsed_hover_vert >= 0 && crsed_hover_vert != crsed_selected_vert)
							{
								crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[1] = crsed_hover_vert;
								crsed_vert_mode = 0;
							}
							else
							{
								crsed_vert_mode = 0;
							}
						}
					}
					else
					{
						crsed_click_tick = 0;
					}
					break;
				}
				case VERT_FLAG_TELEPORT:
				{
					for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_verts; i++)
					{
						if(crsed_mouse_x >= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x - 2 && crsed_mouse_x <= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x + 2 && crsed_mouse_y >= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y - 2 && crsed_mouse_y <= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y + 2 && crsed_course->hole[crsed_selected_hole].vert[i].flag == VERT_FLAG_TELEPORT)
						{
							crsed_hover_vert = i;
							i = 65536;
						}
					}
					if(crsed_mouse_b & 1)
					{
						crsed_click_tick++;
						if(crsed_click_tick == 1)
						{
							if(crsed_hover_vert >= 0 && crsed_hover_vert != crsed_selected_vert)
							{
								crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[0] = crsed_hover_vert;
								crsed_vert_mode = 0;
							}
							else
							{
								crsed_vert_mode = 0;
							}
						}
					}
					else
					{
						crsed_click_tick = 0;
					}
					break;
				}
				case VERT_FLAG_LASER_SEND:
				{
					for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_verts; i++)
					{
						if(crsed_mouse_x >= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x - 2 && crsed_mouse_x <= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x + 2 && crsed_mouse_y >= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y - 2 && crsed_mouse_y <= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y + 2 && crsed_course->hole[crsed_selected_hole].vert[i].flag == VERT_FLAG_LASER_RECV)
						{
							crsed_hover_vert = i;
							i = 65536;
						}
					}
					if(crsed_mouse_b & 1)
					{
						crsed_click_tick++;
						if(crsed_click_tick == 1)
						{
							if(crsed_hover_vert >= 0 && crsed_hover_vert != crsed_selected_vert)
							{
								crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[5] = crsed_hover_vert;
								crsed_vert_mode = 0;
							}
							else
							{
								crsed_vert_mode = 0;
							}
						}
					}
					else
					{
						crsed_click_tick = 0;
					}
					break;
				}
				case VERT_FLAG_POLY_MOTION_AREA:
				{
					for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_verts; i++)
					{
						if(crsed_mouse_x >= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x - 2 && crsed_mouse_x <= crsed_course->hole[crsed_selected_hole].s.x + crsed_course->hole[crsed_selected_hole].vert[i].s.x + 2 && crsed_mouse_y >= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y - 2 && crsed_mouse_y <= crsed_course->hole[crsed_selected_hole].s.y + crsed_course->hole[crsed_selected_hole].vert[i].s.y + 2 && (crsed_course->hole[crsed_selected_hole].vert[i].flag == VERT_FLAG_POLY_DOOR_HORIZ || crsed_course->hole[crsed_selected_hole].vert[i].flag == VERT_FLAG_POLY_DOOR_VERT || crsed_course->hole[crsed_selected_hole].vert[i].flag == VERT_FLAG_POLY_MOVE_HORIZ || crsed_course->hole[crsed_selected_hole].vert[i].flag == VERT_FLAG_POLY_MOVE_VERT))
						{
							crsed_hover_vert = i;
							i = 65536;
						}
					}
					if(crsed_mouse_b & 1)
					{
						crsed_click_tick++;
						if(crsed_click_tick == 1)
						{
							if(crsed_hover_vert >= 0 && crsed_hover_vert != crsed_selected_vert)
							{
								switch(crsed_course->hole[crsed_selected_hole].vert[crsed_hover_vert].flag)
								{
									case VERT_FLAG_POLY_MOVE_HORIZ:
									{
										crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[0] = 1;
										crsed_vert_mode = 0;
										break;
									}
									case VERT_FLAG_POLY_MOVE_VERT:
									{
										crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[0] = 2;
										crsed_vert_mode = 0;
										break;
									}
									case VERT_FLAG_POLY_DOOR_HORIZ:
									{
										crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[0] = 3;
										crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[3] = crsed_hover_vert;
										crsed_vert_mode = 0;
										break;
									}
									case VERT_FLAG_POLY_DOOR_VERT:
									{
										crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[0] = 4;
										crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[3] = crsed_hover_vert;
										crsed_vert_mode = 0;
										break;
									}
								}
							}
							else
							{
								crsed_vert_mode = 0;
							}
						}
					}
					else
					{
						crsed_click_tick = 0;
					}
					break;
				}
				default:
				{
					crsed_hover_poly = -1;
					crsed_hover_point = -1;
					for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_polys; i++)
					{
						for(j = 0; j < crsed_course->hole[crsed_selected_hole].body[i].poly.num_vertices; j++)
						{
							if(crsed_mouse_x >= crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].x + crsed_course->hole[crsed_selected_hole].body[i].pos.x - 2 && crsed_mouse_x <= crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].x + crsed_course->hole[crsed_selected_hole].body[i].pos.x + 2 && crsed_mouse_y >= crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].y + crsed_course->hole[crsed_selected_hole].body[i].pos.y - 2 && crsed_mouse_y <= crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].y + crsed_course->hole[crsed_selected_hole].body[i].pos.y + 2)
							{
								crsed_hover_poly = i;
								crsed_hover_point = j;
								break;
							}
						}
					}
					if(crsed_mouse_b & 1)
					{
						crsed_click_tick++;
						if(crsed_click_tick == 1)
						{
							if(crsed_hover_poly >= 0 && crsed_hover_point >= 0)
							{
								switch(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag)
								{
									case VERT_FLAG_POLY_MORPH:
									case VERT_FLAG_POLY_MOVE_HORIZ:
									case VERT_FLAG_POLY_MOVE_VERT:
									{
										crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[1] = crsed_hover_poly;
										crsed_vert_mode = 0;
										break;
									}
									case VERT_FLAG_CONVEYOR:
									{
										crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[5] = crsed_hover_poly;
										crsed_vert_mode = 0;
										break;
									}
									case VERT_FLAG_POLY_DOOR_HORIZ:
									case VERT_FLAG_POLY_DOOR_VERT:
									{
										if(crsed_connect_count == 0)
										{
											crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[6] = crsed_hover_poly;
											crsed_connect_count++;
										}
										else
										{
											crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[7] = crsed_hover_poly;
											crsed_vert_mode = 0;
										}
										break;
									}
								}
							}
							else
							{
								switch(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag)
								{
									case VERT_FLAG_POLY_MORPH:
									case VERT_FLAG_POLY_MOVE_HORIZ:
									case VERT_FLAG_POLY_MOVE_VERT:
									case VERT_FLAG_CONVEYOR:
									{
										crsed_vert_mode = 0;
										break;
									}
								}
							}
						}
					}
					else
					{
						crsed_click_tick = 0;
					}
					break;
				}
			}
			break;
		}
		
		/* define angle of vert */
		case 2:
		{
			switch(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag)
			{
				case VERT_FLAG_PIPE_ENTER:
				{
					crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[4] = 128 + fixtoi(fixatan2(itofix(240 + crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[1]].s.y - crsed_mouse_y), itofix(320 + crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[1]].s.x - crsed_mouse_x)));
					if(crsed_mouse_b & 1)
					{
						crsed_click_tick++;
						if(crsed_click_tick == 1)
						{
							crsed_vert_mode = 0;
						}
					}
					else
					{
						crsed_click_tick = 0;
					}
					break;
				}
				case VERT_FLAG_TELEPORT:
				{
					crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[1] = 128 + fixtoi(fixatan2(itofix(240 + crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[0]].s.y - crsed_mouse_y), itofix(320 + crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[0]].s.x - crsed_mouse_x)));
					if(crsed_mouse_b & 1)
					{
						crsed_click_tick++;
						if(crsed_click_tick == 1)
						{
							crsed_vert_mode = 0;
						}
					}
					else
					{
						crsed_click_tick = 0;
					}
					break;
				}
			}
			break;
		}
	}
}

void crsed_image_logic(void)
{
	switch(crsed_selected_image)
	{
		case 0:
		{
			crsed_current_image = &crsed_course->image_bank.iconpic;
			break;
		}
		case 1:
		{
			crsed_current_image = &crsed_course->scoreboard.bgpic;
			break;
		}
		case 2:
		{
			crsed_current_image = &crsed_course->image_bank.victory_bgpic;
			break;
		}
		case 3:
		{
			crsed_current_image = &crsed_course->image_bank.victory_podiumpic;
			break;
		}
		case 4:
		{
			crsed_current_image = &crsed_course->image_bank.victory_cloudpic[0];
			break;
		}
		case 5:
		{
			crsed_current_image = &crsed_course->image_bank.victory_cloudpic[1];
			break;
		}
		case 6:
		{
			crsed_current_image = &crsed_course->image_bank.victory_trophypic[0];
			break;
		}
		case 7:
		{
			crsed_current_image = &crsed_course->image_bank.victory_trophypic[1];
			break;
		}
		case 8:
		{
			crsed_current_image = &crsed_course->image_bank.victory_trophypic[2];
			break;
		}
		case 9:
		{
			crsed_current_image = &crsed_course->image_bank.vert_pic[1][0];
			break;
		}
		case 10:
		{
			crsed_current_image = &crsed_course->image_bank.vert_pic[1][1];
			break;
		}
		case 11:
		{
			crsed_current_image = &crsed_course->image_bank.object_pic[0][0];
			break;
		}
		case 12:
		{
			crsed_current_image = &crsed_course->image_bank.object_pic[1][0];
			break;
		}
		case 13:
		{
			crsed_current_image = &crsed_course->image_bank.object_pic[2][0];
			break;
		}
		case 14:
		{
			crsed_current_image = &crsed_course->image_bank.object_pic[2][1];
			break;
		}
		case 15:
		{
			crsed_current_image = &crsed_course->image_bank.object_pic[3][0];
			break;
		}
		case 16:
		{
			crsed_current_image = &crsed_course->image_bank.object_pic[3][1];
			break;
		}
		case 17:
		{
			crsed_current_image = &crsed_course->image_bank.object_pic[4][0];
			break;
		}
		case 18:
		{
			crsed_current_image = &crsed_course->image_bank.object_pic[4][1];
			break;
		}
		case 19:
		{
			crsed_current_image = &crsed_course->image_bank.object_pic[4][2];
			break;
		}
		case 20:
		{
			crsed_current_image = &crsed_course->image_bank.object_pic[4][3];
			break;
		}
		case 21:
		{
			crsed_current_image = &crsed_course->image_bank.animal_pic[0][0];
			break;
		}
		case 22:
		{
			crsed_current_image = &crsed_course->image_bank.animal_pic[0][1];
			break;
		}
		case 23:
		{
			crsed_current_image = &crsed_course->image_bank.animal_pic[0][2];
			break;
		}
		case 24:
		{
			crsed_current_image = &crsed_course->image_bank.animal_pic[0][3];
			break;
		}
		case 25:
		{
			crsed_current_image = &crsed_course->image_bank.animal_pic[1][0];
			break;
		}
		case 26:
		{
			crsed_current_image = &crsed_course->image_bank.animal_pic[1][1];
			break;
		}
		case 27:
		{
			crsed_current_image = &crsed_course->image_bank.animal_pic[1][2];
			break;
		}
		case 28:
		{
			crsed_current_image = &crsed_course->image_bank.animal_pic[1][3];
			break;
		}
	}
}

void crsed_logic(void)
{
	if(key[KEY_ESC])
	{
		crsed_menu_file_exit();
		key[KEY_ESC] = 0;
	}
	if(key[KEY_F1])
	{
		crsed_menu_help_manual();
		key[KEY_F1] = 0;
	}
	if(key[KEY_F3])
	{
		crsed_menu_file_load();
		key[KEY_F3] = 0;
	}
	if(key[KEY_F2])
	{
		if(KEY_EITHER_CTRL)
		{
			crsed_menu_file_save_as();
		}
		else
		{
			crsed_menu_file_save();
		}
		key[KEY_F2] = 0;
	}
	
	/* activate the menu when ALT is pressed */
	if(KEY_EITHER_ALT)
	{
		clear_keybuf();
		crsed_prepare_menus();
		ncdgui_popup_dialog(crsed_main_dialog, 0);
	}
	
	/* see if we need to activate the menu */
	if(mouse_y < 16 && mouse_b & 1)
	{
		clear_keybuf();
		crsed_prepare_menus();
		ncdgui_popup_dialog(crsed_main_dialog, 0);
	}
	
	if(crsed_course)
	{
		if(crsed_mode < 2)
		{
			if(key[KEY_LEFT])
			{
				crsed_selected_hole--;
				if(crsed_selected_hole < 0)
				{
					crsed_selected_hole = crsed_course->num_holes - 1;
				}
				crsed_selected_poly = 0;
				crsed_selected_point = 0;
				crsed_selected_vert = 0;
				crsed_prepare_menus();
				key[KEY_LEFT] = 0;
			}
			if(key[KEY_RIGHT])
			{
				crsed_selected_hole++;
				if(crsed_selected_hole >= crsed_course->num_holes)
				{
					crsed_selected_hole = 0;
				}
				crsed_selected_poly = 0;
				crsed_selected_point = 0;
				crsed_selected_vert = 0;
				crsed_prepare_menus();
				key[KEY_RIGHT] = 0;
			}
			if(key[KEY_Z])
			{
				if(crsed_zoom == 1)
				{
					crsed_zoom = 3;
					crsed_ox = mouse_x * crsed_zoom - 320;
					if(crsed_ox < 0)
					{
						crsed_ox = 0;
					}
					else if(crsed_ox > 640 * crsed_zoom - 640)
					{
						crsed_ox = 640 * crsed_zoom - 640;
					}
					crsed_oy = (mouse_y - 16) * crsed_zoom - 240;
					if(crsed_oy < 0)
					{
						crsed_oy = 0;
					}
					else if(crsed_oy > 480 * crsed_zoom - 480)
					{
						crsed_oy = 480 * crsed_zoom - 480;
					}
				}
				else
				{
					crsed_zoom = 1;
					crsed_ox = 0;
					crsed_oy = 0;
				}
				key[KEY_Z] = 0;
			}
		}
		else
		{
			if(key[KEY_LEFT])
			{
				crsed_selected_image--;
				if(crsed_selected_image < 0)
				{
					crsed_selected_image = 28;
				}
				crsed_prepare_menus();
				key[KEY_LEFT] = 0;
			}
			if(key[KEY_RIGHT])
			{
				crsed_selected_image++;
				if(crsed_selected_image > 28)
				{
					crsed_selected_image = 0;
				}
				crsed_prepare_menus();
				key[KEY_RIGHT] = 0;
			}
		}
		if(key[KEY_TAB])
		{
			if(KEY_EITHER_SHIFT)
			{
				crsed_mode--;
				if(crsed_mode < 0)
				{
					crsed_mode = 2;
				}
			}
			else
			{
				crsed_mode++;
				if(crsed_mode > 2)
				{
					crsed_mode = 0;
				}
			}
			crsed_prepare_menus();
			key[KEY_TAB] = 0;
		}
		crsed_old_mouse_x = crsed_mouse_x;
		crsed_old_mouse_y = crsed_mouse_y;
		crsed_old_mouse_b = crsed_mouse_b;
		crsed_mouse_x = mouse_x / crsed_zoom + (crsed_ox / crsed_zoom);
		crsed_mouse_y = (mouse_y - 16) / crsed_zoom + (crsed_oy / crsed_zoom);
		if(crsed_zoom > 1)
		{
			if(mouse_x < 16)
			{
				crsed_ox -= crsed_zoom;
				if(crsed_ox < 0)
				{
					crsed_ox = 0;
				}
			}
			else if(mouse_x >= 640 - 16)
			{
				crsed_ox += crsed_zoom;
				if(crsed_ox > 640 * crsed_zoom - 640)
				{
					crsed_ox = 640 * crsed_zoom - 640;
				}
			}
			if(mouse_y > 16 && mouse_y < 16 + 16)
			{
				crsed_oy -= crsed_zoom;
				if(crsed_oy < 0)
				{
					crsed_oy = 0;
				}
			}
			else if(mouse_y >= 480 + 16 - 16)
			{
				crsed_oy += crsed_zoom;
				if(crsed_oy > 480 * crsed_zoom - 480)
				{
					crsed_oy = 480 * crsed_zoom - 480;
				}
			}
		}
		crsed_mouse_b = mouse_b;
		switch(crsed_mode)
		{
			case 0:
			{
				crsed_poly_logic();
				break;
			}
			case 1:
			{
				crsed_vert_logic();
				break;
			}
			case 2:
			{
				crsed_image_logic();
				break;
			}
		}
	}
}

void draw_polygon(BITMAP * bp, POLY po, int cx, int cy, int c1, int c2)
{
	int i;
	int p[256] = {0};
	int pi = 0;
	int o = crsed_zoom / 2;

	for(i = 0; i < po.num_vertices; i++)
	{
		p[i * 2] = (cx + po.p[i].x) * crsed_zoom + o;
		p[i * 2 + 1] = (cy + po.p[i].y) * crsed_zoom + o;
		pi++;
	}
	if(c1 > 0)
	{
		polygon(bp, pi, p, c1);
	}
	for(i = 0; i < po.num_vertices - 1; i++)
	{
		line(bp, (cx + po.p[i].x) * crsed_zoom - crsed_ox + o, (cy + po.p[i].y) * crsed_zoom - crsed_oy + o, (cx + po.p[i+1].x) * crsed_zoom - crsed_ox + o, (cy + po.p[i + 1].y) * crsed_zoom - crsed_oy + o, c2);
	}
}

void draw_circle(BITMAP * bp, int x, int y, int r, int color)
{
	int o = crsed_zoom / 2;

	circle(bp, x * crsed_zoom - crsed_ox + o, y * crsed_zoom - crsed_oy + o, r * crsed_zoom, color);
}

void draw_filled_circle(BITMAP * bp, int x, int y, int r, int color)
{
	int o = crsed_zoom / 2;

	circlefill(bp, x * crsed_zoom - crsed_ox + o, y * crsed_zoom - crsed_oy + o, r * crsed_zoom, color);
}

void draw_line(BITMAP * bp, int x1, int y1, int x2, int y2, int color)
{
	int o = crsed_zoom / 2;
	
	line(bp, x1 * crsed_zoom - crsed_ox + o, y1 * crsed_zoom - crsed_oy + o, x2 * crsed_zoom - crsed_ox + o, y2 * crsed_zoom - crsed_oy + o, color);
}

void draw_rectangle(BITMAP * bp, int x1, int y1, int x2, int y2, int color)
{
	int o = crsed_zoom / 2;

	rect(bp, x1 * crsed_zoom - crsed_ox + o, y1 * crsed_zoom - crsed_oy + o, x2 * crsed_zoom - crsed_ox + o, y2 * crsed_zoom - crsed_oy + o, color);
}

void draw_filled_rectangle(BITMAP * bp, int x1, int y1, int x2, int y2, int color)
{
	int o = crsed_zoom / 2;

	rectfill(bp, x1 * crsed_zoom - crsed_ox + o, y1 * crsed_zoom - crsed_oy + o, x2 * crsed_zoom - crsed_ox + o, y2 * crsed_zoom - crsed_oy + o, color);
}

void draw_masked_bitmap(BITMAP * src, BITMAP * dest, int xx, int yy, int x, int y, int w, int h)
{
	stretch_sprite(dest, src, x * crsed_zoom - crsed_ox, y * crsed_zoom - crsed_oy, src->w * crsed_zoom, src->h * crsed_zoom);
}

void draw_vflip_sprite(BITMAP * dest, BITMAP * src, int x, int y)
{
	rotate_scaled_sprite_v_flip(dest, src, x * crsed_zoom - crsed_ox, y * crsed_zoom - crsed_oy, itofix(0), itofix(crsed_zoom));
}

void draw_rotated_sprite(BITMAP * dest, BITMAP * src, int x, int y, fixed angle)
{
	rotate_scaled_sprite(dest, src, x * crsed_zoom - crsed_ox, y * crsed_zoom - crsed_oy, angle, itofix(crsed_zoom));
}

void draw_rotated_vflip_sprite(BITMAP * dest, BITMAP * src, int x, int y, fixed angle)
{
	rotate_scaled_sprite_v_flip(dest, src, x * crsed_zoom - crsed_ox, y * crsed_zoom - crsed_oy, angle, itofix(crsed_zoom));
}

void crsed_render_vert(BITMAP * bp, HOLE * hole, int vert)
{
	int i;

	switch(hole->vert[vert].flag)
	{
		case VERT_FLAG_TEE:
			draw_filled_circle(bp, hole->s.x + hole->vert[vert].s.x + 1, hole->s.y + hole->vert[vert].s.y + 3, 8, makecol(0, 0, 0));
			draw_filled_circle(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, 8, makecol(255, 255, 255));
			break;

		case VERT_FLAG_HOLE:        //draw the cup
			draw_masked_bitmap(crsed_course->image_bank.vert_pic[1][0], bp, 0, 0, hole->s.x + hole->vert[vert].s.x - crsed_course->image_bank.vert_pic[1][0]->w / 2, hole->s.y + hole->vert[vert].s.y - crsed_course->image_bank.vert_pic[1][0]->h / 2, crsed_course->image_bank.vert_pic[1][0]->w, crsed_course->image_bank.vert_pic[1][0]->h);
			break;

		case VERT_FLAG_TREE_0:      //draw the various tree types
			draw_masked_bitmap(crsed_course->image_bank.object_pic[0][0], bp, 0, 0, hole->s.x+hole->vert[vert].s.x-crsed_course->image_bank.object_pic[0][0]->w/2, hole->s.y+hole->vert[vert].s.y-crsed_course->image_bank.object_pic[0][0]->h/2, crsed_course->image_bank.object_pic[0][0]->w, crsed_course->image_bank.object_pic[0][0]->h);
			break;

		case VERT_FLAG_TREE_1:
			draw_masked_bitmap(crsed_course->image_bank.object_pic[1][0], bp, 0, 0, hole->s.x+hole->vert[vert].s.x-crsed_course->image_bank.object_pic[1][0]->w/2, hole->s.y+hole->vert[vert].s.y-crsed_course->image_bank.object_pic[1][0]->h/2, crsed_course->image_bank.object_pic[1][0]->w, crsed_course->image_bank.object_pic[1][0]->h);
			break;

		case VERT_FLAG_ANIMAL_0:    //draw the various animal types
			draw_masked_bitmap(crsed_course->image_bank.animal_pic[0][hole->vert[vert].ref_no[0]], bp, 0, 0, hole->s.x+hole->vert[vert].s.x-crsed_course->image_bank.animal_pic[0][hole->vert[vert].ref_no[0]]->w/2, hole->s.y+hole->vert[vert].s.y-crsed_course->image_bank.animal_pic[0][hole->vert[vert].ref_no[0]]->h/2, crsed_course->image_bank.animal_pic[0][hole->vert[vert].ref_no[0]]->w, crsed_course->image_bank.animal_pic[0][hole->vert[vert].ref_no[0]]->h);
			break;

		case VERT_FLAG_ANIMAL_1:
			draw_masked_bitmap(crsed_course->image_bank.animal_pic[1][hole->vert[vert].ref_no[0]], bp, 0, 0, hole->s.x+hole->vert[vert].s.x-crsed_course->image_bank.animal_pic[1][hole->vert[vert].ref_no[0]]->w/2, hole->s.y+hole->vert[vert].s.y-crsed_course->image_bank.animal_pic[1][hole->vert[vert].ref_no[0]]->h/2, crsed_course->image_bank.animal_pic[1][hole->vert[vert].ref_no[0]]->w, crsed_course->image_bank.animal_pic[1][hole->vert[vert].ref_no[0]]->h);
			break;

		case VERT_FLAG_POLY_MORPH:  //draw the rotating polygon to the poly layer
			draw_polygon(crsed_course_screen, hole->body[hole->vert[vert].ref_no[1]].poly, hole->s.x, hole->s.y, hole->vert[vert].ref_no[3], 1);
			break;

		case VERT_FLAG_POLY_MOVE_HORIZ:  //this vert controls a poly that moves horizontally
			draw_polygon(crsed_course_screen, hole->body[hole->vert[vert].ref_no[1]].poly, hole->s.x, hole->s.y, hole->vert[vert].ref_no[3], 1);
			break;

		case VERT_FLAG_POLY_MOVE_VERT:  //this vert controls a poly that moves horizontally
			draw_polygon(crsed_course_screen, hole->body[hole->vert[vert].ref_no[1]].poly, hole->s.x, hole->s.y, hole->vert[vert].ref_no[3], 1);
			break;

		case VERT_FLAG_LASER_SEND:
			//draw the laser sender/receiver
			draw_masked_bitmap(crsed_course->image_bank.object_pic[2][hole->vert[vert].ref_no[0]], bp, 0, 0, hole->s.x+hole->vert[vert].s.x-crsed_course->image_bank.object_pic[2][0]->w/2, hole->s.y+hole->vert[vert].s.y-crsed_course->image_bank.object_pic[2][0]->h/2, crsed_course->image_bank.object_pic[2][0]->w, crsed_course->image_bank.object_pic[2][0]->h);
			//if the laser is active, draw the laser beam between the vert endpoints
			//laser_display(hole->layer_pic[2], hole->s.x+hole->vert[vert].s.x, hole->s.y+hole->vert[vert].s.y, hole->s.x+hole->vert[hole->vert[vert].ref_no[5]].s.x, hole->s.y+hole->vert[hole->vert[vert].ref_no[5]].s.y, hole->vert[hole->vert[vert].ref_no[5]].ref_no[1]);
			if(hole->vert[hole->vert[vert].ref_no[5]].flag == VERT_FLAG_LASER_RECV)
			{
				draw_line(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, hole->s.x + hole->vert[hole->vert[vert].ref_no[5]].s.x, hole->s.y + hole->vert[hole->vert[vert].ref_no[5]].s.y, hole->vert[hole->vert[vert].ref_no[5]].ref_no[1]);
			}
			break;

		case VERT_FLAG_LASER_RECV:
			//draw the laser sender/receiver
			draw_masked_bitmap(crsed_course->image_bank.object_pic[2][hole->vert[vert].ref_no[0]], bp, 0, 0, hole->s.x+hole->vert[vert].s.x-crsed_course->image_bank.object_pic[2][0]->w/2, hole->s.y+hole->vert[vert].s.y-crsed_course->image_bank.object_pic[2][0]->h/2, crsed_course->image_bank.object_pic[2][0]->w, crsed_course->image_bank.object_pic[2][0]->h);
			break;

		case VERT_FLAG_TELEPORT:   //draw the teleport pad
			if(crsed_course->image_bank.object_pic[3][hole->vert[vert].ref_no[5]])
			{
				draw_masked_bitmap(crsed_course->image_bank.object_pic[3][hole->vert[vert].ref_no[5]], bp, 0, 0, hole->s.x+hole->vert[vert].s.x-crsed_course->image_bank.object_pic[3][0]->w/2, hole->s.y+hole->vert[vert].s.y-crsed_course->image_bank.object_pic[3][0]->h/2, crsed_course->image_bank.object_pic[3][0]->w, crsed_course->image_bank.object_pic[3][0]->h);
			}
			break;

		case VERT_FLAG_CONVEYOR:   //draw the conveyor belt
			if(hole->vert[vert].ref_no[2] != 0 && crsed_course->image_bank.object_pic[4][0])
			{
				if(hole->vert[vert].ref_no[0] < 2)
				{
					for(i = 0; i < (hole->vert[vert].radius*2)/crsed_course->image_bank.object_pic[4][0]->h; i++)
					{
						if(hole->vert[vert].ref_no[0] == 0) draw_masked_bitmap(crsed_course->image_bank.object_pic[4][hole->vert[vert].ref_no[4]/hole->vert[vert].ref_no[2]], bp, 0, 0, hole->s.x+hole->vert[vert].s.x-crsed_course->image_bank.object_pic[4][0]->w/2, hole->s.y+hole->vert[vert].s.y-hole->vert[vert].radius+i*crsed_course->image_bank.object_pic[4][0]->h, crsed_course->image_bank.object_pic[4][0]->w, crsed_course->image_bank.object_pic[4][0]->h);
						else draw_vflip_sprite(bp, crsed_course->image_bank.object_pic[4][hole->vert[vert].ref_no[4]/hole->vert[vert].ref_no[2]], hole->s.x+hole->vert[vert].s.x-crsed_course->image_bank.object_pic[4][0]->w/2, hole->s.y+hole->vert[vert].s.y-hole->vert[vert].radius+i*crsed_course->image_bank.object_pic[4][0]->h);
					}
				}
				else
				{
					for(i = 0; i < (hole->vert[vert].radius*2)/crsed_course->image_bank.object_pic[4][0]->h; i++)
					{
						if(hole->vert[vert].ref_no[0] == 3) draw_rotated_sprite(bp, crsed_course->image_bank.object_pic[4][hole->vert[vert].ref_no[4]/hole->vert[vert].ref_no[2]], hole->s.x+hole->vert[vert].s.x-hole->vert[vert].radius+i*crsed_course->image_bank.object_pic[4][0]->h, hole->s.y+hole->vert[vert].s.y-crsed_course->image_bank.object_pic[4][0]->w/2, itofix(64));
						else draw_rotated_vflip_sprite(bp, crsed_course->image_bank.object_pic[4][hole->vert[vert].ref_no[4]/hole->vert[vert].ref_no[2]], hole->s.x+hole->vert[vert].s.x-hole->vert[vert].radius+i*crsed_course->image_bank.object_pic[4][0]->h, hole->s.y+hole->vert[vert].s.y-crsed_course->image_bank.object_pic[4][0]->w/2, itofix(64));
					}
				}
			}
			break;

		case VERT_FLAG_CLOUD_HORIZ:  //draw the cloud to the poly layer (so it flies overhead..)
		case VERT_FLAG_CLOUD_VERT:  //draw the cloud to the poly layer (so it flies overhead..)
			draw_filled_circle(bp, hole->s.x+hole->vert[vert].s.x, hole->s.y+hole->vert[vert].s.y, hole->vert[vert].radius, hole->vert[vert].ref_no[5]);
			draw_circle(bp, hole->s.x+hole->vert[vert].s.x, hole->s.y+hole->vert[vert].s.y, hole->vert[vert].radius, hole->vert[vert].ref_no[5]-1);
			break;

		case VERT_FLAG_BLINKER:     //draw the flashing blinker light..
			if(hole->vert[vert].ref_no[0])
			{
				if(hole->vert[vert].ref_no[4]) draw_filled_rectangle(bp, hole->s.x+hole->vert[vert].s.x-hole->vert[vert].radius, hole->s.y+hole->vert[vert].s.y-hole->vert[vert].radius, hole->s.x+hole->vert[vert].s.x+hole->vert[vert].radius, hole->s.y+hole->vert[vert].s.y+hole->vert[vert].radius, hole->vert[vert].ref_no[5]);
				else draw_filled_circle(bp, hole->s.x+hole->vert[vert].s.x, hole->s.y+hole->vert[vert].s.y, hole->vert[vert].radius, hole->vert[vert].ref_no[5]);
			}
			break;

		case VERT_FLAG_POLY_DOOR_HORIZ:  //draw the moving doors
		case VERT_FLAG_POLY_DOOR_VERT:  //draw the moving doors
		{
			int lp_no = hole->vert[vert].ref_no[6];
			int rp_no = hole->vert[vert].ref_no[7];
			draw_filled_rectangle(bp, hole->s.x+hole->body[lp_no].poly.p[1].x, hole->s.y+hole->body[lp_no].poly.p[1].y, hole->s.x+hole->body[lp_no].poly.p[3].x, hole->s.y+hole->body[lp_no].poly.p[3].y, hole->vert[vert].ref_no[5]);
			draw_rectangle(bp, hole->s.x+hole->body[lp_no].poly.p[1].x, hole->s.y+hole->body[lp_no].poly.p[1].y, hole->s.x+hole->body[lp_no].poly.p[3].x, hole->s.y+hole->body[lp_no].poly.p[3].y, 1);
			draw_filled_rectangle(bp, hole->s.x+hole->body[rp_no].poly.p[1].x, hole->s.y+hole->body[rp_no].poly.p[1].y, hole->s.x+hole->body[rp_no].poly.p[3].x, hole->s.y+hole->body[rp_no].poly.p[3].y, hole->vert[vert].ref_no[5]);
			draw_rectangle(bp, hole->s.x+hole->body[rp_no].poly.p[1].x, hole->s.y+hole->body[rp_no].poly.p[1].y, hole->s.x+hole->body[rp_no].poly.p[3].x, hole->s.y+hole->body[rp_no].poly.p[3].y, 1);
			break;
		}

		case VERT_FLAG_BUBBLE_HORIZ:
		case VERT_FLAG_BUBBLE_VERT:
			//drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
			draw_circle(bp, hole->s.x+hole->vert[vert].s.x, hole->s.y+hole->vert[vert].s.y, hole->vert[vert].radius, makecol(255, 255, 255));
			draw_filled_circle(bp, hole->s.x+hole->vert[vert].s.x-hole->vert[vert].radius/2, hole->s.y+hole->vert[vert].s.y-hole->vert[vert].radius/2, hole->vert[vert].radius/5, makecol(255, 255, 255));
			//solid_mode();
			break;

		default:
			break;
	}
}

void draw_handle(BITMAP * bp, int x, int y, int selected)
{
	int o = crsed_zoom / 2;
	
	rectfill(bp, x * crsed_zoom - crsed_ox - 2 + o, y * crsed_zoom - crsed_oy - 2 + o, x * crsed_zoom - crsed_ox + 2 + o, y * crsed_zoom - crsed_oy + 2 + o, selected ? makecol(0, 255, 0) : makecol(255, 255, 255));
	rect(bp, x * crsed_zoom - crsed_ox - 2 + o, y * crsed_zoom - crsed_oy - 2 + o, x * crsed_zoom - crsed_ox + 2 + o, y * crsed_zoom - crsed_oy + 2 + o, makecol(0, 0, 0));
}

void crsed_render_vert_handle(BITMAP * bp, HOLE * hole, int vert)
{
	switch(hole->vert[vert].flag)
	{
		
		/* some objects shouldn't be resizeable (make sure program sets proper default size for hole) */
		case VERT_FLAG_TEE:
		case VERT_FLAG_HOLE:
		case VERT_FLAG_TREE_0:
		case VERT_FLAG_TREE_1:
		case VERT_FLAG_LASER_SEND:
		case VERT_FLAG_LASER_RECV:
		{
			draw_handle(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, (crsed_hover_vert == vert || crsed_selected_vert == vert));
			break;
		}
		
		case VERT_FLAG_PIPE_ENTER:
		{
			draw_line(bp, hole->s.x + hole->vert[hole->vert[vert].ref_no[1]].s.x, hole->s.y + hole->vert[hole->vert[vert].ref_no[1]].s.y, hole->s.x + hole->vert[hole->vert[vert].ref_no[1]].s.x + fixtof(fixcos(itofix(hole->vert[vert].ref_no[4]))) * 32.0, hole->s.y + hole->vert[hole->vert[vert].ref_no[1]].s.y + fixtof(fixsin(itofix(hole->vert[vert].ref_no[4]))) * 32.0, makecol(255, 255, 255));
			draw_handle(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, ((crsed_hover_vert == vert && crsed_hover_radius < 0) || (crsed_selected_vert == vert && crsed_selected_radius < 0)));
			draw_circle(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, hole->vert[vert].radius, makecol(255, 255, 255));
			draw_handle(bp, hole->s.x + hole->vert[vert].s.x + hole->vert[vert].radius, hole->s.y + hole->vert[vert].s.y, ((crsed_hover_vert == vert && crsed_hover_radius >= 0) || (crsed_selected_vert == vert && crsed_selected_radius >= 0)));
			break;
		}
		
		case VERT_FLAG_TELEPORT:
		{
			draw_line(bp, hole->s.x + hole->vert[hole->vert[vert].ref_no[0]].s.x, hole->s.y + hole->vert[hole->vert[vert].ref_no[0]].s.y, hole->s.x + hole->vert[hole->vert[vert].ref_no[0]].s.x + fixtof(fixcos(itofix(hole->vert[vert].ref_no[1]))) * 32.0, hole->s.y + hole->vert[hole->vert[vert].ref_no[0]].s.y + fixtof(fixsin(itofix(hole->vert[vert].ref_no[1]))) * 32.0, makecol(255, 255, 255));
			draw_handle(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, ((crsed_hover_vert == vert && crsed_hover_radius < 0) || (crsed_selected_vert == vert && crsed_selected_radius < 0)));
			draw_circle(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, hole->vert[vert].radius, makecol(255, 255, 255));
			draw_handle(bp, hole->s.x + hole->vert[vert].s.x + hole->vert[vert].radius, hole->s.y + hole->vert[vert].s.y, ((crsed_hover_vert == vert && crsed_hover_radius >= 0) || (crsed_selected_vert == vert && crsed_selected_radius >= 0)));
			break;
		}
		
		case VERT_FLAG_LASER_RESPAWN:
		case VERT_FLAG_PIPE_EXIT:
		{
			draw_handle(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, ((crsed_hover_vert == vert && crsed_hover_radius < 0) || (crsed_selected_vert == vert && crsed_selected_radius < 0)));
			draw_circle(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, hole->vert[vert].radius, makecol(255, 255, 255));
			break;
		}
		
		/* normal objects have a position and radius */
		default:
		{
			draw_handle(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, ((crsed_hover_vert == vert && crsed_hover_radius < 0) || (crsed_selected_vert == vert && crsed_selected_radius < 0)));
			draw_circle(bp, hole->s.x + hole->vert[vert].s.x, hole->s.y + hole->vert[vert].s.y, hole->vert[vert].radius, makecol(255, 255, 255));
			draw_handle(bp, hole->s.x + hole->vert[vert].s.x + hole->vert[vert].radius, hole->s.y + hole->vert[vert].s.y, ((crsed_hover_vert == vert && crsed_hover_radius >= 0) || (crsed_selected_vert == vert && crsed_selected_radius >= 0)));
			break;
		}
	}
}

void crsed_poly_render(void)
{
	int i, j;
	int mx, my;
	fixed nangle;
	
	if(crsed_selected_hole >= crsed_course->num_holes)
	{
		return;
	}
	
	if(crsed_course->hole[crsed_selected_hole].layer_pic[0])
	{
		draw_masked_bitmap(crsed_course->hole[crsed_selected_hole].layer_pic[0], crsed_course_screen, 0, 0, 0, 0, crsed_screen->w, crsed_screen->h);
	}
	if(crsed_course->hole[crsed_selected_hole].layer_pic[1])
	{
		draw_masked_bitmap(crsed_course->hole[crsed_selected_hole].layer_pic[1], crsed_course_screen, 0, 0, 0, 0, crsed_screen->w, crsed_screen->h);
	}
	for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_verts; i++)
	{
		crsed_render_vert(crsed_course_screen, &crsed_course->hole[crsed_selected_hole], i);
	}
	for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_polys; i++)
	{
		for(j = 0; j < crsed_course->hole[crsed_selected_hole].body[i].poly.num_vertices - 1; j++)
		{
			draw_line(crsed_course_screen, crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].x + crsed_course->hole[crsed_selected_hole].body[i].pos.x, crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].y + crsed_course->hole[crsed_selected_hole].body[i].pos.y, crsed_course->hole[crsed_selected_hole].body[i].poly.p[j + 1].x + crsed_course->hole[crsed_selected_hole].body[i].pos.x, crsed_course->hole[crsed_selected_hole].body[i].poly.p[j + 1].y + crsed_course->hole[crsed_selected_hole].body[i].pos.y, (i == crsed_selected_poly) ? makecol(255, 255, 0) : makecol(255, 255, 255));
			mx = (crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].x + crsed_course->hole[crsed_selected_hole].body[i].poly.p[j + 1].x) / 2;
			my = (crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].y + crsed_course->hole[crsed_selected_hole].body[i].poly.p[j + 1].y) / 2;
			nangle = fixatan2(itofix(crsed_course->hole[crsed_selected_hole].body[i].poly.p[j + 1].y - crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].y), itofix(crsed_course->hole[crsed_selected_hole].body[i].poly.p[j + 1].x - crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].x)) + itofix(64);
			draw_line(crsed_course_screen, mx + crsed_course->hole[crsed_selected_hole].body[i].pos.x, my + crsed_course->hole[crsed_selected_hole].body[i].pos.y, mx + fixtof(fixcos(nangle)) * 16.0 + crsed_course->hole[crsed_selected_hole].body[i].pos.x, my + fixtof(fixsin(nangle)) * 16.0 + crsed_course->hole[crsed_selected_hole].body[i].pos.y, (i == crsed_selected_poly) ? makecol(255, 255, 0) : makecol(255, 255, 255));
			draw_handle(crsed_course_screen, crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].x + crsed_course->hole[crsed_selected_hole].body[i].pos.x, crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].y + crsed_course->hole[crsed_selected_hole].body[i].pos.y, ((i == crsed_hover_poly && j == crsed_hover_point) || (i == crsed_selected_poly && j == crsed_selected_point)));
		}
		draw_handle(crsed_course_screen, crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].x + crsed_course->hole[crsed_selected_hole].body[i].pos.x, crsed_course->hole[crsed_selected_hole].body[i].poly.p[j].y + crsed_course->hole[crsed_selected_hole].body[i].pos.y, ((i == crsed_hover_poly && (j == crsed_hover_point || crsed_hover_point == 0)) || (i == crsed_selected_poly && (j == crsed_selected_point || crsed_selected_point == 0))));
	}
}

void crsed_vert_render(void)
{
	int i;
	
	if(crsed_selected_hole >= crsed_course->num_holes)
	{
		return;
	}
	if(crsed_course->hole[crsed_selected_hole].layer_pic[0])
	{
		draw_masked_bitmap(crsed_course->hole[crsed_selected_hole].layer_pic[0], crsed_course_screen, 0, 0, 0, 0, crsed_screen->w, crsed_screen->h);
	}
	if(crsed_course->hole[crsed_selected_hole].layer_pic[1])
	{
		draw_masked_bitmap(crsed_course->hole[crsed_selected_hole].layer_pic[1], crsed_course_screen, 0, 0, 0, 0, crsed_screen->w, crsed_screen->h);
	}
	for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_verts; i++)
	{
		crsed_render_vert(crsed_course_screen, &crsed_course->hole[crsed_selected_hole], i);
	}
	if(crsed_vert_mode == 1 && (crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag == VERT_FLAG_POLY_MORPH ||crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag == VERT_FLAG_POLY_MOVE_HORIZ ||crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag == VERT_FLAG_POLY_MOVE_VERT || crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag == VERT_FLAG_POLY_DOOR_HORIZ || crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag == VERT_FLAG_POLY_DOOR_VERT || crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag == VERT_FLAG_CONVEYOR))
	{
		crsed_poly_render();
	}
	else
	{
		for(i = 0; i < crsed_course->hole[crsed_selected_hole].num_verts; i++)
		{
			crsed_render_vert_handle(crsed_course_screen, &crsed_course->hole[crsed_selected_hole], i);
		}
	}
}

void crsed_image_render(void)
{
	if(crsed_current_image)
	{
		draw_sprite(crsed_course_screen, *crsed_current_image, 320 - (*crsed_current_image)->w / 2, 240 - (*crsed_current_image)->h / 2);
	}
}

void crsed_render(void)
{
	int type_index = 0;
	int i;
	
	clear_to_color(crsed_screen, makecol(0, 0, 64));
	blit(crsed_menu_image, crsed_screen, 0, 0, 0, 0, crsed_screen->w, crsed_screen->h);
	hline(crsed_screen, 0, 496, 639, makecol(255, 255, 255));
	rectfill(crsed_screen, 0, 497, 639, 511, makecol(227, 227, 227));
	
	if(crsed_course)
	{
		switch(crsed_mode)
		{
			case 0:
			{
				crsed_poly_render();
				break;
			}
			case 1:
			{
				crsed_vert_render();
				break;
			}
			case 2:
			{
				crsed_image_render();
				break;
			}
		}
		if(crsed_mode == 1 && crsed_vert_mode == 1)
		{
			textprintf_ex(crsed_screen, font, 4, 500, makecol(0, 0, 0), -1, "Click object to connect to...");
		}
		else if(crsed_mode == 1 && crsed_vert_mode == 2)
		{
			textprintf_ex(crsed_screen, font, 4, 500, makecol(0, 0, 0), -1, "Click to set angle...");
		}
		else if(crsed_mode == 2)
		{
			if(!(*crsed_current_image))
			{
				textprintf_ex(crsed_screen, font, 4, 500, makecol(0, 0, 0), -1, "Image: %d [%s] [Not Loaded]", crsed_selected_image, crsed_image_type_name[crsed_selected_image]);
			}
			else
			{
				textprintf_ex(crsed_screen, font, 4, 500, makecol(0, 0, 0), -1, "Image: %d [%s] (%dx%d)", crsed_selected_image, crsed_image_type_name[crsed_selected_image], (*crsed_current_image)->w,  (*crsed_current_image)->h);
			}
		}
		else
		{
			if(crsed_selected_hole >= crsed_course->num_holes)
			{
				textprintf_ex(crsed_screen, font, 4, 500, makecol(0, 0, 0), -1, "No holes. Use \"Hole->Add\" to create a hole.");
			}
			else
			{
				textprintf_ex(crsed_screen, font, 4, 500, makecol(0, 0, 0), -1, "Hole: %d/%d", crsed_selected_hole + 1, crsed_course->num_holes);
				textprintf_ex(crsed_screen, font, 4 + 128, 500, makecol(0, 0, 0), -1, "Par: %d", crsed_course->hole[crsed_selected_hole].par_score);
				if(crsed_mode == 0)
				{
					textprintf_ex(crsed_screen, font, 4 + 256, 500, makecol(0, 0, 0), -1, "Polygon: %d [Vertex: %d]", crsed_selected_poly, crsed_selected_point);
				}
				else 
				{
					for(i = 0; i < 37; i++)
					{
						if(crsed_object_type[i] == crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag)
						{
							type_index = i;
							break;
						}
					}
					textprintf_ex(crsed_screen, font, 4 + 256, 500, makecol(0, 0, 0), -1, "Object: %d [Radius: %d] [Type: %s]", crsed_selected_vert, crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].radius, crsed_object_type_name[type_index]);
				}
			}
		}
	}
	
	IdleUntilVSync();
	vsync();
	DoneVSync();
	
	blit(crsed_screen, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

int main(int argc, char * argv[])
{
	int updated = 0;
	if(!crsed_initialize(argc, argv))
	{
		return -1;
	}
	while(!crsed_quit)
	{
		/* frame skip mode */
		while(gametime_get_frames() - gametime_tick > 0)
		{
			crsed_logic();
			updated = 0;
			++gametime_tick;
		}
		
		/* update and draw the screen */
		if(!updated)
		{
			crsed_render();
			updated = 1;
		}
		else
		{
			IdleUntilVSync();
			vsync();
			DoneVSync();
		}
	}
	crsed_exit();
	return 0;
}
END_OF_MAIN()
