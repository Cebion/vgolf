#include <allegro.h>
#include "../modules/wfsel.h"
#include "../modules/g-idle.h"
#include "../modules/ncdgui.h"
#include "../modules/gametime.h"
#include "../modules/fx.h"
#include "../game/avatar.h"
#include "main.h"
#include "gui.h"
#include "guiproc.h"
#include "undo.h"

int ae_quit = 0;
BITMAP * ae_menu_image = NULL;
BITMAP * ae_screen = NULL;
BITMAP * ae_zoom_screen = NULL;
BITMAP * ae_scratch = NULL;
BITMAP * ae_clipboard = NULL;
BITMAP * ae_float = NULL;
BITMAP * ae_float_bg = NULL;
PALETTE ae_palette;
PALETTE ae_temp_palette;
char ae_filename[1024] = {0};
char ae_image_filename[1024] = {0};
VGOLF_AVATAR * ae_avatar = NULL;
int ae_selected_image = 0;
char * ae_avatar_type[4] = {"Normal", "Happy", "Pain", "Upset"};

BITMAP * ae_tool_image[9] = {NULL};
int ae_selected_tool = AE_TOOL_PAINT;
int ae_selected_color = 0;
int ae_rselected_color = 0;
BITMAP * ae_bg_image = NULL;
BITMAP * ae_palette_image = NULL;
BITMAP * ae_palette_gui_image = NULL;
int ae_hover_tool = -1;

int ae_mouse_x, ae_mouse_y, ae_mouse_b;
int ae_old_mouse_x, ae_old_mouse_y, ae_old_mouse_b;
int ae_click_tick = 0;
int ae_click_x, ae_click_y;
int ae_float_x, ae_float_y;
int ae_float_dx, ae_float_dy;
int ae_hover_avatar = -1;
int ae_hover_color = -1;
int ae_change_count = 0;
int ae_changes = 0;

/* drawing state */
int ae_color = 0;
int ae_drawing = 0;
int ae_selecting = 0;
int ae_select_left = 0;
int ae_select_top = 0;
int ae_select_right = 0;
int ae_select_bottom = 0;
int ae_float_left = 0;
int ae_float_top = 0;
int ae_float_right = 0;
int ae_float_bottom = 0;
int ae_clipboard_left = 0;
int ae_clipboard_top = 0;
int ae_clipboard_right = 0;
int ae_clipboard_bottom = 0;

NCDFS_FILTER_LIST * ae_filter_avatar_files = NULL;
NCDFS_FILTER_LIST * ae_filter_image_files = NULL;

void ae_fix_window_title(void)
{
	char window_text[1024] = {0};
	
	if(ustricmp(ae_filename, ""))
	{
		usprintf(window_text, "%svGolf Avatar Editor - %s", ae_changes ? "*" : "", ae_filename);
	}
	else
	{
		usprintf(window_text, "%svGolf Avatar Editor - Untitled", ae_changes ? "*" : "");
	}
	set_window_title(window_text);
}

void ae_reset_state(void)
{
	ae_color = 0;
	ae_drawing = 0;
	ae_selecting = 0;
	ae_select_left = 0;
	ae_select_top = 0;
	ae_select_right = 0;
	ae_select_bottom = 0;
	ae_float_left = 0;
	ae_float_top = 0;
	ae_float_right = 0;
	ae_float_bottom = 0;
	ae_undo_reset();
	ae_changes = 0;
}

void ae_unfloat_selection(void)
{
	clear_bitmap(ae_float);
	ae_selecting = 0;
}

