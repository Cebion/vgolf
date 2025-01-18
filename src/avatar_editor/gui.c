#include <allegro.h>
#include "../modules/agup/agup.h"
#include "../modules/ncdgui.h"
#include "guiproc.h"
#include "main.h"
#include "undo.h"

char ae_help_text[4096] = {0};

MENU ae_file_menu[] =
{
    {"&New\tF4", ae_menu_file_new, NULL, 0, NULL},
    {"&Open\tF3", ae_menu_file_load, NULL, 0, NULL},
    {"&Save\tF2", ae_menu_file_save, NULL, D_DISABLED, NULL},
    {"Save &As\tCtrl+F2", ae_menu_file_save_as, NULL, D_DISABLED, NULL},
    {"", NULL, NULL, 0, NULL},
    {"Load &Image\tF5", ae_menu_file_load_image, NULL, D_DISABLED, NULL},
    {"", NULL, NULL, 0, NULL},
    {"E&xit\tEsc", ae_menu_file_exit, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU ae_edit_menu[] =
{
    {"&Undo\tCtrl+Z", ae_menu_edit_undo, NULL, D_DISABLED, NULL},
    {"&Redo\tCtrl+Y", ae_menu_edit_redo, NULL, D_DISABLED, NULL},
    {"", NULL, NULL, 0, NULL},
    {"&Copy\tCtrl+C", ae_menu_edit_copy, NULL, 0, NULL},
    {"&Paste\tCtrl+V", ae_menu_edit_paste, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU ae_image_turn_menu[] =
{
    {"&Clockwise\tRight", ae_menu_image_turn_clockwise, NULL, 0, NULL},
    {"&Counter-Clockwise\tLeft", ae_menu_image_turn_counter_clockwise, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU ae_image_shift_menu[] =
{
    {"&Left\tA", ae_menu_image_shift_left, NULL, 0, NULL},
    {"&Right\tD", ae_menu_image_shift_right, NULL, 0, NULL},
    {"&Up\tW", ae_menu_image_shift_up, NULL, 0, NULL},
    {"&Down\tS", ae_menu_image_shift_down, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU ae_image_menu[] =
{
    {"&Mirror\tH", ae_menu_image_mirror, NULL, 0, NULL},
    {"&Flip\tV", ae_menu_image_flip, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"&Turn", NULL, ae_image_turn_menu, 0, NULL},
    {"&Shift", NULL, ae_image_shift_menu, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU ae_help_menu[] =
{
    {"&Manual\tF1", ae_menu_help_manual, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"&About", ae_menu_help_about, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU ae_main_menu[] =
{
    {"&File", NULL, ae_file_menu, 0, NULL},
    {"&Edit", NULL, ae_edit_menu, D_DISABLED, NULL},
    {"&Image", NULL, ae_image_menu, D_DISABLED, NULL},
    {"&Help", NULL, ae_help_menu, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

DIALOG ae_main_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_menu_proc, 0, 0, 408, 16, 0, 0, 0, 0, 0, 0, ae_main_menu, NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG ae_help_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  408, 440, 2,   23,  0,    0,      0,   0,   "vGolf Avatar Editor Help",               NULL, NULL },
   { d_agup_textbox_proc,   8,  32,  392, 372 - 8,  2,   23,  0,    0,      0,   0,   ae_help_text, NULL, NULL },
   { d_agup_button_proc, 8,  404, 392,  28, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

void ae_prepare_menus(void)
{
	if(ae_avatar)
	{
		ae_file_menu[2].flags = 0;
		ae_file_menu[3].flags = 0;
		ae_file_menu[5].flags = 0;
		ae_main_menu[1].flags = 0;
		ae_main_menu[2].flags = 0;
	}
	else
	{
		ae_file_menu[2].flags = D_DISABLED;
		ae_file_menu[3].flags = D_DISABLED;
		ae_file_menu[5].flags = D_DISABLED;
		ae_main_menu[1].flags = D_DISABLED;
		ae_main_menu[2].flags = D_DISABLED;
	}
	if(ae_undo_count > 0)
	{
		ae_edit_menu[0].flags = 0;
	}
	else
	{
		ae_edit_menu[0].flags = D_DISABLED;
	}
	if(ae_redo_count)
	{
		ae_edit_menu[1].flags = 0;
	}
	else
	{
		ae_edit_menu[1].flags = D_DISABLED;
	}
	if(ae_selecting)
	{
		ae_edit_menu[3].flags = 0;
	}
	else
	{
		ae_edit_menu[3].flags = D_DISABLED;
	}
	if(ae_clipboard)
	{
		ae_edit_menu[4].flags = 0;
	}
	else
	{
		ae_edit_menu[4].flags = D_DISABLED;
	}
	if(ae_menu_image)
	{
		destroy_bitmap(ae_menu_image);
	}
	ae_menu_image = ncdgui_generate_image(ae_main_dialog);
}
