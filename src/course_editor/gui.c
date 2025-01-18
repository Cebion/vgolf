#include <allegro.h>
#include "../modules/agup/agup.h"
#include "../modules/ncdgui.h"
#include "gui.h"
#include "guiproc.h"
#include "main.h"

char crsed_help_text[8192] = {0};
char crsed_edit_text[4][1024] = {{0}};

MENU crsed_file_menu[] =
{
    {"&New\tF4", crsed_menu_file_new, NULL, 0, NULL},
    {"&Open\tF3", crsed_menu_file_load, NULL, 0, NULL},
    {"&Save\tF2", crsed_menu_file_save, NULL, D_DISABLED, NULL},
    {"Save &As\tCtrl+F2", crsed_menu_file_save_as, NULL, D_DISABLED, NULL},
    {"", NULL, NULL, 0, NULL},
    {"Load &Palette", crsed_menu_file_load_palette, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"Course Tags", crsed_menu_file_course_tags, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"E&xit\tEsc", crsed_menu_file_exit, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_hole_select_menu[] =
{
    {"1", crsed_menu_hole_select_0, NULL, 0, NULL},
    {"2", crsed_menu_hole_select_1, NULL, 0, NULL},
    {"3", crsed_menu_hole_select_2, NULL, 0, NULL},
    {"4", crsed_menu_hole_select_3, NULL, 0, NULL},
    {"5", crsed_menu_hole_select_4, NULL, 0, NULL},
    {"6", crsed_menu_hole_select_5, NULL, 0, NULL},
    {"7", crsed_menu_hole_select_6, NULL, 0, NULL},
    {"8", crsed_menu_hole_select_7, NULL, 0, NULL},
    {"9", crsed_menu_hole_select_8, NULL, 0, NULL},
    {"10", crsed_menu_hole_select_9, NULL, 0, NULL},
    {"11", crsed_menu_hole_select_10, NULL, 0, NULL},
    {"12", crsed_menu_hole_select_11, NULL, 0, NULL},
    {"13", crsed_menu_hole_select_12, NULL, 0, NULL},
    {"14", crsed_menu_hole_select_13, NULL, 0, NULL},
    {"15", crsed_menu_hole_select_14, NULL, 0, NULL},
    {"16", crsed_menu_hole_select_15, NULL, 0, NULL},
    {"17", crsed_menu_hole_select_16, NULL, 0, NULL},
    {"18", crsed_menu_hole_select_17, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_hole_background_menu[] =
{
    {"Load", crsed_menu_hole_background_load, NULL, 0, NULL},
    {"Save", crsed_menu_hole_background_save, NULL, 0, NULL},
    {"Remove", crsed_menu_hole_background_remove, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_hole_foreground_menu[] =
{
    {"Load", crsed_menu_hole_foreground_load, NULL, 0, NULL},
    {"Save", crsed_menu_hole_foreground_save, NULL, 0, NULL},
    {"Remove", crsed_menu_hole_foreground_remove, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_hole_menu[] =
{
    {"&Select", NULL, crsed_hole_select_menu, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"Add", crsed_menu_hole_add, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"Background", NULL, crsed_hole_background_menu, 0, NULL},
    {"Foreground", NULL, crsed_hole_foreground_menu, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"Set Par", crsed_menu_hole_set_par, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"Import", crsed_menu_hole_import, NULL, 0, NULL},
    {"Export", crsed_menu_hole_export, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"Delete", crsed_menu_hole_delete, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_mode_menu[] =
{
    {"&Polygons", crsed_menu_mode_polygons, NULL, 0, NULL},
    {"&Objects", crsed_menu_mode_objects, NULL, 0, NULL},
    {"&Images", crsed_menu_mode_images, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_object_type_menu[] = 
{
    {"TEE", crsed_menu_object_type_TEE, NULL, 0, NULL},
    {"CUP", crsed_menu_object_type_CUP, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"TREE_0", crsed_menu_object_type_TREE_0, NULL, 0, NULL},
    {"TREE_1", crsed_menu_object_type_TREE_1, NULL, 0, NULL},
    {"ANIMAL_0", crsed_menu_object_type_ANIMAL_0, NULL, 0, NULL},
    {"ANIMAL_1", crsed_menu_object_type_ANIMAL_1, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"HAZARD_WATER", crsed_menu_object_type_HAZARD_WATER, NULL, 0, NULL},
    {"HAZARD_SAND", crsed_menu_object_type_HAZARD_SAND, NULL, 0, NULL},
    {"HAZARD_ROUGH", crsed_menu_object_type_HAZARD_ROUGH, NULL, 0, NULL},
    {"HAZARD_SLIME", crsed_menu_object_type_HAZARD_SLIME, NULL, 0, NULL},
    {"HILL", crsed_menu_object_type_HILL, NULL, 0, NULL},
    {"RUT", crsed_menu_object_type_RUT, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"PIPE_ENTER", crsed_menu_object_type_PIPE_ENTER, NULL, 0, NULL},
    {"PIPE_EXIT", crsed_menu_object_type_PIPE_EXIT, NULL, 0, NULL},
    {"TELEPORT", crsed_menu_object_type_TELEPORT, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"CLOUD_HORIZ", crsed_menu_object_type_CLOUD_HORIZ, NULL, 0, NULL},
    {"CLOUD_VERT", crsed_menu_object_type_CLOUD_VERT, NULL, 0, NULL},
    {"BUBBLE_HORIZ", crsed_menu_object_type_BUBBLE_HORIZ, NULL, 0, NULL},
    {"BUBBLE_VERT", crsed_menu_object_type_BUBBLE_VERT, NULL, 0, NULL},
    {"BLINKER", crsed_menu_object_type_BLINKER, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"LASER_SEND", crsed_menu_object_type_LASER_SEND, NULL, 0, NULL},
    {"LASER_RECEIVE", crsed_menu_object_type_LASER_RECEIVE, NULL, 0, NULL},
    {"LASER_RESPAWN", crsed_menu_object_type_LASER_RESPAWN, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"POLY_MORPH", crsed_menu_object_type_POLY_MORPH, NULL, 0, NULL},
    {"POLY_MOTION_AREA", crsed_menu_object_type_POLY_MOTION_AREA, NULL, 0, NULL},
    {"POLY_MOVE_HORIZ", crsed_menu_object_type_POLY_MOVE_HORIZ, NULL, 0, NULL},
    {"POLY_MOVE_VERT", crsed_menu_object_type_POLY_MOVE_VERT, NULL, 0, NULL},
    {"DOOR_HORIZ", crsed_menu_object_type_DOOR_HORIZ, NULL, 0, NULL},
    {"DOOR_VERTICAL", crsed_menu_object_type_DOOR_VERTICAL, NULL, 0, NULL},
    {"DOOR_RESPAWN", crsed_menu_object_type_DOOR_RESPAWN, NULL, 0, NULL},
    {"CONVEYOR", crsed_menu_object_type_CONVEYOR, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_poly_menu[] =
{
    {"&Invert", crsed_menu_poly_invert, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"&Delete\tCtrl+Del", crsed_menu_poly_delete, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_object_properties_menu[] =
{
    {"&Color", crsed_menu_object_properties_color, NULL, 0, NULL},
    {"C&onnect", crsed_menu_object_properties_connect, NULL, 0, NULL},
    {"Set &Angle", crsed_menu_object_properties_set_angle, NULL, 0, NULL},
    {"Set &Timer", crsed_menu_object_properties_set_timer, NULL, 0, NULL},
    {"Set Timer &2", crsed_menu_object_properties_set_timer2, NULL, 0, NULL},
    {"Set S&peed", crsed_menu_object_properties_set_speed, NULL, 0, NULL},
    {"Set Si&ze", crsed_menu_object_properties_set_size, NULL, 0, NULL},
    {"Set &Direction", crsed_menu_object_properties_set_direction, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_object_menu[] =
{
    {"&Type", NULL, crsed_object_type_menu, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"&Properties", NULL, crsed_object_properties_menu, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"&Delete\tDel", NULL, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_image_type_end_menu[] = 
{
    {"BG", crsed_menu_image_type_end_bg, NULL, 0, NULL},
    {"Podium", crsed_menu_image_type_end_podium, NULL, 0, NULL},
    {"Cloud 1", crsed_menu_image_type_end_cloud_1, NULL, 0, NULL},
    {"Cloud 2", crsed_menu_image_type_end_cloud_2, NULL, 0, NULL},
    {"1st Trophy", crsed_menu_image_type_1st_trophy, NULL, 0, NULL},
    {"2nd Trophy", crsed_menu_image_type_2nd_trophy, NULL, 0, NULL},
    {"3rd Trophy", crsed_menu_image_type_3rd_trophy, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_image_type_object_menu[] = 
{
    {"Hole", crsed_menu_image_type_object_hole, NULL, 0, NULL},
    {"Hole Sunk", crsed_menu_image_type_object_hole_sunk, NULL, 0, NULL},
    {"Tree 1", crsed_menu_image_type_object_tree_1, NULL, 0, NULL},
    {"Tree 2", crsed_menu_image_type_object_tree_2, NULL, 0, NULL},
    {"Laser Off", crsed_menu_image_type_object_laser_1, NULL, 0, NULL},
    {"Laser On", crsed_menu_image_type_object_laser_2, NULL, 0, NULL},
    {"Teleport 1", crsed_menu_image_type_object_teleport_1, NULL, 0, NULL},
    {"Teleport 2", crsed_menu_image_type_object_teleport_2, NULL, 0, NULL},
    {"Conveyor 1", crsed_menu_image_type_object_conveyor_1, NULL, 0, NULL},
    {"Conveyor 2", crsed_menu_image_type_object_conveyor_2, NULL, 0, NULL},
    {"Conveyor 3", crsed_menu_image_type_object_conveyor_3, NULL, 0, NULL},
    {"Conveyor 4", crsed_menu_image_type_object_conveyor_4, NULL, 0, NULL},
    {"Animal 1 Right 1", crsed_menu_image_type_object_animal_1_1, NULL, 0, NULL},
    {"Animal 1 Right 2", crsed_menu_image_type_object_animal_1_2, NULL, 0, NULL},
    {"Animal 1 Left 1", crsed_menu_image_type_object_animal_1_3, NULL, 0, NULL},
    {"Animal 1 Left 2", crsed_menu_image_type_object_animal_1_4, NULL, 0, NULL},
    {"Animal 2 Right 1", crsed_menu_image_type_object_animal_2_1, NULL, 0, NULL},
    {"Animal 2 Right 2", crsed_menu_image_type_object_animal_2_2, NULL, 0, NULL},
    {"Animal 2 Left 1", crsed_menu_image_type_object_animal_2_3, NULL, 0, NULL},
    {"Animal 2 Left 2", crsed_menu_image_type_object_animal_2_4, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_image_type_menu[] = 
{
    {"Title", crsed_menu_image_type_title, NULL, 0, NULL},
    {"Scoreboard", crsed_menu_image_type_scoreboard, NULL, 0, NULL},
    {"Ending", NULL, crsed_image_type_end_menu, 0, NULL},
    {"Object", NULL, crsed_image_type_object_menu, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_image_menu[] =
{
    {"&Type", NULL, crsed_image_type_menu, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"&Load", crsed_menu_image_load, NULL, 0, NULL},
    {"&Save", crsed_menu_image_save, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_help_menu[] =
{
    {"&Manual\tF1", crsed_menu_help_manual, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"&About", crsed_menu_help_about, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU crsed_main_menu[] =
{
    {"&File", NULL, crsed_file_menu, 0, NULL},
    {"&Mode", NULL, crsed_mode_menu, 0, NULL},
    {"&Hole", NULL, crsed_hole_menu, 0, NULL},
    {"&Polygon", NULL, crsed_poly_menu, 0, NULL},
    {"&Object", NULL, crsed_object_menu, 0, NULL},
    {"&Image", NULL, crsed_image_menu, 0, NULL},
    {"Help", NULL, crsed_help_menu, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

DIALOG crsed_main_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_menu_proc, 0, 0, 640, 16, 0, 0, 0, 0, 0, 0, crsed_main_menu, NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG crsed_help_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  640, 512, 2,   23,  0,    0,      0,   0,   "vGolf Course Editor Help",               NULL, NULL },
   { d_agup_textbox_proc,   8,  24,  624, 40 + 380 + 24,  2,   23,  0,    0,      0,   0,   crsed_help_text, NULL, NULL },
   { d_agup_button_proc, 8,  404 + 56 + 16, 624,  28, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG crsed_palette_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  144, 200, 2,   23,  0,    0,      0,   0,   "Properties",               NULL, NULL },
   { d_crsed_palette_proc,   8,  24,  128, 128,  2,   23,  0,    0,      3,   0,   NULL, NULL, NULL },
   { d_agup_button_proc, 8,  160, 128,  32, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG crsed_timer_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  120, 88, 2,   23,  0,    0,      0,   0,   "Properties",               NULL, NULL },
   { d_agup_text_proc,   8,  24 + 4,  48, 8,  2,   23,  0,    0,      0,   0,   "Timer:", NULL, NULL },
   { d_agup_edit_proc,   64,  24,  48, 8,  2,   23,  0,    0,      3,   0,   crsed_edit_text[0], NULL, NULL },
   { d_agup_button_proc, 8,  48, 104,  32, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG crsed_speed_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  120, 88, 2,   23,  0,    0,      0,   0,   "Properties",               NULL, NULL },
   { d_agup_text_proc,   8,  24 + 4,  48, 8,  2,   23,  0,    0,      0,   0,   "Speed:", NULL, NULL },
   { d_agup_edit_proc,   64,  24,  48, 8,  2,   23,  0,    0,      3,   0,   crsed_edit_text[0], NULL, NULL },
   { d_agup_button_proc, 8,  48, 104,  32, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG crsed_speed2_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  136, 112, 2,   23,  0,    0,      0,   0,   "Properties",               NULL, NULL },
   { d_agup_text_proc,   8,  24 + 4,  64, 8,  2,   23,  0,    0,      0,   0,   "X Speed:", NULL, NULL },
   { d_agup_edit_proc,   72,  24,  56, 8,  2,   23,  0,    0,      3,   0,   crsed_edit_text[0], NULL, NULL },
   { d_agup_text_proc,   8,  48 + 4,  64, 8,  2,   23,  0,    0,      0,   0,   "Y Speed:", NULL, NULL },
   { d_agup_edit_proc,   72,  48,  56, 8,  2,   23,  0,    0,      3,   0,   crsed_edit_text[1], NULL, NULL },
   { d_agup_button_proc, 8,  72, 120,  32, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG crsed_tags_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  376, 136, 2,   23,  0,    0,      0,   0,   "Tags",               NULL, NULL },
   { d_agup_text_proc,   8,  24 + 4,  96, 8,  2,   23,  0,    0,      0,   0,   "Course Name:", NULL, NULL },
   { d_agup_edit_proc,   112,  24,  256, 8,  2,   23,  0,    0,      128,   0,   crsed_edit_text[0], NULL, NULL },
   { d_agup_text_proc,   8,  48 + 4,  96, 8,  2,   23,  0,    0,      0,   0,   "Author:", NULL, NULL },
   { d_agup_edit_proc,   112,  48,  256, 8,  2,   23,  0,    0,      256,   0,   crsed_edit_text[1], NULL, NULL },
   { d_agup_text_proc,   8,  72 + 4,  96, 8,  2,   23,  0,    0,      0,   0,   "Comment:", NULL, NULL },
   { d_agup_edit_proc,   112,  72,  256, 8,  2,   23,  0,    0,      1024,   0,   crsed_edit_text[2], NULL, NULL },
   { d_agup_button_proc, 8,  96, 360,  32, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG crsed_direction_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  136, 160, 2,   23,  0,    0,      0,   0,   "Properties",               NULL, NULL },
   { d_agup_radio_proc,   24,  24,  80, 8,  2,   23,  0,    0,      0,   0,   "Up", NULL, NULL },
   { d_agup_radio_proc,   24,  48,  80, 8,  2,   23,  0,    0,      0,   0,   "Down", NULL, NULL },
   { d_agup_radio_proc,   24,  72,  80, 8,  2,   23,  0,    0,      0,   0,   "Left", NULL, NULL },
   { d_agup_radio_proc,   24,  96,  80, 8,  2,   23,  0,    0,      0,   0,   "Right", NULL, NULL },
   { d_agup_button_proc, 8,  120, 120,  32, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG crsed_size_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  120, 88, 2,   23,  0,    0,      0,   0,   "Properties",               NULL, NULL },
   { d_agup_text_proc,   8,  24 + 4,  48, 8,  2,   23,  0,    0,      0,   0,   "Size:", NULL, NULL },
   { d_agup_edit_proc,   64,  24,  48, 8,  2,   23,  0,    0,      3,   0,   crsed_edit_text[0], NULL, NULL },
   { d_agup_button_proc, 8,  48, 104,  32, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG crsed_par_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  120, 88, 2,   23,  0,    0,      0,   0,   "Hole",               NULL, NULL },
   { d_agup_text_proc,   8,  24 + 4,  48, 8,  2,   23,  0,    0,      0,   0,   "Par:", NULL, NULL },
   { d_agup_edit_proc,   64,  24,  48, 8,  2,   23,  0,    0,      3,   0,   crsed_edit_text[0], NULL, NULL },
   { d_agup_button_proc, 8,  48, 104,  32, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

void crsed_prepare_menus(void)
{
	int i;
	
	if(crsed_course)
	{
		/* main menu */
		crsed_main_menu[1].flags = 0;
		crsed_main_menu[2].flags = 0;
		if(crsed_selected_hole < crsed_course->num_holes)
		{
			if(crsed_mode == 0 && crsed_selected_poly < crsed_course->hole[crsed_selected_hole].num_polys)
			{
				crsed_main_menu[3].flags = 0;
			}
			else
			{
				crsed_main_menu[3].flags = D_DISABLED;
			}
			if(crsed_mode == 1)
			{
				crsed_main_menu[4].flags = 0;
			}
			else
			{
				crsed_main_menu[4].flags = D_DISABLED;
			}
		}
		else
		{
			crsed_main_menu[3].flags = D_DISABLED;
			crsed_main_menu[4].flags = D_DISABLED;
		}
		if(crsed_mode == 2)
		{
			crsed_main_menu[5].flags = 0;
		}
		else
		{
			crsed_main_menu[5].flags = D_DISABLED;
		}
		
		/* file menu */
		crsed_file_menu[2].flags = 0;
		crsed_file_menu[3].flags = 0;
		crsed_file_menu[5].flags = 0;
		crsed_file_menu[7].flags = 0;
		
		/* mode menu */
		crsed_mode_menu[0].flags = 0;
		crsed_mode_menu[1].flags = 0;
		crsed_mode_menu[2].flags = 0;
		crsed_mode_menu[crsed_mode].flags = D_SELECTED;
		
		/* hole menu */
		if(crsed_course->num_holes > 0)
		{
			for(i = 0; i < 18; i++)
			{
				crsed_hole_select_menu[i].flags = D_DISABLED;
			}
			for(i = 0; i < crsed_course->num_holes; i++)
			{
				crsed_hole_select_menu[i].flags = 0;
			}
			crsed_hole_select_menu[crsed_selected_hole].flags |= D_SELECTED;
			crsed_hole_menu[0].flags = 0;
			if(crsed_course->num_holes < 18)
			{
				crsed_hole_menu[2].flags = 0;
			}
			else
			{
				crsed_hole_menu[2].flags = D_DISABLED;
			}
			crsed_hole_menu[4].flags = 0;
			if(crsed_course->hole[crsed_selected_hole].layer_pic[0])
			{
				crsed_hole_background_menu[1].flags = 0;
				crsed_hole_background_menu[2].flags = 0;
			}
			else
			{
				crsed_hole_background_menu[1].flags = D_DISABLED;
				crsed_hole_background_menu[2].flags = D_DISABLED;
			}
			crsed_hole_menu[5].flags = 0;
			if(crsed_course->hole[crsed_selected_hole].layer_pic[1])
			{
				crsed_hole_foreground_menu[1].flags = 0;
				crsed_hole_foreground_menu[2].flags = 0;
			}
			else
			{
				crsed_hole_foreground_menu[1].flags = D_DISABLED;
				crsed_hole_foreground_menu[2].flags = D_DISABLED;
			}
			crsed_hole_menu[7].flags = 0;
			crsed_hole_menu[9].flags = 0;
			crsed_hole_menu[10].flags = 0;
			crsed_hole_menu[12].flags = 0;
		}
		else
		{
			crsed_hole_menu[0].flags = D_DISABLED;
			crsed_hole_menu[2].flags = 0;
			crsed_hole_menu[4].flags = D_DISABLED;
			crsed_hole_menu[5].flags = D_DISABLED;
			crsed_hole_menu[7].flags = D_DISABLED;
			crsed_hole_menu[9].flags = D_DISABLED;
			crsed_hole_menu[10].flags = D_DISABLED;
			crsed_hole_menu[12].flags = D_DISABLED;
		}
		
		/* object menu */
		if(crsed_selected_vert < crsed_course->hole[crsed_selected_hole].num_verts)
		{
			crsed_object_menu[2].flags = 0;
			crsed_object_menu[4].flags = 0;
		}
		else
		{
			crsed_object_menu[2].flags = D_DISABLED;
			crsed_object_menu[4].flags = D_DISABLED;
		}
		for(i = 0; i < 37; i++)
		{
			crsed_object_type_menu[i].flags = 0;
		}
		for(i = 0; i < 37; i++)
		{
			if(crsed_selected_vert_type == crsed_object_type[i])
			{
				crsed_object_type_menu[i].flags = D_SELECTED;
				break;
			}
		}
		crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_COLOR].flags = D_DISABLED;
		crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_CONNECT].flags = D_DISABLED;
		crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_ANGLE].flags = D_DISABLED;
		crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_TIMER].flags = D_DISABLED;
		crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_TIMER2].flags = D_DISABLED;
		crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_SPEED].flags = D_DISABLED;
		crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_SIZE].flags = D_DISABLED;
		crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_DIRECTION].flags = D_DISABLED;
		switch(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag)
		{
			case VERT_FLAG_PIPE_ENTER:
			{
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_CONNECT].flags = 0;
				if(crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[1]].flag == VERT_FLAG_PIPE_EXIT)
				{
					crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_ANGLE].flags = 0;
					crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_TIMER].flags = 0;
				}
				break;
			}
			case VERT_FLAG_POLY_MORPH:
			{
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_COLOR].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_CONNECT].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_SPEED].flags = 0;
				break;
			}
			case VERT_FLAG_POLY_MOVE_HORIZ:
			case VERT_FLAG_POLY_MOVE_VERT:
			{
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_COLOR].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_CONNECT].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_SPEED].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_SIZE].flags = 0;
				break;
			}
			case VERT_FLAG_CLOUD_HORIZ:
			case VERT_FLAG_CLOUD_VERT:
			{
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_COLOR].flags = 0;
				break;
			}
			case VERT_FLAG_LASER_SEND:
			{
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_COLOR].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_CONNECT].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_TIMER].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_TIMER2].flags = 0;
				break;
			}
			case VERT_FLAG_TELEPORT:
			{
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_CONNECT].flags = 0;
				if(crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[0]].flag == VERT_FLAG_TELEPORT)
				{
					crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_ANGLE].flags = 0;
				}
				break;
			}
			case VERT_FLAG_BLINKER:
			{
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_COLOR].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_TIMER].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_TIMER2].flags = 0;
				break;
			}
			case VERT_FLAG_POLY_DOOR_HORIZ:
			case VERT_FLAG_POLY_DOOR_VERT:
			{
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_COLOR].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_CONNECT].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_SPEED].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_TIMER].flags = 0;
//				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_TIMER2].flags = 0;
				break;
			}
			case VERT_FLAG_CONVEYOR:
			{
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_SPEED].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_CONNECT].flags = 0;
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_DIRECTION].flags = 0;
				break;
			}
			case VERT_FLAG_POLY_MOTION_AREA:
			{
				crsed_object_properties_menu[CRSED_GUI_OBJECT_PROPERTIES_CONNECT].flags = 0;
				break;
			}
		}
		
		/* image menu */
		for(i = 0; i < 4; i++)
		{
			crsed_image_type_menu[i].flags = 0;
		}
		for(i = 0; i < 7; i++)
		{
			crsed_image_type_end_menu[i].flags = 0;
		}
		for(i = 0; i < 20; i++)
		{
			crsed_image_type_object_menu[i].flags = 0;
		}
		if(crsed_selected_image < 2)
		{
			crsed_image_type_menu[crsed_selected_image].flags = D_SELECTED;
		}
		else if(crsed_selected_image >= 2 && crsed_selected_image < 9)
		{
			crsed_image_type_menu[2].flags = D_SELECTED;
			crsed_image_type_end_menu[crsed_selected_image - 2].flags = D_SELECTED;
		}
		else
		{
			crsed_image_type_menu[3].flags = D_SELECTED;
			crsed_image_type_object_menu[crsed_selected_image - 9].flags = D_SELECTED;
		}
	}
	else
	{
		crsed_main_menu[1].flags = D_DISABLED;
		crsed_main_menu[2].flags = D_DISABLED;
		crsed_main_menu[3].flags = D_DISABLED;
		crsed_main_menu[4].flags = D_DISABLED;
		crsed_main_menu[5].flags = D_DISABLED;
		crsed_file_menu[2].flags = D_DISABLED;
		crsed_file_menu[3].flags = D_DISABLED;
		crsed_file_menu[5].flags = D_DISABLED;
		crsed_file_menu[7].flags = D_DISABLED;
	}
	if(crsed_menu_image)
	{
		destroy_bitmap(crsed_menu_image);
	}
	crsed_menu_image = ncdgui_generate_image(crsed_main_dialog);
}