int ae_initialize(int argc, char * argv[])
{
	BITMAP * bp;
	
	allegro_init();
	set_window_title("vGolf Avatar Editor");
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
	
	ae_filter_avatar_files = ncdfs_filter_list_create();
	if(!ae_filter_avatar_files)
	{
		allegro_message("Could not create file list filter!");
		return 0;
	}
	ncdfs_filter_list_add(ae_filter_avatar_files, "pcx", "PCX Files (*.pcx)", 1);

	ae_filter_image_files = ncdfs_filter_list_create();
	if(!ae_filter_image_files)
	{
		allegro_message("Could not create file list filter!");
		return 0;
	}
	ncdfs_filter_list_add(ae_filter_image_files, "pcx;bmp;tga;lbm", "Image Files (*.pcx;*.bmp;*.tga;*.lbm)", 1);

	InitIdleSystem();
	
	if(set_gfx_mode(GFX_AUTODETECT_WINDOWED, 408, 440, 0, 0))
	{
		return 0;
	}
	set_display_switch_mode(SWITCH_PAUSE);
	PrepVSyncIdle();
	
	ae_screen = create_bitmap(408, 440);
	if(!ae_screen)
	{
		return 0;
	}
	
	bp = load_pcx("avatar_editor.dat#c9a00-00_pcx", ae_palette);
	if(bp)
	{
		destroy_bitmap(bp);
	}
	set_palette(ae_palette);
	
	ae_tool_image[0] = load_pcx("avatar_editor.dat#drawing_tool_0.pcx", ae_temp_palette);
	convert_palette(ae_tool_image[0], ae_temp_palette, ae_palette);
	ae_tool_image[1] = load_pcx("avatar_editor.dat#drawing_tool_1.pcx", ae_temp_palette);
	convert_palette(ae_tool_image[1], ae_temp_palette, ae_palette);
	ae_tool_image[2] = load_pcx("avatar_editor.dat#drawing_tool_2.pcx", ae_temp_palette);
	convert_palette(ae_tool_image[2], ae_temp_palette, ae_palette);
	ae_tool_image[3] = load_pcx("avatar_editor.dat#drawing_tool_3.pcx", ae_temp_palette);
	convert_palette(ae_tool_image[3], ae_temp_palette, ae_palette);
	ae_tool_image[4] = load_pcx("avatar_editor.dat#drawing_tool_4.pcx", ae_temp_palette);
	convert_palette(ae_tool_image[4], ae_temp_palette, ae_palette);
	ae_tool_image[5] = load_pcx("avatar_editor.dat#drawing_tool_5.pcx", ae_temp_palette);
	convert_palette(ae_tool_image[5], ae_temp_palette, ae_palette);
	ae_tool_image[6] = load_pcx("avatar_editor.dat#drawing_tool_6.pcx", ae_temp_palette);
	convert_palette(ae_tool_image[6], ae_temp_palette, ae_palette);
	ae_tool_image[7] = load_pcx("avatar_editor.dat#drawing_tool_7.pcx", ae_temp_palette);
	convert_palette(ae_tool_image[7], ae_temp_palette, ae_palette);
	ae_tool_image[8] = load_pcx("avatar_editor.dat#drawing_tool_8.pcx", ae_temp_palette);
	convert_palette(ae_tool_image[8], ae_temp_palette, ae_palette);
	ae_bg_image = load_pcx("avatar_editor.dat#drawing_bg.pcx", ae_temp_palette);
	convert_palette(ae_bg_image, ae_temp_palette, ae_palette);
	ae_palette_image = load_pcx("avatar_editor.dat#drawing_palette.pcx", NULL);
	ae_palette_gui_image = load_pcx("avatar_editor.dat#drawing_palette_gui.pcx", NULL);
	
	ae_zoom_screen = create_sub_bitmap(ae_screen, 408 - ae_palette_gui_image->w + 3, 16 + 33 + 325, 64, 58);
	if(!ae_zoom_screen)
	{
		return 0;
	}
	
	ae_scratch = create_bitmap(64, 58);
	if(!ae_scratch)
	{
		return 0;
	}
	
	ae_float = create_bitmap(64, 58);
	if(!ae_float)
	{
		return 0;
	}
	clear_bitmap(ae_float);
	
	ae_float_bg = create_bitmap(64, 58);
	if(!ae_float_bg)
	{
		return 0;
	}
	clear_bitmap(ae_float_bg);
	
	show_mouse(NULL);
	if(show_os_cursor(2) < 0)
	{
		ncdgui_initialize(NCDGUI_CURSOR_SOFTWARE);
	}
	else
	{
		ncdgui_initialize(NCDGUI_CURSOR_OS);
	}
	
	ae_prepare_menus();
	
	gametime_init(60); // 100hz timer
	return 1;
}

void ae_exit(void)
{
	delete_file("ae.undo0");
	delete_file("ae.undo1");
	delete_file("ae.undo2");
	delete_file("ae.undo3");
	delete_file("ae.undo4");
	delete_file("ae.undo5");
	delete_file("ae.undo6");
	delete_file("ae.undo7");
	delete_file("ae.redo");
}

void ae_prepare_undo(int type)
{
	if(ae_undo_add(type))
	{
		ae_change_count++;
		ae_changes = 1;
	}
	ae_redo_toggle = 0;
	ae_undo_toggle = 1;
	ae_fix_window_title();
}

void ae_tool_logic(void)
{
	/* handle initial click (save undo state) */
	if(!(ae_old_mouse_b & 1) && (ae_mouse_b & 1))
	{
		if(ae_selected_tool != AE_TOOL_SELECT || ae_selecting == 2)
		{
			ae_prepare_undo(0);
		}
		ae_color = ae_selected_color;
		ae_click_x = ae_mouse_x;
		ae_click_y = ae_mouse_y;
		blit(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, 0, 0, 64, 58);
	}
	else if(!(ae_old_mouse_b & 2) && (ae_mouse_b & 2))
	{
		if(ae_selected_tool != AE_TOOL_SELECT || ae_selecting == 2)
		{
			ae_prepare_undo(0);
		}
		ae_color = ae_rselected_color;
		ae_click_x = ae_mouse_x;
		ae_click_y = ae_mouse_y;
		blit(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, 0, 0, 64, 58);
	}
	switch(ae_selected_tool)
	{
		case AE_TOOL_PAINT:
		{
			putpixel(ae_avatar->image[ae_selected_image], (ae_mouse_x - 8) / 5, (ae_mouse_y - 57) / 5, ae_color);
			break;
		}
		case AE_TOOL_LINE:
		{
			blit(ae_scratch, ae_avatar->image[ae_selected_image], 0, 0, 0, 0, 64, 58);
			line(ae_avatar->image[ae_selected_image], (ae_click_x - 8) / 5, (ae_click_y - 57) / 5, (ae_mouse_x - 8) / 5, (ae_mouse_y - 57) / 5, ae_color);
			break;
		}
		case AE_TOOL_FILL:
		{
			floodfill(ae_avatar->image[ae_selected_image], (ae_mouse_x - 8) / 5, (ae_mouse_y - 57) / 5, ae_color);
			break;
		}
		case AE_TOOL_RECTANGLE:
		{
			blit(ae_scratch, ae_avatar->image[ae_selected_image], 0, 0, 0, 0, 64, 58);
			rect(ae_avatar->image[ae_selected_image], (ae_click_x - 8) / 5, (ae_click_y - 57) / 5, (ae_mouse_x - 8) / 5, (ae_mouse_y - 57) / 5, ae_color);
			break;
		}
		case AE_TOOL_FILLED_RECTANGLE:
		{
			blit(ae_scratch, ae_avatar->image[ae_selected_image], 0, 0, 0, 0, 64, 58);
			rectfill(ae_avatar->image[ae_selected_image], (ae_click_x - 8) / 5, (ae_click_y - 57) / 5, (ae_mouse_x - 8) / 5, (ae_mouse_y - 57) / 5, ae_color);
			break;
		}
		case AE_TOOL_CLEAR:
		{
			clear_to_color(ae_avatar->image[ae_selected_image], ae_color);
			break;
		}
		case AE_TOOL_CIRCLE:
		{
			int rx = ((ae_mouse_x - 8) / 5 - (ae_click_x - 8) / 5) / 2;
			int ry = ((ae_mouse_y - 57) / 5 - (ae_click_y - 57) / 5) / 2;
			int fx = 0, fy = 0;
			if(rx < 0)
			{
				rx = -rx;
				fx = 1;
			}
			if(ry < 0)
			{
				ry = -ry;
				fy = 1;
			}
			blit(ae_scratch, ae_avatar->image[ae_selected_image], 0, 0, 0, 0, 64, 58);
			ellipse(ae_avatar->image[ae_selected_image], (ae_click_x - 8) / 5 + (fx ? -rx : rx), (ae_click_y - 57) / 5 + (fy ? -ry : ry), rx, ry, ae_color);
			break;
		}
		case AE_TOOL_FILLED_CIRCLE:
		{
			int rx = ((ae_mouse_x - 8) / 5 - (ae_click_x - 8) / 5) / 2;
			int ry = ((ae_mouse_y - 57) / 5 - (ae_click_y - 57) / 5) / 2;
			int fx = 0, fy = 0;
			if(rx < 0)
			{
				rx = -rx;
				fx = 1;
			}
			if(ry < 0)
			{
				ry = -ry;
				fy = 1;
			}
			blit(ae_scratch, ae_avatar->image[ae_selected_image], 0, 0, 0, 0, 64, 58);
			ellipsefill(ae_avatar->image[ae_selected_image], (ae_click_x - 8) / 5 + (fx ? -rx : rx), (ae_click_y - 57) / 5 + (fy ? -ry : ry), rx, ry, ae_color);
			break;
		}
		case AE_TOOL_SELECT:
		{
			
			/* begin drawing selection rectangle */
			if(!ae_selecting)
			{
				ae_selecting = 1;
				clear_bitmap(ae_float);
			}
			
			/* drawing selection rectangle */
			if(ae_selecting == 1)
			{
				if(ae_click_x < ae_mouse_x)
				{
					ae_select_left = (ae_click_x - 8) / 5;
					ae_select_right = (ae_mouse_x - 8) / 5;
				}
				else
				{
					ae_select_left = (ae_mouse_x - 8) / 5;
					ae_select_right = (ae_click_x - 8) / 5;
				}
				if(ae_click_y < ae_mouse_y)
				{
					ae_select_top = (ae_click_y - 57) / 5;
					ae_select_bottom = (ae_mouse_y - 57) / 5;
				}
				else
				{
					ae_select_top = (ae_mouse_y - 57) / 5;
					ae_select_bottom = (ae_click_y - 57) / 5;
				}
				ae_float_x = (ae_select_right - ae_select_left) * 5 + 8;
				ae_float_y = (ae_select_bottom - ae_select_top) * 5 + 57;
			}
			
			/* moving the floating selection */
			else if(ae_selecting == 2)
			{
				
				/* click and drag the floating selection */
				if(ae_mouse_x >= ae_select_left * 5 + 8 && ae_mouse_x < (ae_select_right + 1) * 5 + 8 && ae_mouse_y >= ae_select_top * 5 + 57 && ae_mouse_y < (ae_select_bottom + 1) * 5 + 57)
				{
					blit(ae_float_bg, ae_scratch, 0, 0, 0, 0, 64, 58);
					ae_selecting = 3;
				}
				
				/* otherwise, draw a new selection */
				else
				{
					ae_selecting = 1;
					clear_bitmap(ae_float);
				}
			}
			
			/* moving floating selection */
			else if(ae_selecting == 3)
			{
				if(ae_click_tick == 2)
				{
					clear_bitmap(ae_float);
					blit(ae_scratch, ae_float, 0, 0, -ae_float_left, -ae_float_top, ae_float_right + 1, ae_float_bottom + 1);
					rectfill(ae_scratch, ae_float_left, ae_float_top, ae_float_right, ae_float_bottom, 0);
				}
				ae_select_left = ae_float_left + (ae_mouse_x - ae_click_x) / 5 + ae_float_dx;
				ae_select_right = ae_float_right + (ae_mouse_x - ae_click_x) / 5 + ae_float_dx;
				ae_select_top = ae_float_top + (ae_mouse_y - ae_click_y) / 5 + ae_float_dy;
				ae_select_bottom = ae_float_bottom + (ae_mouse_y - ae_click_y) / 5 + ae_float_dy;
				blit(ae_scratch, ae_avatar->image[ae_selected_image], 0, 0, 0, 0, 64, 58);
				draw_sprite(ae_avatar->image[ae_selected_image], ae_float, ae_select_left, ae_select_top);
			}
			
			/* moving the floating selection */
			else if(ae_selecting == 4)
			{
				
				/* click and drag the floating selection */
				if(ae_mouse_x >= ae_select_left * 5 + 8 && ae_mouse_x < (ae_select_right + 1) * 5 + 8 && ae_mouse_y >= ae_select_top * 5 + 57 && ae_mouse_y < (ae_select_bottom + 1) * 5 + 57)
				{
					blit(ae_float_bg, ae_scratch, 0, 0, 0, 0, 64, 58);
					ae_selecting = 3;
				}
				
				/* otherwise, draw a new selection */
				else
				{
					ae_selecting = 1;
					clear_bitmap(ae_float);
				}
			}
			
			/* moving the pasted floating selection */
			else if(ae_selecting == 5)
			{
				
				/* click and drag the floating selection */
				if(ae_mouse_x >= ae_select_left * 5 + 8 && ae_mouse_x < (ae_select_right + 1) * 5 + 8 && ae_mouse_y >= ae_select_top * 5 + 57 && ae_mouse_y < (ae_select_bottom + 1) * 5 + 57)
				{
					blit(ae_float_bg, ae_scratch, 0, 0, 0, 0, 64, 58);
					ae_selecting = 6;
				}
				
				/* otherwise, draw a new selection */
				else
				{
					ae_selecting = 1;
					clear_bitmap(ae_float);
				}
			}
			
			/* moving the pasted floating selection */
			else if(ae_selecting == 6)
			{
				ae_select_left = ae_float_left + (ae_mouse_x - ae_click_x) / 5 + ae_float_dx;
				ae_select_right = ae_float_right + (ae_mouse_x - ae_click_x) / 5 + ae_float_dx;
				ae_select_top = ae_float_top + (ae_mouse_y - ae_click_y) / 5 + ae_float_dy;
				ae_select_bottom = ae_float_bottom + (ae_mouse_y - ae_click_y) / 5 + ae_float_dy;
				blit(ae_float_bg, ae_avatar->image[ae_selected_image], 0, 0, 0, 0, 64, 58);
				draw_sprite(ae_avatar->image[ae_selected_image], ae_float, ae_select_left, ae_select_top);
			}
			break;
		}
	}
}

void ae_logic(void)
{
	if(key[KEY_ESC])
	{
		ae_menu_file_exit();
		key[KEY_ESC] = 0;
	}
	if(key[KEY_F1])
	{
		ae_menu_help_manual();
		key[KEY_F1] = 0;
	}
	if(key[KEY_F3])
	{
		ae_menu_file_load();
		key[KEY_F3] = 0;
	}
	if(key[KEY_F2])
	{
		if(KEY_EITHER_CTRL)
		{
			ae_menu_file_save_as();
		}
		else
		{
			ae_menu_file_save();
		}
		key[KEY_F2] = 0;
	}
	if(key[KEY_F5])
	{
		ae_menu_file_load_image();
		key[KEY_F5] = 0;
	}
	
	/* activate the menu when ALT is pressed */
	if(KEY_EITHER_ALT)
	{
		clear_keybuf();
		ae_prepare_menus();
		ncdgui_popup_dialog(ae_main_dialog, 0);
	}
	
	/* see if we need to activate the menu */
	if(mouse_y < 22 && mouse_b & 1)
	{
		clear_keybuf();
		ae_prepare_menus();
		ncdgui_popup_dialog(ae_main_dialog, 0);
	}
	
	if(key[KEY_A])
	{
		ae_menu_image_shift_left();
		key[KEY_A] = 0;
	}
	else if(key[KEY_D])
	{
		ae_menu_image_shift_right();
		key[KEY_D] = 0;
	}
	else if(key[KEY_W])
	{
		ae_menu_image_shift_up();
		key[KEY_W] = 0;
	}
	else if(key[KEY_S])
	{
		ae_menu_image_shift_down();
		key[KEY_S] = 0;
	}
	else if(KEY_EITHER_CTRL && key[KEY_Z])
	{
		ae_menu_edit_undo();
		key[KEY_Z] = 0;
	}
	else if(KEY_EITHER_CTRL && key[KEY_Y])
	{
		ae_menu_edit_redo();
		key[KEY_Y] = 0;
	}
	else if(KEY_EITHER_CTRL && key[KEY_C])
	{
		ae_menu_edit_copy();
		key[KEY_C] = 0;
	}
	else if(KEY_EITHER_CTRL && key[KEY_V])
	{
		ae_menu_edit_paste();
		key[KEY_V] = 0;
	}
	else if(key[KEY_H])
	{
		ae_menu_image_mirror();
		key[KEY_H] = 0;
	}
	else if(key[KEY_V])
	{
		ae_menu_image_flip();
		key[KEY_V] = 0;
	}
	else if(key[KEY_RIGHT])
	{
		ae_menu_image_turn_clockwise();
		key[KEY_RIGHT] = 0;
	}
	else if(key[KEY_LEFT])
	{
		ae_menu_image_turn_counter_clockwise();
		key[KEY_LEFT] = 0;
	}
	
	if(ae_avatar)
	{
		ae_hover_avatar = -1;
		ae_hover_color = -1;
		ae_hover_tool = -1;
		ae_old_mouse_x = ae_mouse_x;
		ae_old_mouse_y = ae_mouse_y;
		ae_old_mouse_b = ae_mouse_b;
		ae_mouse_x = mouse_x;
		ae_mouse_y = mouse_y;
		ae_mouse_b = mouse_b;
		if(!ae_drawing)
		{
			if(ae_mouse_x >= 8 && ae_mouse_x < 8 + 64 - 1&& ae_mouse_y >= 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8 && ae_mouse_y < 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8 + 64)
			{
				ae_hover_avatar = 0;
			}
			else if(ae_mouse_x >= 8 + 85 && ae_mouse_x < 8 + 85 + 64 - 1 && ae_mouse_y >= 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8 && ae_mouse_y < 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8 + 64)
			{
				ae_hover_avatar = 1;
			}
			else if(ae_mouse_x >= 8 + 2 * 85 && ae_mouse_x < 8 + 2 * 85 + 64 - 1 && ae_mouse_y >= 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8 && ae_mouse_y < 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8 + 64)
			{
				ae_hover_avatar = 2;
			}
			else if(ae_mouse_x >= 8 + 3 * 85 && ae_mouse_x < 8 + 3 * 85 + 64 - 1 && ae_mouse_y >= 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8 && ae_mouse_y < 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8 + 64)
			{
				ae_hover_avatar = 3;
			}
			else if(ae_mouse_x >= 408 - ae_palette_image->w - 1 && ae_mouse_x < 408 - 2 && ae_mouse_y >= 81 && ae_mouse_y < 81 + ae_palette_image->h - 1)
			{
				ae_hover_color = ((ae_mouse_y - 81) / 8) * 8 + (ae_mouse_x - (408 - ae_palette_image->w - 1)) / 8;
			}
			else if(ae_mouse_x >= 0 && ae_mouse_x < 9 * 32 && ae_mouse_y >= 16 && ae_mouse_y < 48)
			{
				ae_hover_tool = ae_mouse_x / 32;
			}
		}
		if(ae_mouse_b & 1)
		{
			ae_click_tick++;
			if(ae_hover_avatar >= 0 && ae_hover_avatar < 4)
			{
				ae_selected_image = ae_hover_avatar;
				ae_unfloat_selection();
			}
			if(ae_hover_color >= 0)
			{
				ae_selected_color = ae_hover_color;
			}
			if(ae_hover_tool >= 0)
			{
				if(ae_selected_tool == AE_TOOL_SELECT && ae_hover_tool != AE_TOOL_SELECT)
				{
					ae_unfloat_selection();
				}
				ae_selected_tool = ae_hover_tool;
			}
			if(ae_mouse_x >= 8 && ae_mouse_x < 8 + ae_avatar->image[ae_selected_image]->w * 5 && ae_mouse_y >= 57 && ae_mouse_y < 57 + ae_avatar->image[ae_selected_image]->h * 5)
			{
				if(KEY_EITHER_CTRL)
				{
					ae_selected_color = ae_avatar->image[ae_selected_image]->line[(ae_mouse_y - 57) / 5][(ae_mouse_x - 8) / 5];
				}
				else
				{
					ae_drawing = 1;
				}
			}
		}
		else if(ae_mouse_b & 2)
		{
			if(ae_hover_color >= 0)
			{
				ae_rselected_color = ae_hover_color;
			}
			if(ae_mouse_x >= 8 && ae_mouse_x < 8 + ae_avatar->image[ae_selected_image]->w * 5 && ae_mouse_y >= 57 && ae_mouse_y < 57 + ae_avatar->image[ae_selected_image]->h * 5)
			{
				if(KEY_EITHER_CTRL)
				{
					ae_rselected_color = ae_avatar->image[ae_selected_image]->line[(ae_mouse_y - 57) / 5][(ae_mouse_x - 8) / 5];
				}
				else
				{
					ae_drawing = 1;
				}
			}
		}
		else
		{
			ae_click_tick = 0;
			ae_drawing = 0;
			if(ae_selecting == 1)
			{
				if(ae_select_right - ae_select_left > 1 || ae_select_bottom - ae_select_top > 1)
				{
					ae_selecting = 2;
					ae_float_left = ae_select_left;
					ae_float_right = ae_select_right;
					ae_float_top = ae_select_top;
					ae_float_bottom = ae_select_bottom;
					ae_float_dx = 0;
					ae_float_dy = 0;
					clear_bitmap(ae_float_bg);
					blit(ae_avatar->image[ae_selected_image], ae_float_bg, 0, 0, 0, 0, 64, 58);
				}
				else
				{
					ae_selecting = 0;
				}
			}
			else if(ae_selecting == 3)
			{
				ae_selecting = 4;
				ae_float_dx = ae_select_left - ae_float_left;
				ae_float_dy = ae_select_top - ae_float_top;
			}
			else if(ae_selecting == 5 || ae_selecting == 6)
			{
				ae_float_dx = ae_select_left - ae_float_left;
				ae_float_dy = ae_select_top - ae_float_top;
			}
		}
		if(ae_mouse_x >= 8 && ae_mouse_x < 8 + ae_avatar->image[ae_selected_image]->w * 5 && ae_mouse_y >= 57 && ae_mouse_y < 57 + ae_avatar->image[ae_selected_image]->h * 5)
		{
			ae_hover_color = ae_avatar->image[ae_selected_image]->line[(ae_mouse_y - 57) / 5][(ae_mouse_x - 8) / 5];
		}
		if(ae_drawing)
		{
			ae_tool_logic();
		}
	}
}

void ae_render(void)
{
	int i;
	
	clear_to_color(ae_screen, makecol(0, 0, 64));
	blit(ae_menu_image, ae_screen, 0, 0, 0, 0, ae_screen->w, ae_screen->h);
	blit(ae_tool_image[ae_selected_tool], ae_screen, 0, 0, 0, 16, ae_screen->w, ae_screen->h);
	blit(ae_palette_gui_image, ae_screen, 0, 0, 408 - ae_palette_gui_image->w, 16 + 33, ae_screen->w, ae_screen->h);
	blit(ae_palette_image, ae_screen, 0, 0, 408 - ae_palette_image->w - 1, 81, ae_screen->w, ae_screen->h);
	rectfill(ae_screen, 408 - ae_palette_gui_image->w + 3, 16 + 33 + 3, 408 - ae_palette_gui_image->w + 32, 16 + 33 + 28, ae_selected_color);
	rectfill(ae_screen, 408 - ae_palette_gui_image->w + 37, 16 + 33 + 3, 408 - ae_palette_gui_image->w + 66, 16 + 33 + 28, ae_rselected_color);
	clear_to_color(ae_zoom_screen, makecol(0, 0, 0));
	if(ae_hover_color >= 0)
	{
		rectfill(ae_screen, 408 - ae_palette_gui_image->w + 3, 16 + 33 + 291, 408 - ae_palette_gui_image->w + 66, 16 + 33 + 316, ae_hover_color);
	}
	
	if(ae_avatar)
	{
		rectfill(ae_screen, 8, 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8, 8 + 3 * 85 + ae_avatar->image[ae_selected_image]->w - 1, 57 + ae_avatar->image[ae_selected_image]->h * 5 + ae_avatar->image[ae_selected_image]->h + 8 + 8 + 8 - 1, makecol(0, 0, 0));
		if(ae_hover_avatar >= 0 && ae_hover_avatar < 4)
		{
			rectfill(ae_screen, 8 + ae_hover_avatar * 85, 57 + ae_avatar->image[ae_hover_avatar]->h * 5 + 8, 8 + ae_hover_avatar * 85 + 64 - 1, 57 + ae_avatar->image[ae_hover_avatar]->h * 5 + 8 + ae_avatar->image[ae_hover_avatar]->h + 16 - 1, makecol(64, 58, 64));
//			draw_sprite(ae_screen, ae_avatar->image[i], 8 + i * 85, 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8);
		}
		rectfill(ae_screen, 8 + ae_selected_image * 85, 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8, 8 + ae_selected_image * 85 + 64 - 1, 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8 + ae_avatar->image[ae_selected_image]->h + 16 - 1, makecol(0, 128, 0));
//		rectfill(ae_screen, 640 - ae_avatar->image[ae_selected_image]->w - 8 - ae_palette_image->w - 8, 192 + ae_selected_image * (ae_avatar->image[0]->h + 16), 640 - ae_avatar->image[ae_selected_image]->w - 8 - ae_palette_image->w - 8 + 64, 192 + ae_selected_image * (ae_avatar->image[0]->h + 16) + ae_avatar->image[0]->h + 16, makecol(0, 128, 0));
		
		for(i = 0; i < VGOLF_AVATAR_MAX_IMAGES; i++)
		{
			draw_sprite(ae_screen, ae_avatar->image[i], 8 + i * 85, 57 + ae_avatar->image[ae_selected_image]->h * 5 + 8);
			textprintf_centre_ex(ae_screen, font, 8 + i * 85 + ae_avatar->image[i]->w / 2, 57 + ae_avatar->image[ae_selected_image]->h * 5 + ae_avatar->image[ae_selected_image]->h + 8, makecol(255, 255, 255), -1, "%s", ae_avatar_type[i]);
		}
//		draw_sprite(ae_screen, ae_avatar->image[ae_selected_image], 640 - ae_avatar->image[ae_selected_image]->w - 8 - ae_palette_image->w - 8, 65);
//		textprintf_centre_ex(ae_screen, font, 640 - ae_avatar->image[ae_selected_image]->w - 8 - ae_palette_image->w - 8 + ae_avatar->image[ae_selected_image]->w / 2, 57, makecol(255, 255, 255), -1, "Preview");
		
		set_clip_rect(ae_screen, 8, 57, 8 + ae_avatar->image[ae_selected_image]->w * 5 - 1, 57 + ae_avatar->image[ae_selected_image]->h * 5 - 1);
		drawing_mode(DRAW_MODE_COPY_PATTERN, ae_bg_image, 8, 57);
		rectfill(ae_screen, 8, 57, 8 + ae_avatar->image[ae_selected_image]->w * 5 - 1, 57 + ae_avatar->image[ae_selected_image]->h * 5 - 1, makecol(255, 0, 255));
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
		stretch_sprite(ae_screen, ae_avatar->image[ae_selected_image], 8, 57, ae_avatar->image[ae_selected_image]->w * 5, ae_avatar->image[ae_selected_image]->h * 5);
		stretch_sprite(ae_screen, ae_float, 8 + ae_select_left * 5, 57 + ae_select_top * 5, ae_float->w * 5, ae_float->h * 5);
		if(ae_selecting)
		{
			rect(ae_screen, 8 + ae_select_left * 5, 57 + ae_select_top * 5, 8 + ae_select_right * 5 + 4, 57 + ae_select_bottom * 5 + 4, makecol(0, 0, 0));
		}
		set_clip_rect(ae_screen, 0, 0, ae_screen->w - 1, ae_screen->h - 1);
		
		drawing_mode(DRAW_MODE_COPY_PATTERN, ae_bg_image, 0, 0);
		rectfill(ae_zoom_screen, 0, 0, 63, 63, makecol(255, 0, 255));
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
		stretch_sprite(ae_zoom_screen, ae_avatar->image[ae_selected_image], (-(ae_mouse_x - 8) / 5) * 12 + 24 + 2, (-(ae_mouse_y - 57) / 5) * 12 + 24 + 2, ae_avatar->image[ae_selected_image]->w * 12, ae_avatar->image[ae_selected_image]->h * 12);
		rect(ae_zoom_screen, 32 - 7, 32 - 7, 32 + 6, 32 + 6, makecol(255, 255, 255));
		rect(ae_zoom_screen, 32 - 6, 32 - 6, 32 + 5, 32 + 5, makecol(0, 0, 0));
	}
	
	IdleUntilVSync();
	vsync();
	DoneVSync();
	
	blit(ae_screen, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

int main(int argc, char * argv[])
{
	int updated = 0;
	if(!ae_initialize(argc, argv))
	{
		return -1;
	}
	while(!ae_quit)
	{
		/* frame skip mode */
		while(gametime_get_frames() - gametime_tick > 0)
		{
			ae_logic();
			updated = 0;
			++gametime_tick;
		}
		
		/* update and draw the screen */
		if(!updated)
		{
			ae_render();
			updated = 1;
		}
		else
		{
			IdleUntilVSync();
			vsync();
			DoneVSync();
		}
	}
	ae_exit();
	return 0;
}
END_OF_MAIN()
